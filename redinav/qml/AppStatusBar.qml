import QtQuick 2.0
import QtQuick.Window 2.3
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.3

StatusBar {
    id: root

    style: StatusBarStyle {
        padding {
            left: 8
            right: 8
            top: 3
            bottom: 3
        }
        background: Rectangle {
            implicitHeight: 16
            implicitWidth: 200
            color: activePalette.alternateBase
            Rectangle {
                anchors.top: parent.top
                width: parent.width
                height: 1
            }
        }
    }

    Menu {
        id: dumpCancelMenu
        MenuItem {
            text: "Cancel"
            onTriggered: {
                backend.cancelKeysDump();
            }
        }
    }

    Menu {
        id: importCancelMenu
        MenuItem {
            text: "Cancel"
            onTriggered: {
                backend.cancelKeysImport()
            }
        }
    }

    RowLayout {

        Layout.fillWidth: true
        spacing: 5

        Text {
            id: dumpProgress
            color: activePalette.text
            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.RightButton
                onClicked: {
                    dumpCancelMenu.popup()
                }
            }
        }

        Text {
            id: importProgress
            color: activePalette.text
            text: ""
            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.RightButton
                onClicked: {
                    importCancelMenu.popup()
                }
            }
        }

    }

    Connections {
        target: backend

        onKeysDumpProgress: {
            dumpProgress.text = "Dump: " + current + "/" + total;
        }

        onKeysImportProgress: {
            importProgress.text = "Import: " + current + "/" + total;
        }

        onKeysDumpFinished: {
            dumpProgress.text = "";
        }

        onKeysImportFinished: {
            importProgress.text = "";
            backend.reloadKeys(dbIndex)
        }

    }

    Connections {
        target: mainTreeView

    }

}
