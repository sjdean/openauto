// SettingsView.qml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: settingsView
    width: 800
    height: 480

    Rectangle {
        id: displayRectangle
        color: "#a3caed"
        anchors.fill: parent

        TabBar {
            id: tabBar
            height: 60
            currentIndex: 2
            width: settingsView.width

            TabButton {
                text: "Vehicle"
                icon.source: "images/fi-br-car-alt.svg"
                display: AbstractButton.TextUnderIcon
            }

            TabButton {
                text: "Media Player"
                icon.source: "images/fi-br-desktop-wallpaper.svg"
                display: AbstractButton.TextUnderIcon
            }


            TabButton {
                text: "AndroidAuto"
                icon.source: "images/android-auto.svg"
                display: AbstractButton.TextUnderIcon
            }

            TabButton {
                text: "Audio"
                icon.source: "images/fi-br-music-alt.svg"
                display: AbstractButton.TextUnderIcon
            }

            TabButton {
                text: "Video"
                icon.source: "images/fi-br-screen.svg"
                display: AbstractButton.TextUnderIcon
            }
        }

        StackLayout {
            width: settingsView.width - 20
            height: settingsView.height - tabBar.height - 20
            anchors.left: parent.left
            anchors.top: tabBar.bottom
            anchors.leftMargin: 10
            anchors.topMargin: 10
            anchors.horizontalCenter: parent.horizontalCenter
            currentIndex: tabBar.currentIndex

            Column {
                id: vehicleTab
                Layout.fillHeight: true
                Layout.fillWidth: true
                spacing: 10

                Row {
                    spacing: 10

                    Text {
                        text: "Driving Position"
                        font.pixelSize: 15
                    }
                    ComboBox {
                        id: drivingPosition
                        model: ["Left", "Right", "Centre"]
                    }

                }

                Row {
                    spacing: 10
                    Text {
                        text: "Fuel Type"
                        font.pixelSize: 15
                    }
                    ComboBox {
                        id: fuelType
                        model: ["Unleaded", "Leaded", "Diesel", "Electric", "Biodiesel", "E85", "LPG","CNG", "LNG", "Electric"]
                    }
                }

                Row {
                    spacing: 10
                    Text {
                        text: "Electric Charging Type"
                        font.pixelSize: 15
                    }
                    ComboBox {
                        id: electricChargingType
                        model: ["J1772", "Mennekes", "Chadeno", "Tesla Supercharger", "GBT", "Other"]
                    }
                }

                Row {
                    spacing: 10
                    Text {
                        text: "Car Make"
                        font.pixelSize: 15
                    }

                    TextField {
                        id: carMaketext
                        placeholderText: qsTr("Text Field")
                    }

                }

                Row {
                    spacing: 10
                    Text {
                        text: "Car Model"
                        font.pixelSize: 15
                    }

                    TextField {
                        id: carModelText
                        placeholderText: qsTr("Text Field")
                    }

                }

                // Example of navigating back to the main view
                Button {
                    text: "Back to Main"
                    onClicked: {
                        // Assuming the parent window has a stackView named 'stackView'
                        stackView.pop()
                    }
                }
            }

            Column {
                id: mediaPlayerTab
                Layout.fillHeight: true
                Layout.fillWidth: true
                spacing: 10

                Row {
                    spacing: 10
                    CheckBox {
                        id: autoPlayback
                        text: qsTr("Auto play last song on playback")
                    }
                }

                Row {
                    spacing: 10
                    CheckBox {
                        id: autoStart
                        text: qsTr("Instant playback on track/album select")
                    }
                }
            }


            Column {
                id: androidAutoTab
                Layout.fillHeight: true
                Layout.fillWidth: true
                spacing: 10

                Text {
                    text: qsTr("Enable Audio Channels")
                    font.pixelSize: 12
                    font.styleName: "Bold"
                    font.bold: true
                }
                Row {
                    spacing: 10

                    CheckBox {
                        id: checkBox
                        text: qsTr("Media")
                    }

                    CheckBox {
                        id: checkBox1
                        text: qsTr("Guidance")
                    }

                    CheckBox {
                        id: checkBox2
                        text: qsTr("Telephony")
                    }

                }
                Text {
                    text: qsTr("Audio Output")
                    font.pixelSize: 12
                    font.styleName: "Bold"
                    font.bold: true
                }
                Row {
                    spacing: 10
                    RadioButton {
                        id: radioAudioRT
                        text: qsTr("RT Audio")
                    }

                    RadioButton {
                        id: radioAudioQT
                        text: qsTr("Qt")
                    }

                }

                Row {
                    Text {
                        text: "Frame Rate"
                        font.pixelSize: 15
                    }
                    spacing: 10
                    ComboBox {
                        id: comboFrameRate
                        flat: false
                        displayText: ""
                        textRole: ""
                    }

                }
                  Row {
                    spacing: 10
                    Text {
                        text: "Resolution"
                        font.pixelSize: 15
                    }
                    ComboBox {
                        id: comboResolution
                        flat: false
                        displayText: ""
                        textRole: ""
                    }
                }

                Row {
                    spacing: 10
                    Text {
                        text: "OMX Layer"
                        font.pixelSize: 15
                    }
                    SpinBox {
                        id: spinOMXLayer
                    }
                }
                Text {
                    text: qsTr("Margin")
                    font.pixelSize: 12
                    font.styleName: "Bold"
                    font.bold: true
                }
                Row {
                    spacing: 10
                    Text {
                        text: "Height"
                        font.pixelSize: 15
                    }
                    SpinBox {
                        id: spinMarginHeight
                    }
                    Text {
                        text: "Length"
                        font.pixelSize: 15
                    }
                    SpinBox {
                        id: spinMarginWeight
                    }
                }
                Row {
                    spacing: 10
                    Text {
                        text: "DPI"
                        font.pixelSize: 15
                    }
                    Slider {
                        id: sliderDPI
                        value: 0.5
                    }

                }
            }

            Column {
                id: audioTab
                Layout.fillHeight: true
                Layout.fillWidth: true
                spacing: 10
                Row {
                    spacing: 10
                    Text {
                        text: "Playback Volume"
                        font.pixelSize: 15
                    }
                    Slider {
                        id: sliderVolumePlayback
                        value: 0.5
                    }

                }
                Row {
                    spacing: 10
                    Text {
                        text: "Capture Volume"
                        font.pixelSize: 15
                    }
                    Slider {
                        id: sliderVolumeCapture
                        value: 0.5
                    }

                }
                Row {
                    spacing: 10
                    Text {
                        text: "Output Device"
                        font.pixelSize: 15
                    }
                    ComboBox {
                        id: comboAudioOutputDevkce
                    }

                }
                Row {
                    spacing: 10
                    Text {
                        text: "Input Device"
                        font.pixelSize: 15
                    }
                    ComboBox {
                        id: comboAudioInputDevice
                    }

                }
            }

            Column {
                id: videoTab
                Layout.fillHeight: true
                Layout.fillWidth: true
                spacing: 10

                Text {
                    id: videoTabHader
                    text: qsTr("Screen Brightness")
                    font.pixelSize: 12
                    font.styleName: "Bold"
                    font.bold: true
                }

                Row {

                    width: 800
                    height: 40
                    spacing: 10
                    Text {
                        text: "Current"
                        font.pixelSize: 15
                    }
                     Slider {
                        id: currentBrightness
                        from: 0
                        to: 100
                        value: 50
                    }
                }

                Row {
                    width: 800
                    height: 40
                    spacing: 10
                    Text {
                        text: "Day Min/Max"
                        font.pixelSize: 15
                    }
                    RangeSlider {
                        id: brightnessRangeDay
                        from: 0
                        to: 100
                    }
                }


                Row {
                    width: 800
                    height: 40
                    spacing: 10
                    Text {
                        text: "Night Min/Max"
                        font.pixelSize: 15
                    }
                    RangeSlider {
                        id: brightnessRangeNight
                        from: 0
                        to: 100
                    }

                }

                Text {
                    id: _text2
                    text: qsTr("Graphics Render Mode")
                    font.pixelSize: 12
                    font.weight: Font.Bold
                }

                Row {
                    spacing: 10
                    RadioButton {
                        id: radioEGL

                        text: qsTr("EGL")
                        checked: true
                    }

                    RadioButton {
                        id: radioX11
                        text: qsTr("X11")
                    }
                }


                Text {
                    id: _text3
                    text: qsTr("Rotate Screen 180 degrees?")
                    font.pixelSize: 12
                    font.styleName: "Bold"
                }

                Row {
                    spacing: 10
                    Switch {
                        id: switchRotate
                        text: qsTr("Rotate Display")
                    }

                }




            }

        }

    }



}
