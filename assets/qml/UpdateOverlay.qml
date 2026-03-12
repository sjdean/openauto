import QtQuick 2.0
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.1

Item {
    id: root
    // Assuming 'UpdateManager' is registered as a context property
    property var manager: UpdateManager 

    ColumnLayout {
        anchors.centerIn: parent
        spacing: 20
        width: 400

        Text {
            text: "System Update"
            font.pointSize: 24
            color: "white"
            Layout.alignment: Qt.AlignHCenter
        }

        Text {
            text: "Current Version: " + manager.currentVersion
            color: "gray"
            Layout.alignment: Qt.AlignHCenter
        }

        BusyIndicator {
            id: busySpinner
            running: false
            Layout.alignment: Qt.AlignCenter
            visible: running
        }

        // Status Message Area
        Text {
            id: statusText
            text: manager.updateAvailable ? "Update " + manager.latestVersion + " available." : "System is up to date."
            color: "white"
            font.pointSize: 14
            Layout.alignment: Qt.AlignHCenter
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
        }

        // Progress Bar (Only visible during download)
        ProgressBar {
            id: downloadProgress
            Layout.fillWidth: true
            from: 0
            to: 100
            value: 0
            visible: false
        }

        // Action Buttons
        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: 20

            Button {
                text: "Check for Updates"
                onClicked: {
                    statusText.text = "Checking..."
                    manager.checkForUpdate()
                }
                enabled: !downloadProgress.visible
            }

            Button {
                text: "Download"
                visible: manager.updateAvailable && !downloadProgress.visible
                onClicked: {
                    downloadProgress.visible = true
                    statusText.text = "Downloading..."
                    manager.downloadUpdate()
                }
            }

            Button {
                id: installBtn
                text: "Install & Reboot"
                visible: false
                onClicked: {
                    statusText.text = "Installing... Do not turn off power."
                    installBtn.enabled = false
                    manager.applyUpdate()
                }
            }
        }
    }

    // Signal Connections
    Connections {
        target: manager

        onCheckComplete: {
            // available (bool), version (string)
            if (!available) {
                statusText.text = "No updates found."
            }
        }

        onDownloadProgress: {
            // bytesReceived (int), bytesTotal (int)
            downloadProgress.visible = true
            if (bytesTotal > 0) {
                downloadProgress.value = (bytesReceived / bytesTotal) * 100
            }
        }

        onDownloadFinished: {
            downloadProgress.visible = false
            statusText.text = "Download Complete. Ready to Install."
            installBtn.visible = true
        }

        onInstallFinished: {
            // success (bool), message (string)
            statusText.text = message
            if (!success) {
                installBtn.enabled = true
            }
        }

        onErrorOccurred: {
            statusText.text = "Error: " + message
            downloadProgress.visible = false
        }
    }
}