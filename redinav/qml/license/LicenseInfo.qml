import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 1.4
import QtQuick.Dialogs 1.2
import QtQuick.Controls.Styles 1.4


import "../common"

Dialog {
    title: "Your License Key"
    id: root

    contentItem: Rectangle {
        implicitHeight: 150
        implicitWidth: 500
        color: activePalette.window

        ColumnLayout {
            id: column
            anchors.fill: parent

            RowLayout {
                height: 40
                Layout.margins: 10
                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                Layout.fillWidth: true
                TextArea {
                    Layout.fillWidth:true
                    implicitHeight: 40
                    id: licenseKey
                    readOnly: true
                    text: licenseManager.licenseKey()
                    horizontalAlignment: Text.AlignHCenter
                    font.pixelSize: 20
                    selectByMouse: true
                }
            }

            RowLayout {
                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                Layout.fillWidth: true
                Text {
                    color: activePalette.text
                    text: {
                        var s = "Expire: "
                        if (licenseManager.expire() > 0) {
                            var daysLeft = licenseManager.daysLeft()
                            return s + licenseManager.keyExpireDate() + " (" + daysLeft + " days left)"
                        }
                        return s + "NEVER"
                    }
                    horizontalAlignment: Text.AlignHCenter
                }
            }


            RowLayout {
                spacing: 5
                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.margins: 10
                RichTextWithLinks {
                    color: activePalette.text
                    html: {
                        return "<a href=\"" + licenseManager.keyUrl() + "\">View/Manage license</a>" + " | " +
                               qsTr("<a href=\"%1/my-account/downloads/\">Downloads</a>").arg(licenseManager.storeUrl())
                    }
                }

                Item { Layout.fillWidth: true; }

                Button {
                    Connections {
                        id: tmpConn
                        target:  licenseManager
                        onUpdatesChecked: {
                            if (!isAvail) {
                                notifyDialog.showMsg("No new updates available!", "Updates");
                            }
                            tmpConn.enabled = false;
                        }
                    }
                    text: "Check for updates"
                    onClicked: {
                        licenseManager.validate(true,true)
                        root.close()
                    }
                }

                Button {
                    text: "Close"
                    onClicked: {
                        root.close()
                    }
                }

            }


        }


    }

}

/*##^## Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
 ##^##*/
