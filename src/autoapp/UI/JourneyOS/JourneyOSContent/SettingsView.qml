import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import JourneyOS

Item {
    id: settingsView
    width: 800
    height: 480

    // -- THEME CONSTANTS --
    readonly property color cTextMain: "#3a4856"
    readonly property color cTextDim: "#9dadbc"
    readonly property color cAccent: "#3a4856"
    readonly property color cSurface: "#ebfbff"
    readonly property color cBorder: "#3a4856"
    readonly property color cBase: "#a3caed"

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
                color: Qt.rgba(255, 255, 255, 0.5)
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
                    color: cTextDim
                    font.italic: true
                }
                SettingRow {
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
                    color: cTextDim
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
                    color: cTextDim
                    font.italic: true
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }
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
            color: "#D9FFFFFF"
            z: 10 // Ensure it stays on top of the ScrollView

            Rectangle {
                width: parent.width; height: 1; color: "#E0E0E0"; anchors.top: parent.top
            }

            RowLayout {
                anchors.fill: parent
                anchors.margins: 15
                spacing: 20

                Button {
                    text: "Back"
                    Layout.preferredWidth: 120
                    Layout.fillHeight: true
                    background: Rectangle {
                        color: parent.down ? "#EEE" : "transparent"
                        radius: 8
                        border.color: "#CCC"
                        border.width: 1
                    }
                    contentItem: Text {
                        text: parent.text
                        font.pixelSize: 16
                        color: cTextDim
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    onClicked: stackView.pop()
                }

                Item {
                    Layout.fillWidth: true
                }

                Button {
                    text: "Save Changes"
                    Layout.preferredWidth: 180
                    Layout.fillHeight: true
                    background: Rectangle {
                        color: parent.down ? Qt.darker(cAccent, 1.2) : cAccent
                        radius: 8
                    }
                    contentItem: Text {
                        text: parent.text
                        font.pixelSize: 16
                        font.bold: true
                        color: "white"
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
        font.pixelSize: 16
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
            radius: 4
        }
        contentItem: Text {
            leftPadding: 10
            text: parent.displayText
            color: cTextMain
            font.pixelSize: 15
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
                color: cSurface; border.color: cBorder
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
        font.pixelSize: 15
        background: Rectangle {
            color: parent.activeFocus ? "#FAFAFA" : cSurface
            border.color: parent.activeFocus ? cAccent : cBorder
            border.width: 1
            radius: 4
        }
    }

    component ModernCheckBox : CheckBox {
        font.pixelSize: 15
        indicator: Rectangle {
            implicitWidth: 24; implicitHeight: 24
            x: parent.leftPadding
            y: parent.height / 2 - height / 2
            radius: 4
            color: cSurface
            border.color: parent.checked ? cAccent : cBorder
            Rectangle {
                width: 14; height: 14
                anchors.centerIn: parent
                radius: 2
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
        font.pixelSize: 15
        background: Rectangle {
            implicitWidth: 140
            border.color: cBorder
            color: cSurface
            radius: 4
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

