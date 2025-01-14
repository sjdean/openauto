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
            currentIndex: 3
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
                        id: driverPosition
                        model: driverPositionModel.comboBoxItems
                        currentIndex: driverPositionModel.comboBoxItems.indexOf(driverPositionModel.currentComboBoxItem)
                        onCurrentIndexChanged: {
                            driverPositionModel.currentComboBoxItem = driverPositionModel.comboBoxItems[currentIndex];
                        }
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
                        model: fuelTypeModel.comboBoxItems
                        currentIndex: fuelTypeModel.comboBoxItems.indexOf(fuelTypeModel.currentComboBoxItem)
                        onCurrentIndexChanged: {
                            fuelTypeModel.currentComboBoxItem = fuelTypeModel.comboBoxItems[currentIndex];
                        }
                    }
                }

                Row {
                    spacing: 10
                    Text {
                        text: "Electric Charging Type"
                        font.pixelSize: 15
                    }
                    ComboBox {
                        id: evConnectorType
                        model: evConnectorTypeModel.comboBoxItems
                        currentIndex: evConnectorTypeModel.comboBoxItems.indexOf(evConnectorTypeModel.currentComboBoxItem)
                        onCurrentIndexChanged: {
                            evConnectorTypeModel.currentComboBoxItem = evConnectorTypeModel.comboBoxItems[currentIndex];
                        }
                    }
                }

                Row {
                    spacing: 10
                    Text {
                        text: "Car Make"
                        font.pixelSize: 15
                    }

                    TextField {
                        id: carMake
                        placeholderText: qsTr("Car Make")
                        text: settingsViewHandler.carMake
                        onTextChanged: settingsViewHandler.carMake = text
                    }

                }

                Row {
                    spacing: 10
                    Text {
                        text: "Car Model"
                        font.pixelSize: 15
                    }

                    TextField {
                        id: carModel
                        placeholderText: qsTr("Car Model")
                        text: settingsViewHandler.carModel
                        onTextChanged: settingsViewHandler.carModel = text
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
                        checked: settingsViewHandler.mediaAutoPlayback
                        onCheckedChanged: settingsViewHandler.mediaAutoPlayback = checked
                        text: qsTr("Auto play last song on playback")
                    }
                }

                Row {
                    spacing: 10
                    CheckBox {
                        id: autoStart
                        checked: settingsViewHandler.mediaAutoStart
                        onCheckedChanged: settingsViewHandler.mediaAutoStart = checked
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
                        id: channelMedia
                        checked: settingsViewHandler.aaChannelMedia
                        onCheckedChanged: settingsViewHandler.aaChannelMedia = checked
                        text: qsTr("Media")
                    }

                    CheckBox {
                        id: channelGuidance
                        checked: settingsViewHandler.aaChannelGuidance
                        onCheckedChanged: settingsViewHandler.aaChannelGuidance = checked
                        text: qsTr("Guidance")
                    }

                    CheckBox {
                        id: channelTelephony
                        checked: settingsViewHandler.aaChannelTelephony
                        onCheckedChanged: settingsViewHandler.aaChannelTelephony = checked
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
                        id: audioRt
                        text: qsTr("RT Audio")
                        checked: settingsViewHandler.audioTypeRt
                        onCheckedChanged: settingsViewHandler.audioTypeRt = checked
                    }

                    RadioButton {
                        id: audioQt
                        text: qsTr("Qt")
                        checked: settingsViewHandler.audioTypeQt
                        onCheckedChanged: settingsViewHandler.audioTypeQt = checked
                    }
                }

                Row {
                    Text {
                        text: "Frame Rate"
                        font.pixelSize: 15
                    }
                    spacing: 10

                    ComboBox {
                        id: frameRate
                        model: frameRateModel.comboBoxItems
                        currentIndex: frameRateModel.comboBoxItems.indexOf(frameRateModel.currentComboBoxItem)
                        onCurrentIndexChanged: {
                            frameRateModel.currentComboBoxItem = frameRateModel.comboBoxItems[currentIndex];
                        }
                    }

                }
                  Row {
                    spacing: 10
                    Text {
                        text: "Resolution"
                        font.pixelSize: 15
                    }
                    ComboBox {
                        id: resolution
                        model: resolutionModel.comboBoxItems
                        currentIndex: resolutionModel.comboBoxItems.indexOf(resolutionModel.currentComboBoxItem)
                        onCurrentIndexChanged: {
                            resolutionModel.currentComboBoxItem = resolutionModel.comboBoxItems[currentIndex];
                        }
                    }
                }

                Row {
                    spacing: 10
                    Text {
                        text: "OMX Layer"
                        font.pixelSize: 15
                    }
                    SpinBox {
                        id: omxLayer
                        value: settingsViewHandler.videoOMXLayer
                        onValueChanged: settingsViewHandler.videoOMXLayer = value
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
                        id: marginHeight
                        value: settingsViewHandler.videoMarginHeight
                        onValueChanged: settingsViewHandler.videoMarginHeight = value
                    }
                    Text {
                        text: "Length"
                        font.pixelSize: 15
                    }
                    SpinBox {
                        id: marginWidth
                        value: settingsViewHandler.videoMarginWidth
                        onValueChanged: settingsViewHandler.videoMarginWidth = value
                    }
                }
                Row {
                    spacing: 10
                    Text {
                        text: "DPI"
                        font.pixelSize: 15
                    }
                    Slider {
                        id: dpi
                        value: settingsViewHandler.screenDPI
                        onValueChanged: settingsViewHandler.screenDPI = value
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
                    RangeSlider {
                        id: volumePlaybackRange
                        from: 0
                        to: 255
                        first.value: settingsViewHandler.audioVolumePlaybackMin
                        second.value: settingsViewHandler.audioVolumePlaybackMax

                        onFirstChanged: settingsViewHandler.audioVolumePlaybackMin = first.value
                        onSecondChanged: settingsViewHandler.audioVolumePlaybackMax = second.value
                    }
                }
                Row {
                    spacing: 10
                    Text {
                        text: "Output Device"
                        font.pixelSize: 15
                    }

                    ComboBox {
                        id: pulseAudioDeviceOutput
                        model: pulseAudioDeviceModelOutput.comboBoxItems
                        currentIndex: pulseAudioDeviceModelOutput.comboBoxItems.indexOf(pulseAudioDeviceModelOutput.currentComboBoxItem)
                        onCurrentIndexChanged: {
                            pulseAudioDeviceModelOutput.currentComboBoxItem = pulseAudioDeviceModelOutput.comboBoxItems[currentIndex];
                        }
                    }
                }


                Row {
                    spacing: 10
                    Text {
                        text: "Capture Volume"
                        font.pixelSize: 15
                    }
                    Slider {
                        id: volumeCapture
                        from: 0
                        to: 255
                        value: settingsViewHandler.audioVolumeCapture
                        onValueChanged: settingsViewHandler.audioVolumeCapture = value
                    }
                }
                Row {
                    spacing: 10
                    Text {
                        text: "Capture Range"
                        font.pixelSize: 15
                    }
                    RangeSlider {
                        id: volumeCaptureRange
                        from: 0
                        to: 255
                        first.value: settingsViewHandler.audioVolumeCaptureMin
                        second.value: settingsViewHandler.audioVolumeCaptureMax

                        onFirstChanged: settingsViewHandler.audioVolumeCaptureMin = first.value
                        onSecondChanged: settingsViewHandler.audioVolumeCaptureMax = second.value
                    }
                }
                Row {
                    spacing: 10
                    Text {
                        text: "Input Device"
                        font.pixelSize: 15
                    }
                    ComboBox {
                        id: pulseAudioDeviceInput
                        model: pulseAudioDeviceModelInput.comboBoxItems
                        currentIndex: pulseAudioDeviceModelInput.comboBoxItems.indexOf(pulseAudioDeviceModelInput.currentComboBoxItem)
                        onCurrentIndexChanged: {
                            pulseAudioDeviceModelInput.currentComboBoxItem = pulseAudioDeviceModelInput.comboBoxItems[currentIndex];
                        }
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
                        text: "Day Min/Max"
                        font.pixelSize: 15
                    }
                    RangeSlider {
                        id: brightnessDayRange
                        from: 0
                        to: 255
                        first.value: settingsViewHandler.screenBrightnessDayMin
                        second.value: settingsViewHandler.screenBrightnessDayMax
                        
                        onFirstChanged: settingsViewHandler.screenBrightnessDayMin = first.value
                        onSecondChanged: settingsViewHandler.screenBrightnessDayMax = second.value
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
                        id: brightnessNightRange
                        from: 0
                        to: 255
                        first.value: settingsViewHandler.screenBrightnessNightMin
                        second.value: settingsViewHandler.screenBrightnessNightMax
                        
                        onFirstChanged: settingsViewHandler.screenBrightnessNightMin = first.value
                        onSecondChanged: settingsViewHandler.screenBrightnessNightMax = second.value
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
                        id: videoEgl

                        text: qsTr("EGL")
                        checked: settingsViewHandler.videoTypeEgl
                        onCheckedChanged: settingsViewHandler.videoTypeEgl = checked
                    }

                    RadioButton {
                        id: videoX11
                        text: qsTr("X11")
                        checked: settingsViewHandler.videoTypeX11
                        onCheckedChanged: settingsViewHandler.videoTypeX11 = checked
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
                        id: rotateDisplay
                        text: qsTr("Rotate Display")
                        checked: settingsViewHandler.videoRotateDisplay
                        onCheckedChanged: settingsViewHandler.videoRotateDisplay = checked
                    }

                }

            }

        }

    }

}
