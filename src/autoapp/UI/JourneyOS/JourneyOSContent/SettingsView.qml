import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import JourneyOS

Item {
    id: settingsView
    width: parent ? parent.width : 800
    height: parent ? parent.height : 480

    // -- SETTINGS PAGE THEME -- mapped from Constants for easy future theming
    readonly property color cTextMain: Constants.textOnSettings
    readonly property color cTextDim:  Constants.textDimOnSettings
    readonly property color cTextNote: Constants.textNoteOnSettings
    readonly property color cAccent:   Constants.textOnSettings
    readonly property color cSurface:  Constants.settingsSurface
    readonly property color cBorder:   Constants.settingsBorder
    readonly property color cBase:     Constants.settingsPageBackground

    readonly property int paddingOuter: Constants.settingsPaddingOuter
    readonly property int labelWidth: Constants.settingsLabelWidth
    readonly property int controlHeight: Constants.settingsControlHeight
    readonly property int headerHeight: Constants.settingsHeaderHeight
    readonly property int footerHeight: Constants.settingsFooterHeight

    Rectangle {
        id: rootRect
        color: cBase
        anchors.fill: parent

        // ---------------------------------------------------------
        // 1. TAB BAR
        // ---------------------------------------------------------
        TabBar {
            id: tabBar
            width: parent.width
            height: settingsView.headerHeight
            anchors.top: parent.top
            currentIndex: 0

            background: Rectangle {
                color: Constants.settingsTabBar
            }

            Repeater {
                id: tabRepeater
                model: [
                    {text: "Vehicle", icon: "images/settings-vehicle.svg"},
                    {text: "Media", icon: "images/settings-media.svg"},
                    {text: "Auto", icon: "images/android-auto.svg"},
                    {text: "Audio", icon: "images/settings-audio.svg"},
                    {text: "Video", icon: "images/settings-video.svg"},
                    {text: "System", icon: "images/settings.svg"},
                    {text: "Look", icon: "images/day.svg"},
                    {text: "CAN Bus", icon: "images/settings.svg"}
                ]
                TabButton {
                    id: tabBtn
                    width: Math.max(80, tabBar.width / tabRepeater.count)
                    contentItem: ColumnLayout {
                        spacing: 5
                        Image {
                            source: modelData.icon
                            sourceSize.width: 24; sourceSize.height: 24
                            Layout.alignment: Qt.AlignHCenter
                            opacity: tabBtn.checked ? 1.0 : 0.4
                        }
                        Text {
                            text: modelData.text
                            font.pointSize: 14
                            font.bold: tabBtn.checked
                            color: tabBtn.checked ? cAccent : cTextDim
                            Layout.alignment: Qt.AlignHCenter
                        }
                    }
                    background: Rectangle {
                        color: "transparent"
                    }
                }
            }
        }

        // ---------------------------------------------------------
        // 2. CONTENT AREA
        // ---------------------------------------------------------
        StackLayout {
            id: contentStack
            anchors.top: tabBar.bottom
            anchors.bottom: footerBar.top // This is the crucial missing link
            anchors.left: parent.left
            anchors.right: parent.right
            currentIndex: tabBar.currentIndex

            // --- TAB 0: VEHICLE ---
            SettingsPage {
                SectionHeader {
                    text: "Car Configuration"
                }
                SettingRow {
                    label: "Driving Position"
                    control: ModernComboBox {
                        model: driverPositionModel.comboBoxItems
                        currentIndex: findIndex(model, settingsViewHandler.carDriverPosition)
                        onActivated: settingsViewHandler.carDriverPosition = model[currentIndex].value
                    }
                }
                SettingRow {
                    label: "Fuel Type"
                    control: ModernComboBox {
                        model: fuelTypeModel.comboBoxItems
                        currentIndex: findIndex(model, settingsViewHandler.carFuelType)
                        onActivated: settingsViewHandler.carFuelType = model[currentIndex].value
                    }
                }
                SettingRow {
                    label: "Charging Type"
                    control: ModernComboBox {
                        model: evConnectorTypeModel.comboBoxItems
                        currentIndex: findIndex(model, settingsViewHandler.carEvConnectorType)
                        onActivated: settingsViewHandler.carEvConnectorType = model[currentIndex].value
                    }
                }
                SectionHeader {
                    text: "Identification"
                }
                SettingRow {
                    label: "Car Make"
                    control: ModernTextField {
                        placeholderText: "e.g. Ford"
                        text: settingsViewHandler.carMake
                        onEditingFinished: settingsViewHandler.carMake = text
                    }
                }
                SettingRow {
                    label: "Car Model"
                    control: ModernTextField {
                        placeholderText: "e.g. Mustang"
                        text: settingsViewHandler.carModel
                        onEditingFinished: settingsViewHandler.carModel = text
                    }
                }
                SettingRow {
                    label: "Year"
                    control: ModernTextField {
                        placeholderText: "e.g. 2003"
                        text: settingsViewHandler.carYear
                        onEditingFinished: settingsViewHandler.carYear = text
                    }
                }
            }

            // --- TAB 1: MEDIA ---
            SettingsPage {
                SectionHeader {
                    text: "Playback Behavior"
                }
                ModernCheckBox {
                    checked: settingsViewHandler.mediaAutoPlayback
                    onToggled: settingsViewHandler.mediaAutoPlayback = checked
                    text: qsTr("Auto-play last song on startup")
                }
                ModernCheckBox {
                    checked: settingsViewHandler.mediaAutoStart
                    onToggled: settingsViewHandler.mediaAutoStart = checked
                    text: qsTr("Instant playback on selection")
                }
            }

            // --- TAB 2: ANDROID AUTO ---
            SettingsPage {
                SectionHeader {
                    text: "Audio Channels"
                }
                RowLayout {
                    Layout.leftMargin: settingsView.labelWidth + 20
                    spacing: 30
                    ModernCheckBox {
                        text: "Media"; checked: settingsViewHandler.aaChannelMedia; onToggled: settingsViewHandler.aaChannelMedia = checked
                    }
                    ModernCheckBox {
                        text: "Guidance"; checked: settingsViewHandler.aaChannelGuidance; onToggled: settingsViewHandler.aaChannelGuidance = checked
                    }
                    ModernCheckBox {
                        text: "Telephony"; checked: settingsViewHandler.aaChannelTelephony; onToggled: settingsViewHandler.aaChannelTelephony = checked
                    }
                }

                SectionHeader {
                    text: "Video Settings"
                }
                SettingRow {
                    label: "Frame Rate"
                    control: ModernComboBox {
                        model: frameRateModel.comboBoxItems
                        currentIndex: findIndex(model, settingsViewHandler.aaFrameRate)
                        onActivated: settingsViewHandler.aaFrameRate = model[currentIndex].value
                    }
                }
                SettingRow {
                    label: "Resolution"
                    control: ModernComboBox {
                        model: resolutionModel.comboBoxItems
                        currentIndex: findIndex(model, settingsViewHandler.aaResolution)
                        onActivated: settingsViewHandler.aaResolution = model[currentIndex].value
                    }
                }

                SectionHeader {
                    text: "Margins"
                }
                SettingRow {
                    label: "Video Margin (H/W)"
                    control: RowLayout {
                        spacing: 10
                        ModernSpinBox {
                            from: 0;
                            to: 200; value: settingsViewHandler.videoMarginHeight; onValueModified: settingsViewHandler.videoMarginHeight = value
                        }
                        ModernSpinBox {
                            from: 0;
                            to: 200; value: settingsViewHandler.videoMarginWidth; onValueModified: settingsViewHandler.videoMarginWidth = value
                        }
                    }
                }

                SectionHeader {
                    text: "Home Button Overlay"
                }
                ModernCheckBox {
                    Layout.leftMargin: settingsView.labelWidth + 20
                    text: qsTr("Show home button overlay")
                    checked: settingsViewHandler.androidAutoShowHomeButton
                    onToggled: settingsViewHandler.androidAutoShowHomeButton = checked
                }
                SettingRow {
                    label: "Visibility"
                    control: ModernComboBox {
                        model: [
                            { text: qsTr("Touch to reveal"), value: "touchToReveal" },
                            { text: qsTr("Always visible"),  value: "alwaysVisible"  }
                        ]
                        textRole: "text"
                        currentIndex: {
                            var v = settingsViewHandler.androidAutoHomeButtonVisibility
                            for (var i = 0; i < model.length; ++i)
                                if (model[i].value === v) return i
                            return 0
                        }
                        onActivated: settingsViewHandler.androidAutoHomeButtonVisibility = model[currentIndex].value
                    }
                }
                SettingRow {
                    label: "Position"
                    control: ModernComboBox {
                        // Matches the enum in AndroidAutoView.qml:
                        // 0=TopLeft 1=TopRight 2=BottomLeft 3=BottomRight
                        // 4=TopCentre 5=BottomCentre 6=MiddleLeft 7=MiddleRight
                        model: [
                            { text: qsTr("Top Left"),      value: 0 },
                            { text: qsTr("Top Centre"),    value: 4 },
                            { text: qsTr("Top Right"),     value: 1 },
                            { text: qsTr("Middle Left"),   value: 6 },
                            { text: qsTr("Middle Right"),  value: 7 },
                            { text: qsTr("Bottom Left"),   value: 2 },
                            { text: qsTr("Bottom Centre"), value: 5 },
                            { text: qsTr("Bottom Right"),  value: 3 }
                        ]
                        textRole: "text"
                        currentIndex: {
                            var v = settingsViewHandler.androidAutoHomeButtonPosition
                            for (var i = 0; i < model.length; ++i)
                                if (model[i].value === v) return i
                            return 5 // fallback: Bottom Left
                        }
                        onActivated: settingsViewHandler.androidAutoHomeButtonPosition = model[currentIndex].value
                    }
                }
            }

            // --- TAB 3: AUDIO ---
            SettingsPage {
                SectionHeader {
                    text: "Output"
                }
                // Device selection only available in Head Unit mode
                SettingRow {
                    visible: ConfigGate.showConfig
                    label: "Device"
                    control: ModernComboBox {
                        model: pulseAudioDeviceModelOutput.comboBoxItems
                        currentIndex: findIndex(model, settingsViewHandler.audioPlaybackDevice)
                        onActivated: settingsViewHandler.audioPlaybackDevice = model[currentIndex].value
                    }
                }
                Label {
                    visible: !ConfigGate.showConfig
                    text: "Audio output device is managed by the operating system."
                    font.pointSize: 13
                    color: cTextNote
                    font.italic: true
                }
                SettingRow {
                    visible: ConfigGate.showConfig
                    label: "Volume Limit"
                    control: RangeSlider {
                        Layout.fillWidth: true
                        from: 0
                        to: 255
                        first.value: settingsViewHandler.audioVolumePlaybackMin
                        second.value: settingsViewHandler.audioVolumePlaybackMax
                        first.onMoved: settingsViewHandler.audioVolumePlaybackMin = first.value
                        second.onMoved: settingsViewHandler.audioVolumePlaybackMax = second.value
                    }
                }
                Label {
                    visible: !ConfigGate.showConfig
                    text: "Volume limits are managed by the operating system."
                    font.pointSize: 13
                    color: cTextNote
                    font.italic: true
                }

                SectionHeader {
                    text: "Input"
                }
                SettingRow {
                    visible: ConfigGate.showConfig
                    label: "Microphone"
                    control: ModernComboBox {
                        model: pulseAudioDeviceModelInput.comboBoxItems
                        currentIndex: findIndex(model, settingsViewHandler.audioCaptureDevice)
                        onActivated: settingsViewHandler.audioCaptureDevice = model[currentIndex].value
                    }
                }
                Label {
                    visible: !ConfigGate.showConfig
                    text: "Audio input device is managed by the operating system."
                    font.pointSize: 13
                    color: cTextNote
                    font.italic: true
                }
            }

            // --- TAB 4: VIDEO ---
            SettingsPage {
                SectionHeader {
                    text: "Screen"
                }
                SettingRow {
                    label: "Rotate 180°"
                    control: Switch {
                        checked: settingsViewHandler.videoRotateDisplay
                        onToggled: settingsViewHandler.videoRotateDisplay = checked
                    }
                }
            }

            // --- TAB 5: SYSTEM ---
            // TODO (4.2): Add "System Information" section below Device Mode:
            //   - Device name / hostname
            //   - JourneyOS version + build date  (from updateManager.currentVersion + build timestamp)
            //   - Kernel / OS version
            //   - Hardware profile summary (display type, audio HAT, CAN bus, GPS)
            SettingsPage {
                SectionHeader {
                    text: "Device Mode"
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 16

                    Column {
                        spacing: 4
                        Layout.fillWidth: true

                        Label {
                            text: "Head Unit Mode"
                            font.pointSize: 15
                            font.bold: true
                            color: cTextMain
                        }
                        Label {
                            text: settingsViewHandler.headUnitMode
                                  ? "Full hardware control enabled — Bluetooth, Wi-Fi, and audio devices are managed by this application."
                                  : "System-managed mode — hardware settings are controlled by the host operating system."
                            font.pointSize: 13
                            color: cTextDim
                            wrapMode: Text.WordWrap
                            width: parent.width
                        }
                    }

                    Switch {
                        id: headUnitSwitch
                        // On Linux this is freely toggleable; on Mac/Windows it is locked off.
                        enabled: ConfigGate.showConfig
                        checked: settingsViewHandler.headUnitMode
                        onToggled: settingsViewHandler.headUnitMode = checked
                    }
                }

                Label {
                    visible: !ConfigGate.showConfig
                    text: "Head Unit Mode is not available on this platform. Hardware settings are always managed by the operating system."
                    font.pointSize: 13
                    color: cTextNote
                    font.italic: true
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }

                // ── OTA Updates (platform-gated) ──────────────────────────────
                SectionHeader {
                    visible: ConfigGate.showConfig
                    text: "Software Updates"
                }

                RowLayout {
                    visible: ConfigGate.showConfig
                    Layout.fillWidth: true
                    spacing: 16

                    Column {
                        spacing: 4
                        Layout.fillWidth: true

                        Label {
                            text: "Current version: " + updateManager.currentVersion
                            font.pointSize: 13
                            color: cTextMain
                        }
                        Label {
                            id: updateStatusLabel
                            text: updateManager.updateAvailable
                                  ? ("Update available: " + updateManager.latestVersion)
                                  : "Up to date"
                            font.pointSize: 13
                            color: updateManager.updateAvailable ? cAccent : cTextDim
                        }
                    }

                    ModernButton {
                        text: "Check for Updates"
                        Layout.alignment: Qt.AlignVCenter
                        onClicked: updateManager.checkForUpdate()
                    }
                }

            }

            // --- TAB 6: LOOK ---
            SettingsPage {
                SectionHeader { text: "Appearance" }

                SettingRow {
                    label: "Primary Accent"
                    control: RowLayout {
                        // anchors.fill: parent is unreliable after reparenting via children alias;
                        // use explicit property bindings which rebind on parentChanged instead.
                        spacing: 10
                        ModernTextField {
                            id: primaryAccentField
                            Layout.fillWidth: true
                            text: settingsViewHandler.uiAccentPrimary
                            placeholderText: "e.g. #DEB3FF  (blank = default)"
                            onEditingFinished: settingsViewHandler.uiAccentPrimary = text.trim()
                        }
                        Rectangle {
                            width: 32; height: 32; radius: Constants.radiusInput
                            color: primaryAccentField.text !== "" ? primaryAccentField.text : Constants.palettePrimary
                            border.color: cBorder; border.width: 1
                        }
                        ModernButton {
                            text: "Reset"
                            onClicked: { primaryAccentField.text = ""; settingsViewHandler.uiAccentPrimary = "" }
                        }
                    }
                }

                SettingRow {
                    label: "Secondary Accent"
                    control: RowLayout {
                        spacing: 10
                        ModernTextField {
                            id: brand2AccentField
                            Layout.fillWidth: true
                            text: settingsViewHandler.uiAccentBrand2
                            placeholderText: "e.g. #7ED6FF  (blank = default)"
                            onEditingFinished: settingsViewHandler.uiAccentBrand2 = text.trim()
                        }
                        Rectangle {
                            width: 32; height: 32; radius: Constants.radiusInput
                            color: brand2AccentField.text !== "" ? brand2AccentField.text : Constants.paletteSecondary
                            border.color: cBorder; border.width: 1
                        }
                        ModernButton {
                            text: "Reset"
                            onClicked: { brand2AccentField.text = ""; settingsViewHandler.uiAccentBrand2 = "" }
                        }
                    }
                }

                SettingRow {
                    label: "Button Opacity"
                    control: RowLayout {
                        spacing: 10
                        Slider {
                            Layout.fillWidth: true
                            from: 0.0; to: 1.0; stepSize: 0.01
                            value: settingsViewHandler.uiButtonOpacity
                            onMoved: settingsViewHandler.uiButtonOpacity = value
                        }
                        Label {
                            text: Math.round(settingsViewHandler.uiButtonOpacity * 100) + " %"
                            color: cTextMain
                            font.pointSize: Constants.fontBody
                            Layout.preferredWidth: 44
                            horizontalAlignment: Text.AlignRight
                        }
                    }
                }
            }

            // --- TAB 7: CAN BUS ---
            SettingsPage {

                SectionHeader { text: "Signal Mapping File" }

                SettingRow {
                    label: "Mapping File"
                    control: RowLayout {
                        spacing: 10
                        ModernTextField {
                            id: mappingFileField
                            Layout.fillWidth: true
                            text: settingsViewHandler.canBusMappingFile
                            placeholderText: "e.g. /home/pi/.local/share/journeyos/canbus/renault.json"
                            onEditingFinished: settingsViewHandler.canBusMappingFile = text.trim()
                        }
                        ModernButton {
                            text: "Browse…"
                            onClicked: mappingLibrarySheet.open()
                        }
                    }
                }

                Label {
                    text: "Point to the JSON signal mapping file for your vehicle. "
                        + "The file will be sent to connected CAN bus peripherals on next connection."
                    font.pointSize: 13
                    color: cTextDim
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }

                // ── Community map library sheet ───────────────────────────────
                Popup {
                    id: mappingLibrarySheet
                    modal: true
                    anchors.centerIn: parent
                    width: Math.min(560, parent.width - 40)
                    height: Math.min(500, parent.height - 40)
                    padding: 20

                    background: Rectangle {
                        color: cSurface
                        radius: Constants.radiusInput
                        border.color: cBorder
                        border.width: 1
                    }

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 12

                        Label {
                            text: "Community Mapping Library"
                            font.pointSize: 16
                            font.bold: true
                            color: cTextMain
                        }

                        RowLayout {
                            spacing: 8
                            Layout.fillWidth: true

                            ModernTextField {
                                id: libSearchMake
                                Layout.fillWidth: true
                                placeholderText: "Make (e.g. Renault)"
                                text: settingsViewHandler.carMake
                            }
                            ModernTextField {
                                id: libSearchModel
                                Layout.fillWidth: true
                                placeholderText: "Model (e.g. Laguna)"
                                text: settingsViewHandler.carModel
                            }
                            ModernTextField {
                                id: libSearchYear
                                Layout.preferredWidth: 90
                                placeholderText: "Year"
                                text: settingsViewHandler.carYear
                                inputMethodHints: Qt.ImhDigitsOnly
                            }
                            ModernButton {
                                text: "Search"
                                onClicked: {
                                    if (mappingLibrary)
                                        mappingLibrary.search(
                                            libSearchMake.text.trim(),
                                            libSearchModel.text.trim(),
                                            parseInt(libSearchYear.text) || 0)
                                }
                            }
                        }

                        Label {
                            text: mappingLibrary ? mappingLibrary.statusMessage : ""
                            font.pointSize: 12
                            color: cTextDim
                            wrapMode: Text.WordWrap
                            Layout.fillWidth: true
                            visible: text.length > 0
                        }

                        BusyIndicator {
                            visible: mappingLibrary && mappingLibrary.loading
                            running: visible
                            Layout.alignment: Qt.AlignHCenter
                        }

                        ListView {
                            id: libResultsView
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            clip: true
                            model: mappingLibrary ? mappingLibrary.results : []
                            spacing: 6

                            delegate: Rectangle {
                                width: libResultsView.width
                                height: libEntryCol.implicitHeight + 16
                                color: cBackground
                                radius: Constants.radiusInput
                                border.color: cBorder
                                border.width: 1

                                Column {
                                    id: libEntryCol
                                    anchors { left: parent.left; right: parent.right;
                                              top: parent.top; margins: 8 }
                                    spacing: 4

                                    RowLayout {
                                        width: parent.width
                                        spacing: 6

                                        Label {
                                            text: modelData.name || (modelData.make + " " + modelData.model)
                                            font.pointSize: 13
                                            font.bold: true
                                            color: cTextMain
                                            Layout.fillWidth: true
                                        }

                                        Label {
                                            visible: modelData.validated === true
                                            text: "✓ Validated"
                                            font.pointSize: 11
                                            color: "#4CAF50"
                                        }
                                    }

                                    Label {
                                        text: {
                                            let s = modelData.year_from + "–" + modelData.year_to
                                            if (modelData.platform) s += "  ·  " + modelData.platform
                                            return s
                                        }
                                        font.pointSize: 12
                                        color: cTextDim
                                    }

                                    ModernButton {
                                        text: "Download & Use"
                                        onClicked: {
                                            if (mappingLibrary)
                                                mappingLibrary.download(index)
                                            mappingLibrarySheet.close()
                                        }
                                    }
                                }
                            }
                        }

                        ModernButton {
                            text: "Close"
                            Layout.alignment: Qt.AlignRight
                            onClicked: mappingLibrarySheet.close()
                        }
                    }
                }

                SectionHeader { text: "Connected Devices" }

                Repeater {
                    model: canBusDeviceModel.devices

                    delegate: Rectangle {
                        Layout.fillWidth: true
                        height: deviceColumn.implicitHeight + 24
                        color: cSurface
                        radius: Constants.radiusInput
                        border.color: cBorder
                        border.width: 1

                        Column {
                            id: deviceColumn
                            anchors { left: parent.left; right: parent.right; top: parent.top; margins: 12 }
                            spacing: 6

                            RowLayout {
                                width: parent.width
                                spacing: 8

                                Label {
                                    text: modelData.deviceId
                                    font.pointSize: 14
                                    font.bold: true
                                    color: cTextMain
                                    Layout.fillWidth: true
                                }
                                Label {
                                    text: modelData.status === "ok" ? "Ready" : "Updating…"
                                    font.pointSize: 12
                                    color: modelData.status === "ok" ? "#4CAF50" : cTextDim
                                }
                            }

                            Label {
                                text: "Firmware: " + modelData.firmwareVersion
                                font.pointSize: 12
                                color: cTextDim
                            }

                            Repeater {
                                model: modelData.canPorts
                                delegate: Label {
                                    text: "\u2022 Port " + modelData.portId
                                        + " — " + (modelData.bitrate / 1000) + " kbit/s"
                                        + (modelData.fdCapable ? "  FD" : "")
                                        + (modelData.linkActive ? "  \u25CF" : "  \u25CB")
                                    font.pointSize: 12
                                    color: modelData.linkActive ? cTextMain : cTextDim
                                }
                            }
                        }
                    }
                }

                Label {
                    visible: canBusDeviceModel.devices.length === 0
                    text: "No CAN bus peripherals connected."
                    font.pointSize: 13
                    font.italic: true
                    color: cTextDim
                    Layout.fillWidth: true
                }
            }
        }

        // ── OTA dialogs — declared outside StackLayout so they overlay everything ──
        Dialog {
            id: updateFoundDialog
            title: "Update Available"
            modal: true
            anchors.centerIn: parent
            standardButtons: Dialog.Ok | Dialog.Cancel

            Label {
                text: "Version " + updateManager.latestVersion + " is available.\n\nDownload and install now? The device will reboot automatically."
                wrapMode: Text.WordWrap
                width: Math.min(360, parent.width - 40)
            }

            onAccepted: {
                downloadingDialog.open()
                updateManager.downloadUpdate()
                // applyUpdate() is triggered by onDownloadFinished below
            }
        }

        // Shown while the bundle is downloading — not dismissable.
        Dialog {
            id: downloadingDialog
            title: "Downloading Update"
            modal: true
            closePolicy: Popup.NoAutoClose
            anchors.centerIn: parent
            standardButtons: Dialog.NoButton

            Column {
                spacing: 12
                width: Math.min(360, parent.width - 40)
                Label {
                    text: "Downloading version " + updateManager.latestVersion + "…"
                    wrapMode: Text.WordWrap
                    width: parent.width
                }
                ProgressBar {
                    id: downloadProgressBar
                    width: parent.width
                    from: 0; to: 1; value: 0
                }
                Label {
                    id: downloadProgressLabel
                    text: "0 %"
                    font.pointSize: 12
                    color: cTextDim
                }
            }
        }

        // Shown while RAUC applies the bundle — not dismissable.
        Dialog {
            id: installingDialog
            title: "Installing Update"
            modal: true
            closePolicy: Popup.NoAutoClose
            anchors.centerIn: parent
            standardButtons: Dialog.NoButton

            Label {
                text: "Installing update. Please do not power off the device.\nThe system will reboot automatically when done."
                wrapMode: Text.WordWrap
                width: Math.min(360, parent.width - 40)
            }
        }

        // Shown on download error, network error, or install failure.
        Dialog {
            id: otaErrorDialog
            title: "Update Failed"
            modal: true
            anchors.centerIn: parent
            standardButtons: Dialog.Ok

            Label {
                id: otaErrorLabel
                text: ""
                wrapMode: Text.WordWrap
                width: Math.min(360, parent.width - 40)
            }
        }

        // OTA signal connections — must live in rootRect (not inside SettingsPage)
        // because SettingsPage.userContent only accepts QQuickItem children.
        Connections {
            target: updateManager

            function onCheckComplete(available, version) {
                if (available) updateFoundDialog.open()
            }

            function onDownloadProgress(bytesReceived, bytesTotal) {
                if (bytesTotal > 0) {
                    const frac = bytesReceived / bytesTotal
                    downloadProgressBar.value = frac
                    downloadProgressLabel.text = Math.round(frac * 100) + " %"
                }
            }

            function onDownloadFinished() {
                downloadingDialog.close()
                installingDialog.open()
                updateManager.applyUpdate()
            }

            function onInstallFinished(success, message) {
                installingDialog.close()
                if (!success) {
                    otaErrorLabel.text = message
                    otaErrorDialog.open()
                }
                // On success the C++ side has already called systemctl reboot.
            }

            function onErrorOccurred(message) {
                downloadingDialog.close()
                installingDialog.close()
                otaErrorLabel.text = message
                otaErrorDialog.open()
            }
        }

        // ---------------------------------------------------------
        // 3. FLOATING GLASS FOOTER
        // ---------------------------------------------------------
        Rectangle {
            id: footerBar
            height: settingsView.footerHeight
            width: parent.width
            anchors.bottom: parent.bottom
            color: Constants.settingsFooter
            z: 10

            Rectangle {
                width: parent.width; height: 1; color: Constants.settingsFooterBorder; anchors.top: parent.top
            }

            RowLayout {
                anchors.fill: parent
                anchors.margins: 15
                spacing: 20

                // Cancel — red outline, discards changes
                Button {
                    text: "Cancel"
                    Layout.preferredWidth: 120
                    Layout.fillHeight: true
                    background: Rectangle {
                        color: "transparent"
                        radius: 8
                        border.color: Constants.btnCancelBorder
                        border.width: 2
                        opacity: parent.down ? 0.6 : 1.0
                    }
                    contentItem: Text {
                        text: parent.text
                        font.pointSize: 16
                        color: Constants.btnCancelFg
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        opacity: parent.parent.down ? 0.6 : 1.0
                    }
                    onClicked: stackView.pop()
                }

                Item {
                    Layout.fillWidth: true
                }

                // Save Changes — green confirm, writes settings
                Button {
                    text: "Save Changes"
                    Layout.preferredWidth: 180
                    Layout.fillHeight: true
                    background: Rectangle {
                        color: parent.down ? Constants.btnConfirmBgPressed : Constants.btnConfirmBg
                        radius: 8
                    }
                    contentItem: Text {
                        text: parent.text
                        font.pointSize: 16
                        font.bold: true
                        color: Constants.btnConfirmFg
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    onClicked: {
                        // Commit any text field edits that didn't trigger onEditingFinished
                        // (e.g. when the user types then immediately clicks Save)
                        // Each setter calls configuration->save() internally — no explicit save needed.
                        settingsViewHandler.uiAccentPrimary = primaryAccentField.text.trim()
                        settingsViewHandler.uiAccentBrand2  = brand2AccentField.text.trim()
                        stackView.pop()
                    }
                }
            }
        }
    }

    // ---------------------------------------------------------
    // 4. COMPONENTS
    // ---------------------------------------------------------

    component SettingsPage : ScrollView {
        id: pageRoot
        default property alias userContent: innerColumn.children

        clip: true
        anchors.fill: parent

        // No need to manually set contentHeight — Flickable does it automatically with Column

        Flickable {
            id: flickable
            contentWidth: innerColumn.width
            contentHeight: innerColumn.height

            Column {
                id: innerColumn
                width: flickable.availableWidth  // This uses the full available width correctly
                leftPadding: settingsView.paddingOuter
                rightPadding: settingsView.paddingOuter
                topPadding: settingsView.paddingOuter
                bottomPadding: settingsView.paddingOuter
                spacing: 20

                // Children (your SettingRow, SectionHeader, etc.) go here via default property
            }
        }

        ScrollBar.vertical: ScrollBar {
        }
    }

    component SectionHeader : Label {
        font.pointSize: Constants.fontSubtitle
        font.bold: true
        font.capitalization: Font.AllUppercase
        color: cTextMain
        Layout.topMargin: 10
        Layout.bottomMargin: 5
        Layout.fillWidth: true

        bottomPadding: 5
        Rectangle {
            width: parent.width
            height: 1
            color: cBorder
            anchors.bottom: parent.bottom
        }
    }

    component SettingRow : RowLayout {
        id: rowRoot
        property alias label: labelText.text
        property alias labelItem: labelText
        property alias control: controlContainer.children
        spacing: 10
        Layout.fillWidth: true
        Layout.preferredHeight: settingsView.controlHeight

        Label {
            id: labelText
            text: rowRoot.label // Explicitly point to the property
            font.pointSize: 15
            color: cTextMain
            Layout.preferredWidth: settingsView.labelWidth
            Layout.alignment: Qt.AlignVCenter
            elide: Text.ElideRight
            height: implicitHeight
        }

        Item {
            id: controlContainer
            Layout.fillWidth: true
            Layout.preferredHeight: settingsView.controlHeight
            Layout.alignment: Qt.AlignVCenter
        }
    }

    component ModernComboBox : ComboBox {
        id: control
        Layout.fillWidth: true
        Layout.preferredHeight: settingsView.controlHeight
        textRole: "display"

        background: Rectangle {
            color: parent.down ? "#EEE" : cSurface
            border.color: parent.activeFocus ? cAccent : cBorder
            border.width: 1
            radius: Constants.radiusInput
        }
        contentItem: Text {
            leftPadding: 10
            text: parent.displayText
            color: cTextMain
            font.pointSize: Constants.fontBody
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
        popup: Popup {
            y: control.height - 1
            width: control.width
            implicitHeight: Math.min(contentList.contentHeight, 300)
            padding: 1
            contentItem: ListView {
                id: contentList
                clip: true
                implicitHeight: contentHeight
                model: control.delegateModel
                currentIndex: control.highlightedIndex
                ScrollIndicator.vertical: ScrollIndicator {
                }
            }
            background: Rectangle {
                color: cSurface; border.color: cBorder; radius: Constants.radiusInput
            }
        }
        delegate: ItemDelegate {
            width: control.width
            text: model.display
            highlighted: ListView.isCurrentItem
            background: Rectangle {
                color: highlighted ? "#E0F7FA" : cSurface
            }
            contentItem: Text {
                text: parent.text
                color: highlighted ? cAccent : cTextMain
                padding: 10
            }
            onClicked: {
                control.currentIndex = index
                control.popup.close()
                control.activated(index)
            }
        }
    }

    component ModernTextField : TextField {
        Layout.fillWidth: true
        Layout.preferredHeight: settingsView.controlHeight
        color: cTextMain
        font.pointSize: Constants.fontBody
        background: Rectangle {
            color: parent.activeFocus ? "#FAFAFA" : cSurface
            border.color: parent.activeFocus ? cAccent : cBorder
            border.width: 1
            radius: Constants.radiusInput
        }
    }

    component ModernCheckBox : CheckBox {
        font.pointSize: Constants.fontBody
        indicator: Rectangle {
            implicitWidth: 24; implicitHeight: 24
            x: parent.leftPadding
            y: parent.height / 2 - height / 2
            radius: Constants.radiusInput
            color: cSurface
            border.color: parent.checked ? cAccent : cBorder
            Rectangle {
                width: 14; height: 14
                anchors.centerIn: parent
                radius: Constants.radiusInput - 1
                color: cAccent
                visible: parent.parent.checked
            }
        }
        contentItem: Text {
            text: parent.text
            font: parent.font
            color: cTextMain
            verticalAlignment: Text.AlignVCenter
            leftPadding: parent.indicator.width + parent.spacing
        }
    }

    component ModernSpinBox : SpinBox {
        id: rootSpin
        Layout.preferredHeight: settingsView.controlHeight
        font.pointSize: Constants.fontBody
        Layout.fillWidth: true

        background: Rectangle {
            border.color: cBorder
            color: cSurface
            radius: Constants.radiusInput
        }

        // ── Increment (+) button ────────────────────────────────────────────
        up.indicator: Rectangle {
            x: rootSpin.mirrored ? 0 : parent.width - width
            height: parent.height
            implicitWidth: settingsView.controlHeight
            implicitHeight: settingsView.controlHeight
            color: rootSpin.up.pressed ? Qt.darker(cSurface, 1.25) : cSurface
            border.color: cBorder
            border.width: 1
            radius: Constants.radiusInput
            Text {
                text: "+"
                font.pointSize: Constants.fontBody + 2
                font.bold: true
                color: cTextMain
                anchors.centerIn: parent
            }
        }

        // ── Decrement (−) button ────────────────────────────────────────────
        down.indicator: Rectangle {
            x: rootSpin.mirrored ? parent.width - width : 0
            height: parent.height
            implicitWidth: settingsView.controlHeight
            implicitHeight: settingsView.controlHeight
            color: rootSpin.down.pressed ? Qt.darker(cSurface, 1.25) : cSurface
            border.color: cBorder
            border.width: 1
            radius: Constants.radiusInput
            Text {
                text: "−"
                font.pointSize: Constants.fontBody + 2
                font.bold: true
                color: cTextMain
                anchors.centerIn: parent
            }
        }

        // ── Value display ───────────────────────────────────────────────────
        contentItem: TextInput {
            z: 2
            text: rootSpin.textFromValue(rootSpin.value, rootSpin.locale)
            font: rootSpin.font
            color: cTextMain
            selectionColor: cAccent
            selectedTextColor: "#ffffff"
            horizontalAlignment: Qt.AlignHCenter
            verticalAlignment: Qt.AlignVCenter
            readOnly: !rootSpin.editable
            validator: rootSpin.validator
            inputMethodHints: Qt.ImhFormattedNumbersOnly
        }
    }

    // Styled action button — used for in-row actions (Reset, Check for Updates)
    component ModernButton : Button {
        Layout.preferredHeight: settingsView.controlHeight
        background: Rectangle {
            color: parent.down ? Qt.darker(cSurface, 1.25) : cSurface
            border.color: cBorder
            border.width: 1
            radius: Constants.radiusInput
        }
        contentItem: Text {
            text: parent.text
            font.pointSize: Constants.fontBody
            font.bold: true
            color: cTextMain
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }

    function findIndex(model, currentValue) {
        if (!model) return 0;
        for (let i = 0; i < model.length; i++) {
            if (model[i].value === currentValue) return i;
        }
        return 0;
    }
}
