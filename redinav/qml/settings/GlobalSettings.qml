import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 1.4
import QtQuick.Dialogs 1.2
import Qt.labs.settings 1.0
import "."

Dialog {
    property int defaultHeight: 35

    id: root
    title: qsTr("Settings")
    standardButtons: StandardButton.Close
    width: 700
    height: 500

    TabView {
        id: tabView
        anchors.fill: parent

        Tab {

            id: tab1
            title: "General"
            anchors.margins: 20

            ColumnLayout {

                ComboboxOption {
                    id: appTheme
                    Layout.fillWidth: true
                    Layout.preferredHeight: root.defaultHeight
                    value: "default"
                    model: ["default", "light", "dark"]
                    label: qsTr("Theme (restart required)")
                }

                ComboboxOption {
                    id: appFont

                    Layout.fillWidth: true
                    Layout.preferredHeight: root.defaultHeight

                    value: Qt.platform.os == "osx"? "Helvetica Neue" : "Open Sans"
                    model: Qt.fontFamilies()
                    label: qsTr("Font (restart required)")
                }

                ComboboxOption {
                    id: appFontSize

                    Layout.fillWidth: true
                    Layout.preferredHeight: root.defaultHeight

                    model: ["8", "9", "10", "11", "12", "13", "14", "15", "16", "17", "18"]
                    value: Qt.platform.os == "osx"? "12" : "10"
                    label: qsTr("Font Size (restart required)")
                }

                IntOption {
                    id: defaultConnectionTimeout
                    Layout.fillWidth: true
                    Layout.preferredHeight: root.defaultHeight
                    value: 5
                    label: qsTr("Default connection timeout (restart required)")
                }

                BoolOption {
                    id: disableFlushDb
                    Layout.fillWidth: true
                    Layout.preferredHeight: root.defaultHeight
                    value: true
                    label: qsTr("Disable 'Flush database' functionality")
                }

                BoolOption {
                    id: disableNamespaceDelete
                    Layout.fillWidth: true
                    Layout.preferredHeight: root.defaultHeight
                    value: true
                    label: qsTr("Disable 'Delete Namespace' functionality")
                }

                BoolOption {
                    id: doubleClickTreeItemActivation
                    Layout.fillWidth: true
                    Layout.preferredHeight: root.defaultHeight
                    value: false
                    label: qsTr("Activate tree items with double-click")
                }

                ResetSettings {
                    Layout.fillWidth: true
                    Layout.preferredHeight: root.defaultHeight
                    label: qsTr("Reset settings (restart required)")
                    gSettings: globalSettings1
                }

                Item {Layout.fillWidth: true; Layout.fillHeight: true;}

                RowLayout {
                    Layout.alignment: Qt.AlignBottom
                    Image {
                        Layout.preferredWidth: 30
                        Layout.preferredHeight: 30
                        source: qsTr("qrc:/resources/images/icons/themes/%1/alert.svg").arg(approot.theme)
                    }
                    Text {
                        color: activePalette.text
                        text: qsTr("Some settings may require application restart!")
                    }
                }

                Settings {
                    id: globalSettings1
                    category: "app"
                    property alias appTheme: appTheme.value
                    property alias appFont: appFont.value
                    property alias appFontSize: appFontSize.value
                    property alias defaultConnectionTimeout: defaultConnectionTimeout.value
                    property alias disableFlushDb: disableFlushDb.value
                    property alias disableNamespaceDelete: disableNamespaceDelete.value
                    property alias doubleClickTreeItemActivation: doubleClickTreeItemActivation.value
                }


            }

        }

        Tab {
            id: tab2
            title: "Logging"
            anchors.margins: 20
            ColumnLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignTop

                BoolOption {
                    id: infoLog
                    Layout.fillWidth: true
                    Layout.preferredHeight: root.defaultHeight
                    value: true
                    label: qsTr("Enable informational log (restart required)")
                }

                BoolOption {
                    id: verboseLog
                    Layout.fillWidth: true
                    Layout.preferredHeight: root.defaultHeight
                    value: false
                    label: qsTr("Enable verbose logging (restart required)")
                }

                BoolOption {
                    id: verboseConnectionLog
                    Layout.fillWidth: true
                    Layout.preferredHeight: root.defaultHeight
                    value: false
                    label: qsTr("Enable detailed connection log (restart required)")
                }


                BoolOption {
                    id: logToFile
                    Layout.fillWidth: true
                    Layout.preferredHeight: root.defaultHeight
                    value: true
                    label: qsTr("Log to file (restart required)")
                }


                Item {Layout.fillWidth: true; Layout.fillHeight: true;}

                RowLayout {
                    Layout.alignment: Qt.AlignBottom
                    Image {
                        Layout.preferredWidth: 30
                        Layout.preferredHeight: 30
                        source: qsTr("qrc:/resources/images/icons/themes/%1/alert.svg").arg(approot.theme)
                    }
                    Text {
                        color: activePalette.text
                        text: qsTr("Some settings may require application restart!")
                    }
                }

                Settings {
                    id: globalSettings2
                    category: "app"
                    property alias verboseConnectionLog: verboseConnectionLog.value
                    property alias logToFile: logToFile.value
                    property alias infoLog: infoLog.value
                    property alias verboseLog: verboseLog.value
                }

            }
        }


    }


}
