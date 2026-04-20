#include <f1x/openauto/autoapp/Service/MappingLibraryService.hpp>

#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QStandardPaths>
#include <QUrl>
#include <QVariantMap>

static constexpr const char* INDEX_URL =
    "https://raw.githubusercontent.com/journeyos/canbus-maps/main/index.json";

namespace f1x::openauto::autoapp::service {

MappingLibraryService::MappingLibraryService(QObject* parent)
    : QObject(parent)
    , nam_(new QNetworkAccessManager(this))
{}

// ─── public slots ─────────────────────────────────────────────────────────────

void MappingLibraryService::search(const QString& make, const QString& model, int year)
{
    // If the index is already loaded, just re-filter.
    if (!index_.isEmpty()) {
        applyFilter(make, model, year);
        return;
    }
    fetchIndex(make, model, year);
}

void MappingLibraryService::download(int index)
{
    if (index < 0 || index >= results_.size()) {
        setStatusMessage("Invalid selection.");
        return;
    }

    const QVariantMap entry = results_.at(index).toMap();
    const QString file      = entry.value("file").toString();
    const QString sha256    = entry.value("sha256").toString();

    if (file.isEmpty()) {
        setStatusMessage("Index entry has no file path.");
        return;
    }

    const QString rawBase = QStringLiteral(
        "https://raw.githubusercontent.com/journeyos/canbus-maps/main/maps/");
    const QUrl url(rawBase + file);

    setLoading(true);
    setStatusMessage("Downloading…");

    QNetworkRequest req(url);
    req.setTransferTimeout(30000);
    auto* reply = nam_->get(req);

    connect(reply, &QNetworkReply::finished, this, [this, reply, file, sha256]() {
        reply->deleteLater();
        setLoading(false);

        if (reply->error() != QNetworkReply::NoError) {
            setStatusMessage("Download failed: " + reply->errorString());
            return;
        }

        const QByteArray data = reply->readAll();

        // Validate JSON before writing
        QJsonParseError err;
        QJsonDocument::fromJson(data, &err);
        if (err.error != QJsonParseError::NoError) {
            setStatusMessage("Downloaded file is not valid JSON.");
            return;
        }

        // Determine local path: use the file basename only
        const QString baseName = QFileInfo(file).fileName();
        QDir dir(localDownloadDir());
        if (!dir.exists())
            dir.mkpath(".");

        const QString localPath = dir.filePath(baseName);
        QFile out(localPath);
        if (!out.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            setStatusMessage("Could not write to: " + localPath);
            return;
        }
        out.write(data);
        out.close();

        // Verify SHA-256 if provided
        if (!sha256.isEmpty() && !verifySha256(localPath, sha256)) {
            QFile::remove(localPath);
            setStatusMessage("SHA-256 verification failed — file removed.");
            return;
        }

        setStatusMessage("Downloaded: " + baseName);
        emit downloadComplete(localPath);
    });
}

// ─── private ──────────────────────────────────────────────────────────────────

void MappingLibraryService::fetchIndex(const QString& make, const QString& model, int year)
{
    setLoading(true);
    setStatusMessage("Fetching community index…");

    QNetworkRequest req((QUrl(INDEX_URL)));
    req.setTransferTimeout(15000);
    auto* reply = nam_->get(req);

    connect(reply, &QNetworkReply::finished, this, [this, reply, make, model, year]() {
        reply->deleteLater();
        setLoading(false);

        if (reply->error() != QNetworkReply::NoError) {
            setStatusMessage("Could not fetch index: " + reply->errorString());
            return;
        }

        QJsonParseError err;
        const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll(), &err);
        if (err.error != QJsonParseError::NoError || !doc.isArray()) {
            setStatusMessage("Index format error.");
            return;
        }

        index_.clear();
        for (const QJsonValue& v : doc.array())
            index_.append(v.toObject().toVariantMap());

        applyFilter(make, model, year);
    });
}

void MappingLibraryService::applyFilter(const QString& make, const QString& model, int year)
{
    const QString makeLower  = make.trimmed().toLower();
    const QString modelLower = model.trimmed().toLower();

    QVariantList filtered;
    for (const QVariant& v : std::as_const(index_)) {
        const QVariantMap e = v.toMap();

        if (!makeLower.isEmpty() &&
            !e.value("make").toString().toLower().contains(makeLower))
            continue;

        if (!modelLower.isEmpty() &&
            !e.value("model").toString().toLower().contains(modelLower))
            continue;

        if (year > 0) {
            const int from = e.value("year_from", 0).toInt();
            const int to   = e.value("year_to",   9999).toInt();
            if (year < from || year > to)
                continue;
        }

        filtered.append(v);
    }

    results_ = filtered;
    emit resultsChanged();

    if (filtered.isEmpty())
        setStatusMessage("No maps found matching your search.");
    else
        setStatusMessage(QString::number(filtered.size()) + " map(s) found.");
}

QString MappingLibraryService::localDownloadDir() const
{
    return QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation)
           + QStringLiteral("/canbus");
}

bool MappingLibraryService::verifySha256(const QString& filePath,
                                          const QString& expectedHex) const
{
    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly))
        return false;
    const QByteArray hash =
        QCryptographicHash::hash(f.readAll(), QCryptographicHash::Sha256).toHex();
    return hash.compare(expectedHex.toLatin1(), Qt::CaseInsensitive) == 0;
}

void MappingLibraryService::setLoading(bool v)
{
    if (loading_ == v) return;
    loading_ = v;
    emit loadingChanged();
}

void MappingLibraryService::setStatusMessage(const QString& msg)
{
    if (statusMessage_ == msg) return;
    statusMessage_ = msg;
    emit statusMessageChanged();
}

} // namespace f1x::openauto::autoapp::service