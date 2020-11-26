import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 1.4
import QtQuick.Dialogs 1.2
import QtQuick 2.9
import QtQuick.Controls.Styles 1.4
import QtQuick.Window 2.3
import org.redinav.qml 1.0
import Qt.labs.settings 1.0
import "../common"
import "../common/helper.js" as JS


Dialog {

    width: 800
    height: 600

    property var palette: activePalette

    GridLayout {

        columns: 8

        Rectangle {
            width: 100
            height: 100
            ColumnLayout {
                anchors.fill: parent
                Rectangle {
                    Layout.minimumHeight: 40
                    Layout.fillWidth: true
                    Text {
                        text: "alternateBase <br>" + palette.alternateBase;
                        color: "black";
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                }
                Rectangle {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    color: palette.alternateBase
                }
            }
        }

        Rectangle {
            width: 100
            height: 100
            ColumnLayout {
                anchors.fill: parent
                Rectangle {
                    Layout.minimumHeight: 40
                    Layout.fillWidth: true
                    Text {
                        text: "base <br>" + palette.base;
                        color: "black";
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                }
                Rectangle {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    color: palette.base
                }
            }
        }


        Rectangle {
            width: 100
            height: 100
            ColumnLayout {
                anchors.fill: parent
                Rectangle {
                    Layout.minimumHeight: 40
                    Layout.fillWidth: true
                    Text {
                        text: "button <br>" + palette.button;
                        color: "black";
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                }
                Rectangle {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    color: palette.button
                }
            }
        }

        Rectangle {
            width: 100
            height: 100
            ColumnLayout {
                anchors.fill: parent
                Rectangle {
                    Layout.minimumHeight: 40
                    Layout.fillWidth: true
                    Text {
                        text: "buttonText <br>" + palette.buttonText;
                        color: "black";
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                }
                Rectangle {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    color: palette.buttonText
                }
            }
        }


        Rectangle {
            width: 100
            height: 100
            ColumnLayout {
                anchors.fill: parent
                Rectangle {
                    Layout.minimumHeight: 40
                    Layout.fillWidth: true
                    Text {
                        text: "dark <br>" + palette.dark;
                        color: "black";
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                }
                Rectangle {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    color: palette.dark
                }
            }
        }


        Rectangle {
            width: 100
            height: 100
            ColumnLayout {
                anchors.fill: parent
                Rectangle {
                    Layout.minimumHeight: 40
                    Layout.fillWidth: true
                    Text {
                        text: "highlight <br>" + palette.highlight;
                        color: "black";
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                }
                Rectangle {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    color: palette.highlight
                }
            }
        }


        Rectangle {
            width: 100
            height: 100
            ColumnLayout {
                anchors.fill: parent
                Rectangle {
                    Layout.minimumHeight: 40
                    Layout.fillWidth: true
                    Text {
                        text: "highlightedText <br>" + palette.highlightedText;
                        color: "black";
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                }
                Rectangle {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    color: palette.highlightedText
                }
            }
        }


        Rectangle {
            width: 100
            height: 100
            ColumnLayout {
                anchors.fill: parent
                Rectangle {
                    Layout.minimumHeight: 40
                    Layout.fillWidth: true
                    Text {
                        text: "light <br>" + palette.light;
                        color: "black";
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                }
                Rectangle {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    color: palette.light
                }
            }
        }



        Rectangle {
            width: 100
            height: 100
            ColumnLayout {
                anchors.fill: parent
                Rectangle {
                    Layout.minimumHeight: 40
                    Layout.fillWidth: true
                    Text {
                        text: "mid <br>" + palette.mid;
                        color: "black";
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                }
                Rectangle {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    color: palette.mid
                }
            }
        }

        Rectangle {
            width: 100
            height: 100
            ColumnLayout {
                anchors.fill: parent
                Rectangle {
                    Layout.minimumHeight: 40
                    Layout.fillWidth: true
                    Text {
                        text: "midlight <br>" + palette.midlight;
                        color: "black";
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                }
                Rectangle {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    color: palette.midlight
                }
            }
        }

        Rectangle {
            width: 100
            height: 100
            ColumnLayout {
                anchors.fill: parent
                Rectangle {
                    Layout.minimumHeight: 40
                    Layout.fillWidth: true
                    Text {
                        text: "shadow <br>" + palette.shadow;
                        color: "black";
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                }
                Rectangle {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    color: palette.shadow
                }
            }
        }


        Rectangle {
            width: 100
            height: 100
            ColumnLayout {
                anchors.fill: parent
                Rectangle {
                    Layout.minimumHeight: 40
                    Layout.fillWidth: true
                    Text {
                        text: "text <br>" + palette.text;
                        color: "black";
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                }
                Rectangle {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    color: palette.text
                }
            }
        }


        Rectangle {
            width: 100
            height: 100
            ColumnLayout {
                anchors.fill: parent
                Rectangle {
                    Layout.minimumHeight: 40
                    Layout.fillWidth: true
                    Text {
                        text: "window <br>" + palette.window;
                        color: "black";
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                }
                Rectangle {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    color: palette.window
                }
            }
        }


        Rectangle {
            width: 100
            height: 100
            ColumnLayout {
                anchors.fill: parent
                Rectangle {
                    Layout.minimumHeight: 40
                    Layout.fillWidth: true
                    Text {
                        text: "windowText <br>" + palette.windowText;
                        color: "black";
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                }
                Rectangle {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    color: palette.windowText
                }
            }
        }




    }


}
