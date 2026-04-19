#pragma once

#include <QJsonObject>
#include <QObject>
#include <QString>

#include "f1x/openauto/autoapp/Configuration/IConfiguration.hpp"

class QNetworkAccessManager;

namespace f1x::openauto::autoapp::service {

// ─── VinDecodeService ─────────────────────────────────────────────────────────
//
// Decodes a VIN into make/model/year using:
//   1. A local JSON cache  (~/.local/share/journeyos/vin_cache.json) — hit first
//   2. NHTSA vPIC API      (https://vpic.nhtsa.dot.gov/api/vehicles/DecodeVin/…)
//   3. Bundled WMI table   (:/assets/canbus/wmi_table.json) — offline manufacturer fallback
//
// Connect DeviceManager::vinReceived  → lookupVin
// Connect decoded                     → SettingsViewModel::applyVinDecode
//
// Thread safety: all methods called on the object's thread; QNAM handles its
// own async I/O internally.

class VinDecodeService : public QObject {
    Q_OBJECT

public:
    explicit VinDecodeService(
        configuration::IConfiguration::Pointer configuration,
        QObject* parent = nullptr);

signals:
    // Emitted when a result is available (from cache, NHTSA, or WMI fallback).
    // make/model may be empty strings; year may be 0 if not determined.
    void decoded(const QString& make, const QString& model, int year);

public slots:
    // Triggered by DeviceManager::vinReceived. ecuName and calibrationId are
    // available for future use but not currently used in the decode logic.
    void lookupVin(const QString& vin,
                   const QString& ecuName,
                   const QString& calibrationId);

private:
    void queryNhtsa(const QString& vin, const QString& offlineMake);
    static void parseNhtsaResults(const QByteArray& json,
                                  QString& make, QString& model, int& year);

    QString  cacheFilePath() const;
    QJsonObject loadCache() const;
    void     saveCache(QJsonObject cache, const QString& vin,
                       const QString& make, const QString& model, int year) const;

    QString wmiLookup(const QString& vin) const;

    configuration::IConfiguration::Pointer configuration_;
    QNetworkAccessManager* nam_ = nullptr;
    QJsonObject wmiTable_;
};

} // namespace f1x::openauto::autoapp::service