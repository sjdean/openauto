import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import JourneyOS

Item {
    id: settingsView
    width: 800
    height: 480

    // -- SETTINGS PAGE THEME -- mapped from Constants for easy future theming
    readonly property color cTextMain: Constants.textOnSettings
    readonly property color cTextDim:  Constants.textDimOnSettings
    readonly property color cTextNote: Constants.textNoteOnSettings
    readonly property color cAccent:   Constants.textOnSettings
    readonly property color cSurface:  Constants.settingsSurface
    readonly property color cBorder:   Constants.settingsBorder
    readonly property color cBase:     Constants.settingsPageBackground

    readonly property int paddingOuter: 30
    readonly property int labelWidth: 280
    readonly property int controlHeight: 40
    readonly property int headerHeight: 70
    readonly property int footerHeight: 70

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
                    {text: "Vehicle", icon: "images/fi-br-car-alt.svg"},
                    {text: "Media", icon: "images/fi-br-desktop-wallpaper.svg"},
                    {text: "Auto", icon: "images/android-auto.svg"},
                    {text: "Audio", icon: "images/fi-br-music-alt.svg"},
                    {text: "Video", icon: "images/fi-br-screen.svg"},
                    {text: "System", icon: "images/fi-br-settings.svg"}
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
                            font.pixelSize: 14
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
                    font.pixelSize: 13
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
                    font.pixelSize: 13
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
                    font.pixelSize: 13
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
                            font.pixelSize: 15
                            font.bold: true
                            color: cTextMain
                        }
                        Label {
                            text: settingsViewHandler.headUnitMode
                                  ? "Full hardware control enabled — Bluetooth, Wi-Fi, and audio devices are managed by this application."
                                  : "System-managed mode — hardware settings are controlled by the host operating system."
                            font.pixelSize: 13
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
                    font.pixelSize: 13
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
                            font.pixelSize: 13
                            color: cTextMain
                        }
                        Label {
                            id: updateStatusLabel
                            text: updateManager.updateAvailable
                                  ? ("Update available: " + updateManager.latestVersion)
                                  : "Up to date"
                            font.pixelSize: 13
                            color: updateManager.updateAvailable ? cAccent : cTextDim
                        }
                    }

                    Button {
                        text: "Check for Updates"
                        Layout.alignment: Qt.AlignVCenter
                        onClicked: {
                            if (!wifiViewModel.connected) {
                                wifiWarningDialog.open()
                            } else {
                                updateManager.checkForUpdate()
                            }
                        }
                    }
                }

            }
        }

        // ── OTA dialogs — declared outside StackLayout so they overlay everything ──
        Dialog {
            id: wifiWarningDialog
            title: "Wi-Fi Required"
            modal: true
            anchors.centerIn: parent
            standardButtons: Dialog.Ok

            Label {
                text: "A Wi-Fi connection is required to check for updates.\nPlease connect to a network first."
                wrapMode: Text.WordWrap
                width: 360
            }
        }

        Dialog {
            id: updateFoundDialog
            title: "Update Available"
            modal: true
            anchors.centerIn: parent
            standardButtons: Dialog.Ok | Dialog.Cancel

            Label {
                text: "Version " + updateManager.latestVersion + " is available.\n\nThe update will be downloaded and applied on next restart."
                wrapMode: Text.WordWrap
                width: 360
            }

            onAccepted: {
                updateManager.downloadUpdate()
                updateManager.applyUpdate()
            }
        }

        // OTA update-found connection — must live in rootRect (not inside SettingsPage)
        // because SettingsPage.userContent only accepts QQuickItem children.
        Connections {
            target: updateManager
            function onCheckComplete(available, version) {
                if (available) updateFoundDialog.open()
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
                        font.pixelSize: 16
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
                        font.pixelSize: 16
                        font.bold: true
                        color: Constants.btnConfirmFg
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    onClicked: {
                        settingsViewHandler.save()
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
        font.pixelSize: Constants.fontSubtitle
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
            font.pixelSize: 15
            color: cTextMain
            Layout.preferredWidth: settingsView.labelWidth
            Layout.alignment: Qt.AlignVCenter
            elide: Text.ElideRight
            height: 20
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
            font.pixelSize: Constants.fontBody
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
        font.pixelSize: Constants.fontBody
        background: Rectangle {
            color: parent.activeFocus ? "#FAFAFA" : cSurface
            border.color: parent.activeFocus ? cAccent : cBorder
            border.width: 1
            radius: Constants.radiusInput
        }
    }

    component ModernCheckBox : CheckBox {
        font.pixelSize: Constants.fontBody
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
        font.pixelSize: Constants.fontBody
        background: Rectangle {
            implicitWidth: 140
            border.color: cBorder
            color: cSurface
            radius: Constants.radiusInput
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

