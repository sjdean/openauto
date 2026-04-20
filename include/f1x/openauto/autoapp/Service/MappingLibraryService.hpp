#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>

class QNetworkAccessManager;

namespace f1x::openauto::autoapp::service {

// ─── MappingLibraryService ────────────────────────────────────────────────────
//
// Fetches the community CAN mapping index from GitHub and downloads selected
// mapping files to local storage.
//
// Community repo: https://github.com/journeyos/canbus-maps
// Index URL:      https://raw.githubusercontent.com/journeyos/canbus-maps/main/index.json
//
// Index entry schema:
//   { "name", "make", "model", "year_from", "year_to", "platform",
//     "file", "sha256", "validated" }
//
// Usage in QML:
//   mappingLibrary.search(make, model, year)
//   mappingLibrary.download(resultIndex)
//   mappingLibrary.downloadComplete(localPath) → update settingsViewModel.canBusMappingFile
//
// Thread safety: all methods and signals run on the object's thread.

class MappingLibraryService : public QObject {
    Q_OBJECT

    Q_PROPERTY(bool     loading       READ loading       NOTIFY loadingChanged)
    Q_PROPERTY(QVariantList results   READ results       NOTIFY resultsChanged)
    Q_PROPERTY(QString  statusMessage READ statusMessage NOTIFY statusMessageChanged)

public:
    explicit MappingLibraryService(QObject* parent = nullptr);

    bool        loading()       const { return loading_; }
    QVariantList results()      const { return results_; }
    QString     statusMessage() const { return statusMessage_; }

    // Fetch the index then filter by make/model/year.
    // Pass empty make/model and year=0 to list all entries.
    Q_INVOKABLE void search(const QString& make, const QString& model, int year);

    // Download the file at results[index] to local storage.
    Q_INVOKABLE void download(int index);

signals:
    void loadingChanged();
    void resultsChanged();
    void statusMessageChanged();

    // Emitted with the absolute local path when a download completes successfully.
    void downloadComplete(const QString& localPath);

private:
    void fetchIndex(const QString& make, const QString& model, int year);
    void applyFilter(const QString& make, const QString& model, int year);
    QString localDownloadDir() const;
    bool verifySha256(const QString& filePath, const QString& expectedHex) const;

    void setLoading(bool v);
    void setStatusMessage(const QString& msg);

    QNetworkAccessManager* nam_     = nullptr;
    QVariantList           index_;   // full parsed index array
    QVariantList           results_; // filtered subset
    bool                   loading_ = false;
    QString                statusMessage_;
};

} // namespace f1x::openauto::autoapp::service