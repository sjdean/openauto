import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import JourneyOS

// Full-screen CAN bus telemetry dashboard.
// Pushed onto the Journey stackView when the OBD button is tapped.
// Emits requestHome() to pop back — handled by Journey.qml's Connections.
Item {
    id: dashboard
    width:  parent ? parent.width  : 800
    height: parent ? parent.height : 480

    signal requestHome()

    // Convenience: safe access to canBusReceiver (may be null in non-CAN builds)
    readonly property bool hasData: canBusReceiver !== null && canBusReceiver !== undefined
                                    && canBusReceiver.configured

    Rectangle {
        anchors.fill: parent
        color: Constants.primaryBackgroundColor
    }

    // ── Top bar: back button, title, tab selector ─────────────────────────────
    Item {
        id: topBar
        anchors.top:   parent.top
        anchors.left:  parent.left
        anchors.right: parent.right
        height: 44

        Rectangle {
            anchors.fill: parent
            color: Qt.rgba(0, 0, 0, 0.3)
        }

        JourneyButton {
            id: backButton
            anchors.left:           parent.left
            anchors.leftMargin:     8
            anchors.verticalCenter: parent.verticalCenter
            height: 30
            text:   "Back"
            icon.source: "images/close.svg"
            iconSize: height * 0.45
            onClicked: dashboard.requestHome()
        }

        Text {
            anchors.centerIn: parent
            text:           "CAN Bus"
            color:          "white"
            font.pixelSize: 16
            font.bold:      true
        }

        // Tab selector
        Row {
            anchors.right:          parent.right
            anchors.rightMargin:    8
            anchors.verticalCenter: parent.verticalCenter
            spacing: 4

            Repeater {
                model: ["Live Data", "Learning"]
                JourneyButton {
                    height: 30
                    text:   modelData
                    // Highlight active tab with an underline
                    Rectangle {
                        anchors.bottom:           parent.bottom
                        anchors.horizontalCenter: parent.horizontalCenter
                        width:  parent.width * 0.7
                        height: 2
                        color:  Constants.okColor
                        visible: tabView.currentIndex === index
                    }
                    onClicked: tabView.currentIndex = index
                }
            }
        }
    }

    // ── Tab content ───────────────────────────────────────────────────────────
    StackLayout {
        id: tabView
        anchors.top:    topBar.bottom
        anchors.left:   parent.left
        anchors.right:  parent.right
        anchors.bottom: parent.bottom
        currentIndex:   0

        // ── Tab 0: Live Data ──────────────────────────────────────────────────
        Item {
            id: liveTab

            // Top row: Speed | Steering + Info | RPM
            Item {
                id: topRow
                anchors.top:    parent.top
                anchors.left:   parent.left
                anchors.right:  parent.right
                anchors.topMargin: 8
                height: parent.height * 0.52

                // Speed gauge
                CanBusGauge {
                    id: speedGauge
                    anchors.left:           parent.left
                    anchors.leftMargin:     16
                    anchors.verticalCenter: parent.verticalCenter
                    width:  parent.height * 0.9
                    height: parent.height * 0.9
                    value:    hasData ? canBusReceiver.speedMph    : 0
                    maxValue: 150
                    label:    "Speed"
                    unit:     "mph"
                }

                // Centre column: steering bar + key stats
                Column {
                    anchors.left:   speedGauge.right
                    anchors.right:  rpmGauge.left
                    anchors.top:    parent.top
                    anchors.bottom: parent.bottom
                    anchors.leftMargin:  8
                    anchors.rightMargin: 8
                    spacing: 6

                    CanBusSteeringBar {
                        width:  parent.width
                        height: 52
                        rawValue: hasData ? canBusReceiver.steeringPosition : 0
                    }

                    // Info grid: outside temp, radiator temp, engine status
                    Grid {
                        width:   parent.width
                        columns: 2
                        rowSpacing:    4
                        columnSpacing: 8

                        // Outside temperature
                        Text { text: "Outside"; color: "#888888"; font.pixelSize: 11 }
                        Text {
                            text: hasData ? canBusReceiver.outsideTemp + " °C" : "—"
                            color: {
                                if (!hasData) return "#888888"
                                var t = canBusReceiver.outsideTemp
                                return t <= 0 ? "#42A5F5" : t < 10 ? Constants.waitColor
                                     : t < 30 ? Constants.okColor  : Constants.badColor
                            }
                            font.pixelSize: 13
                            font.bold: true
                        }

                        // Radiator temperature
                        Text { text: "Coolant";  color: "#888888"; font.pixelSize: 11 }
                        Text {
                            text: hasData ? canBusReceiver.radiatorTemp + " °C" : "—"
                            color: {
                                if (!hasData) return "#888888"
                                var t = canBusReceiver.radiatorTemp
                                return t < 70  ? "#42A5F5"         // cold
                                     : t < 100 ? Constants.okColor  // normal
                                     : Constants.badColor            // overheating
                            }
                            font.pixelSize: 13
                            font.bold: true
                        }

                        // Engine status
                        Text { text: "Engine"; color: "#888888"; font.pixelSize: 11 }
                        Text {
                            text: {
                                if (!hasData) return "—"
                                switch (canBusReceiver.engineStatus) {
                                    case 0: return "Off"
                                    case 1: return "Starting"
                                    case 2: return "Running"
                                    case 4: return "Stopping"
                                    default: return "Unknown"
                                }
                            }
                            color: {
                                if (!hasData) return "#888888"
                                return canBusReceiver.engineStatus === 2 ? Constants.okColor
                                     : canBusReceiver.engineStatus === 0 ? "#888888"
                                     : Constants.waitColor
                            }
                            font.pixelSize: 13
                            font.bold: true
                        }

                        // Lights
                        Text { text: "Lights"; color: "#888888"; font.pixelSize: 11 }
                        Text {
                            text: {
                                if (!hasData) return "—"
                                var l = canBusReceiver.lights
                                if (l === 0)   return "Off"
                                if (l === 128) return "Side"
                                if (l >= 192)  return "Head" + (canBusReceiver.lightsHigh ? " + High" : "")
                                return "On"
                            }
                            color: hasData && canBusReceiver.lights > 0
                                   ? Constants.okColor : "#888888"
                            font.pixelSize: 13
                            font.bold: true
                        }

                        // Throttle
                        Text { text: "Throttle"; color: "#888888"; font.pixelSize: 11 }
                        Text {
                            text: hasData ? canBusReceiver.throttlePosition : "—"
                            color: "white"
                            font.pixelSize: 13
                            font.bold: true
                        }
                    }
                }

                // RPM gauge
                CanBusGauge {
                    id: rpmGauge
                    anchors.right:          parent.right
                    anchors.rightMargin:    16
                    anchors.verticalCenter: parent.verticalCenter
                    width:  parent.height * 0.9
                    height: parent.height * 0.9
                    value:    hasData ? canBusReceiver.rpm       : 0
                    maxValue: 7000
                    midPoint: 0.8
                    label:    "RPM"
                    unit:     "rpm"
                }
            }

            // Bottom row: Fuel bar | Odometer + Range | Battery bar
            Item {
                id: bottomRow
                anchors.top:    topRow.bottom
                anchors.left:   parent.left
                anchors.right:  parent.right
                anchors.bottom: parent.bottom
                anchors.topMargin:    4
                anchors.bottomMargin: 8

                // Fuel bar (range in miles → treat 400 miles as full)
                CanBusBar {
                    id: fuelBar
                    anchors.left:           parent.left
                    anchors.leftMargin:     16
                    anchors.top:            parent.top
                    anchors.bottom:         parent.bottom
                    width: speedGauge.width
                    value:    hasData ? canBusReceiver.rangeMiles  : 0
                    maxValue: 400
                    label:    "Range"
                    valueText: hasData ? canBusReceiver.rangeMiles + " mi" : "—"
                }

                // Centre: odometer + range in litres + service miles
                Column {
                    anchors.left:   fuelBar.right
                    anchors.right:  batteryBar.left
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.leftMargin:  8
                    anchors.rightMargin: 8
                    spacing: 5

                    Grid {
                        width: parent.width
                        columns: 2
                        rowSpacing:    4
                        columnSpacing: 8

                        Text { text: "Odometer";   color: "#888888"; font.pixelSize: 11 }
                        Text {
                            text: hasData ? canBusReceiver.odometerMiles + " mi" : "—"
                            color: "white"; font.pixelSize: 13; font.bold: true
                        }

                        Text { text: "Range (L)";  color: "#888888"; font.pixelSize: 11 }
                        Text {
                            text: hasData ? canBusReceiver.rangeLitres + " L" : "—"
                            color: "white"; font.pixelSize: 13; font.bold: true
                        }

                        Text { text: "Service";    color: "#888888"; font.pixelSize: 11 }
                        Text {
                            text: hasData ? canBusReceiver.serviceMiles + " mi" : "—"
                            color: hasData && canBusReceiver.serviceMiles < 200
                                   ? Constants.badColor : "white"
                            font.pixelSize: 13; font.bold: true
                        }

                        Text { text: "Battery";    color: "#888888"; font.pixelSize: 11 }
                        Text {
                            text: hasData && canBusReceiver.batteryVoltage > 0
                                  ? (canBusReceiver.batteryVoltage / 10).toFixed(1) + " V" : "—"
                            color: {
                                if (!hasData || canBusReceiver.batteryVoltage <= 0)
                                    return "#888888"
                                var v = canBusReceiver.batteryVoltage / 10
                                return v < 11.8 ? Constants.badColor
                                     : v < 12.4 ? Constants.waitColor
                                     : Constants.okColor
                            }
                            font.pixelSize: 13; font.bold: true
                        }
                    }
                }

                // Battery voltage bar (100–150 = 10.0–15.0 V, nominal 124 = 12.4V)
                CanBusBar {
                    id: batteryBar
                    anchors.right:          parent.right
                    anchors.rightMargin:    16
                    anchors.top:            parent.top
                    anchors.bottom:         parent.bottom
                    width: rpmGauge.width
                    value:    hasData ? canBusReceiver.batteryVoltage  : 0
                    minValue: 100   // 10.0 V
                    maxValue: 150   // 15.0 V
                    label:    "Volts"
                    valueText: hasData && canBusReceiver.batteryVoltage > 0
                               ? (canBusReceiver.batteryVoltage / 10).toFixed(1) : "—"
                    // Invert colours: low voltage = red, high = green
                    colorEmpty: Constants.badColor
                    colorFull:  Constants.okColor
                }
            }
        }

        // ── Tab 1: Learning (placeholder) ─────────────────────────────────────
        Item {
            id: learningTab

            Column {
                anchors.centerIn: parent
                spacing: 12

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text:           "Learning Mode"
                    color:          "white"
                    font.pixelSize: 20
                    font.bold:      true
                }
                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text:           "Live CAN frame sniffer — coming in a future update."
                    color:          "#888888"
                    font.pixelSize: 14
                }
                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text:           "Will stream raw frames from the RPi bridge\nfor mapping new signals."
                    color:          "#666666"
                    font.pixelSize: 12
                    horizontalAlignment: Text.AlignHCenter
                }
            }
        }
    }
}