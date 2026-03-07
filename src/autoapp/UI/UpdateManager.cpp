#include "f1x/openauto/autoapp/UI/UpdateManager.hpp"
#include <qloggingcategory.h>

Q_LOGGING_CATEGORY(lcUpdate, "journeyos.ota")

namespace f1x::openauto::autoapp::UI {

UpdateManager::UpdateManager(configuration::IConfiguration::Pointer config, QObject *parent)
    : QObject(parent)
    , m_config(std::move(config))
{
}

QString UpdateManager::getCurrentVersion() const
{
    // Single source of truth: populated from the CMakeLists.txt version string via a define.
    // Falls back to a human-readable placeholder when the define is absent (dev builds).
#ifdef OPENAUTO_VERSION_STRING
    return QStringLiteral(OPENAUTO_VERSION_STRING);
#else
    return QStringLiteral("dev");
#endif
}

void UpdateManager::checkForUpdate()
{
    const QString serverUrl = m_config->getSettingByName<QString>(
        "Updates", "ServerUrl", "https://updates.example.com/ota/latest.json");

    qInfo(lcUpdate) << "OTA check stub — server URL:" << serverUrl;

    // Stub: no real HTTP request yet. Always reports no update available.
    m_updateAvailable = false;
    m_latestVersion.clear();
    m_updateUrl.clear();

    emit updateAvailableChanged();
    emit latestVersionChanged();
    emit checkComplete(m_updateAvailable, m_latestVersion);
}

void UpdateManager::downloadUpdate()
{
    if (m_updateUrl.isEmpty()) {
        qWarning(lcUpdate) << "downloadUpdate() called with no update URL — check for update first.";
        return;
    }
    qInfo(lcUpdate) << "OTA download stub — would fetch:" << m_updateUrl;
}

void UpdateManager::applyUpdate()
{
    if (m_updateUrl.isEmpty()) {
        qWarning(lcUpdate) << "applyUpdate() called with no update URL — check for update first.";
        return;
    }
    // Stub: log the RAUC command but do NOT execute it.
    qInfo(lcUpdate) << "OTA apply stub — would run: rauc install" << m_updateUrl;
}

} // namespace
