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

    qInfo(lcUpdate) << "checking for update url=" << serverUrl;

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
        qWarning(lcUpdate) << "download called with no update URL";
        return;
    }
    qInfo(lcUpdate) << "download url=" << m_updateUrl;
}

void UpdateManager::applyUpdate()
{
    if (m_updateUrl.isEmpty()) {
        qWarning(lcUpdate) << "apply called with no update URL";
        return;
    }
    qInfo(lcUpdate) << "apply url=" << m_updateUrl << " (would run: rauc install)";
}

} // namespace
