import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import "common"
import "common/helper.js" as JS
import "license"

Rectangle {
    color: "#00000000"
    anchors.fill: parent

    ColumnLayout {
        id: columnLayout
        anchors.fill: parent

        Rectangle {
            id: appNameRectangle
            height: 80
            color: "#00000000"
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            Layout.minimumHeight: 90
            Layout.fillWidth: true

            Column {
                id: column
                anchors.horizontalCenter: parent.horizontalCenter

                Row {
                    anchors.horizontalCenter: parent.horizontalCenter
                    Image {
                        id: logo;
                        width: 150
                        height: 150
                        anchors.verticalCenter: parent.verticalCenter
                        fillMode: Image.PreserveAspectFit
                        source: "qrc:/resources/images/redinav.svg"
                    }

                }

                Row {
                    anchors.horizontalCenter: parent.horizontalCenter
                    Text {
                        color: activePalette.text
                        horizontalAlignment: Text.AlignHCenter
                        id: appNameText
                        text: "RediNav"
                        font.pixelSize: 40
                    }
                }
                Row {
                    anchors.horizontalCenter: parent.horizontalCenter
                    Text {
                        color: activePalette.text
                        horizontalAlignment: Text.AlignHCenter
                        id: appSlogan
                        text: "The best friend of all Redis servers!"
                        font.bold: true
                    }
                }
                Row {
                    anchors.horizontalCenter: parent.horizontalCenter
                    Text {
                        color: activePalette.text
                        horizontalAlignment: Text.AlignHCenter
                        text: "version " + "<u>" + backend.getVersion() + "</u>"
                    }
                }
                Row {
                    anchors.horizontalCenter: parent.horizontalCenter
                    RichTextWithLinks {
                        color: activePalette.text
                        horizontalAlignment: Text.AlignHCenter
                        font.pixelSize: 12
                        html:  qsTr('<br><br><br><a href=\"%1/wp-content/uploads/redinav-src.tar.gz\">Source code</a> covered by <a href=\"https://www.gnu.org/licenses/gpl.txt\">GPLv3</a>').arg(licenseManager.storeUrl());
                    }
                }

                Row {
                    anchors.horizontalCenter: parent.horizontalCenter
                    RichTextWithLinks {
                        color: activePalette.text
                        horizontalAlignment: Text.AlignHCenter
                        font.pixelSize: 12
                        html:  '<br>Using <a href="http://download.qt.io/official_releases/qt/5.11/">Qt 5.11 Framework</a> under <a href="https://www.gnu.org/licenses/lgpl-3.0.txt">LGPLv3</a>'
                    }
                }

                Row {
                    anchors.horizontalCenter: parent.horizontalCenter
                    RichTextWithLinks {
                        color: activePalette.text
                        horizontalAlignment: Text.AlignHCenter
                        font.pixelSize: 12
                        html: {
                            var s = qsTr(''+
                                    '<br>'+
                                    '<br>'+
                                    '<br>'+
                                    'Get help and support at <a href="%1">%2</a>'+
                                    '<br>'+
                                    'or email us directly at <a href="mailto:%3">%3</a>'+
                                    '<br>'+
                                    '').arg(licenseManager.storeUrl()).arg(licenseManager.storeUrl()).arg(licenseManager.supportEmail());
                            if (JS.isOSX()) {
                                s = s +
                                        '<br>'+
                                        'If you like RediNav, we\'d appreciate a short review at <a href="https://itunes.apple.com/us/app/redinav/id1413724884">App Store</a>'+
                                        '<br>Help us make it even better! Thank you!'+
                                        '';
                            }

                            return s;

                        }
                    }
                }

                Row {
                    visible: licenseManager.valid
                    anchors.horizontalCenter: parent.horizontalCenter
                    height: 160


                    Loader {
                        id: licenseInfoLoader
                        active: false
                        source: ""
                        onLoaded: {
                            licenseInfoLoader.item.open()
                        }
                    }

                    Button {
                        id: licenseInfoButton
                        text: " License && Updates  "
                        onClicked: {
                            licenseInfoLoader.setSource("qrc:/qml/license/LicenseInfo.qml")
                            licenseInfoLoader.active = true;
                        }
                    }
                }
            }
        }



        Rectangle {
            id: appInfoRectangle
            color: "#00000000"
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            Layout.minimumHeight: 200
            Layout.fillWidth: true

        }




    }
}

/*##^## Designer {
    D{i:0;autoSize:true;height:480;width:640}D{i:5;anchors_width:438;anchors_x:8;anchors_y:147}
D{i:4;anchors_height:100;anchors_width:200;anchors_x:384;anchors_y:300}D{i:1;anchors_height:0;anchors_x:0;anchors_y:0}
}
 ##^##*/
