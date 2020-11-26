import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Dialogs 1.2
import QtQml.Models 2.3
import "common"
import "settings"
import "common/helper.js" as JS
import "license"
import org.redinav.qml 1.0



ToolBar {

    id: appToolbar

    LicenseKeyDialog {
        id: licenseKeyDialog
    }


    Connections {
        target:  licenseManager

        onLicenseCheckFinished: {
            licenseRestricted.visible = !success;
            if (message && !success) {
                notifyDialog.showError(message, "License problem")
            }
        }

        onUpdatesChecked: {
            if (isAvail) {
                notifyDialog.showMsg(qsTr("New version is available - %1. <a href=\"%2\">Download</a> and reinstall manually").arg(newVersion).arg(downloadUrl), "Updates")
            }
        }

        onRequestLicenseKeyActivation: {
            JS.requestLicenseActivation();
        }

    }

    RowLayout {
        anchors.fill: parent
        Button {
            iconSource: qsTr("qrc:/resources/images/icons/themes/%1/ol_add.svg").arg(approot.theme)
            text: " " + "Add connection"
            onClicked: {
                JS.loader("qrc:/qml/settings/ConnectionSettignsDialog.qml", {
                              "settings" : backend.createEmptyConfig()
                          },
                          function(loader) {
                              loader.item.open()
                          })
            }
        }


        Button {
            enabled: !limiter.isActive || limiter.canAccess(Limiter.ImportConnections)
            iconSource: qsTr("qrc:/resources/images/icons/themes/%1/ol_import.svg").arg(approot.theme)
            text: " " + "Import Connections"
            onClicked: {
                fileSelector.title = qsTr("Import Connections from JSON")
                fileSelector.nameFilters = ["Connections (*.json)", "All files (*.*)"]
                fileSelector.selectExisting = true
                fileSelector.callback = function(fileUrl) {
                    backend.importConnections(backend.getPathFromUrl(fileUrl))
                    mainDataArea.removeAllTabs()
                }
                fileSelector.open()
            }
        }

        Button {
            enabled: !limiter.isActive || limiter.canAccess(Limiter.ExportConnections)
            iconSource: qsTr("qrc:/resources/images/icons/themes/%1/ol_export.svg").arg(approot.theme)
            text: " " + "Export Connections "

            onClicked: {
                fileSelector.title = qsTr("Export Connections")
                fileSelector.nameFilters = ["Connections (*.json)", "All files (*.*)"]
                fileSelector.selectExisting = false
                fileSelector.callback = function(fileUrl) {
                    backend.saveConnectionsConfigToFile(backend.getPathFromUrl(fileUrl))
                }
                fileSelector.open()
            }

        }

        /*** Used internally to show palette colors
        InternalColors {
            id: colorsDemo
        }

        Button {
            id: b1
            text: "Active Palette"
            onClicked: {
                colorsDemo.palette = activePalette
                colorsDemo.open()
            }
        }

        Button {
            id: b2
            text: "Disabled Palette"
            onClicked: {
                colorsDemo.palette = disabledPalette
                colorsDemo.open()
            }
        }

        Button {
            id: b3
            text: "Inactive Palette"
            onClicked: {
                colorsDemo.palette = inactivePalette
                colorsDemo.open()
            }
        }
        **/

        Item { Layout.fillWidth: {return !licenseRestricted.visible; } }

        Rectangle {
            id: licenseRestricted
            visible: licenseManager.requireActivation()
            color: "#00000000"
            Layout.fillHeight: true
            Layout.fillWidth: true

            Row {
                id: row
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.top: parent.top
                spacing: 5

                Image {
                    width: 30
                    height: 30
                    anchors.verticalCenter: parent.verticalCenter
                    Layout.maximumHeight: 30
                    Layout.maximumWidth: 30
                    source: "qrc:/resources/images/icons/alert.svg"
                }


                Text {
                    color: activePalette.text
                    text: "Restricted mode"
                    anchors.verticalCenter: parent.verticalCenter
                }



                Button {
                    text: qsTr("Activate License")
                    anchors.verticalCenter: parent.verticalCenter
                    onClicked: {
                        JS.requestLicenseActivation();
                    }
                }
            }
        }


        ToolButton {
            iconSource: qsTr("qrc:/resources/images/icons/themes/%1/ol_settings.svg").arg(approot.theme)
            text: qsTr("Settings")
            tooltip: "Global Application Settings"
            onClicked: {
                settingsDialog.open()
            }
        }


    }


}



/*##^## Designer {
    D{i:13;anchors_height:400;anchors_width:200}
}
 ##^##*/
