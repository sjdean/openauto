#include "f1x/openauto/autoapp/Service/VinDecodeService.hpp"
#include "f1x/openauto/autoapp/Configuration/ConfigurationKeys.hpp"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QStandardPaths>
#include <qloggingcategory.h>

Q_LOGGING_CATEGORY(lcVin, "journeyos.vin")

namespace f1x::openauto::autoapp::service {

using configuration::ConfigGroup;
using configuration::ConfigKey;

// ─── Construction ─────────────────────────────────────────────────────────────

VinDecodeService::VinDecodeService(configuration::IConfiguration::Pointer configuration,
                                   QObject* parent)
    : QObject(parent)
    , configuration_(std::move(configuration))
    , nam_(new QNetworkAccessManager(this))
{
    // Load bundled WMI table from Qt resources.
    QFile wmiFile(QStringLiteral(":/assets/canbus/wmi_table.json"));
    if (wmiFile.open(QIODevice::ReadOnly)) {
        wmiTable_ = QJsonDocument::fromJson(wmiFile.readAll()).object();
        qCDebug(lcVin) << "WMI table loaded:" << wmiTable_.size() << "entries";
    } else {
        qCWarning(lcVin) << "WMI table not found at :/assets/canbus/wmi_table.json";
    }
}

// ─── Public slot ─────────────────────────────────────────────────────────────

void VinDecodeService::lookupVin(const QString& vin,
                                 const QString& /*ecuName*/,
                                 const QString& /*calibrationId*/)
{
    if (vin.length() != 17) {
        qCWarning(lcVin) << "ignoring invalid VIN length" << vin.length();
        return;
    }

    qCInfo(lcVin) << "VIN lookup:" << vin;

    // ── 1. Local cache ────────────────────────────────────────────────────────
    const QJsonObject cache = loadCache();
    if (cache.contains(vin)) {
        const QJsonObject entry = cache.value(vin).toObject();
        const QString make  = entry.value(QStringLiteral("make")).toString();
        const QString model = entry.value(QStringLiteral("model")).toString();
        const int     year  = entry.value(QStringLiteral("year")).toInt();
        qCInfo(lcVin) << "cache hit:" << make << model << year;
        emit decoded(make, model, year);
        return;
    }

    // ── 2. WMI offline fallback (returned with NHTSA result if available) ──
    const QString offlineMake = wmiLookup(vin);
    if (!offlineMake.isEmpty())
        qCDebug(lcVin) << "WMI offline make:" << offlineMake;

    // ── 3. NHTSA API ──────────────────────────────────────────────────────────
    queryNhtsa(vin, offlineMake);
}

// ─── NHTSA async query ────────────────────────────────────────────────────────

void VinDecodeService::queryNhtsa(const QString& vin, const QString& offlineMake)
{
    const QUrl url(
        QStringLiteral("https://vpic.nhtsa.dot.gov/api/vehicles/DecodeVin/%1?format=json")
            .arg(vin));

    QNetworkRequest req(url);
    req.setTransferTimeout(10000);

    QNetworkReply* reply = nam_->get(req);
    connect(reply, &QNetworkReply::finished, this,
            [this, vin, offlineMake, reply]() {
        reply->deleteLater();

        if (reply->error() != QNetworkReply::NoError) {
            qCWarning(lcVin) << "NHTSA request failed:" << reply->errorString();
            // Fall back to WMI-only result
            if (!offlineMake.isEmpty()) {
                saveCache(loadCache(), vin, offlineMake, QString(), 0);
                emit decoded(offlineMake, QString(), 0);
            }
            return;
        }

        QString make, model;
        int year = 0;
        parseNhtsaResults(reply->readAll(), make, model, year);

        // Use offline make if NHTSA didn't return one
        if (make.isEmpty())
            make = offlineMake;

        qCInfo(lcVin) << "NHTSA result:" << make << model << year;
        saveCache(loadCache(), vin, make, model, year);
        emit decoded(make, model, year);
    });
}

// ─── NHTSA response parser ────────────────────────────────────────────────────

void VinDecodeService::parseNhtsaResults(const QByteArray& json,
                                         QString& make, QString& model, int& year)
{
    const QJsonArray results =
        QJsonDocument::fromJson(json).object().value(QStringLiteral("Results")).toArray();

    for (const auto& r : results) {
        const QJsonObject o   = r.toObject();
        const QString    var  = o.value(QStringLiteral("Variable")).toString();
        const QString    val  = o.value(QStringLiteral("Value")).toString();

        if (val.isEmpty() || val == QLatin1String("Not Applicable"))
            continue;

        if (var == QLatin1String("Make")) {
            // Title-case: "RENAULT" → "Renault"
            make = val.toLower();
            if (!make.isEmpty())
                make[0] = make[0].toUpper();
        } else if (var == QLatin1String("Model")) {
            model = val;
        } else if (var == QLatin1String("Model Year")) {
            year = val.toInt();
        }
    }
}

// ─── Cache helpers ────────────────────────────────────────────────────────────

QString VinDecodeService::cacheFilePath() const
{
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    return dir + QStringLiteral("/vin_cache.json");
}

QJsonObject VinDecodeService::loadCache() const
{
    QFile f(cacheFilePath());
    if (!f.open(QIODevice::ReadOnly))
        return {};
    return QJsonDocument::fromJson(f.readAll()).object();
}

void VinDecodeService::saveCache(QJsonObject cache,
                                 const QString& vin,
                                 const QString& make,
                                 const QString& model,
                                 int year) const
{
    QJsonObject entry;
    entry[QStringLiteral("make")]  = make;
    entry[QStringLiteral("model")] = model;
    entry[QStringLiteral("year")]  = year;
    cache[vin] = entry;

    const QString path = cacheFilePath();
    QDir().mkpath(QFileInfo(path).absolutePath());
    QFile f(path);
    if (f.open(QIODevice::WriteOnly | QIODevice::Truncate))
        f.write(QJsonDocument(cache).toJson(QJsonDocument::Compact));
    else
        qCWarning(lcVin) << "failed to write VIN cache to" << path;
}

// ─── WMI offline lookup ───────────────────────────────────────────────────────

QString VinDecodeService::wmiLookup(const QString& vin) const
{
    if (vin.length() < 3)
        return {};

    // Try 3-char WMI first, then 2-char prefix as fallback
    const QString wmi3 = vin.left(3).toUpper();
    const QString wmi2 = vin.left(2).toUpper();

    if (wmiTable_.contains(wmi3))
        return wmiTable_.value(wmi3).toString();
    if (wmiTable_.contains(wmi2))
        return wmiTable_.value(wmi2).toString();

    return {};
}

} // namespace f1x::openauto::autoapp::service