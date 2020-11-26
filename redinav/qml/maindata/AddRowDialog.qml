import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Dialogs 1.2
import QtQuick.Window 2.3
import "../common"
import "../common/helper.js" as JS

Dialog {

    property var key
    property string keyType

    function setProps(key, keyType) {
        dialog.key = key
        dialog.keyType = keyType
    }

    id: dialog
    title: "New " + JS.keyTypeMemberTitle(dialog.keyType, true)
    width: 550
    height: 500
    visible: false

    standardButtons: StandardButton.NoButton

    Item {
        anchors.fill: parent

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 5

            // HASH KEY
            RowLayout {
                visible:  dialog.keyType === "hash"
                Rectangle {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    color: "transparent"
                    TextEditor {
                        label: "Field name"
                        id: hashKey
                        text: ""
                        allowSaveToFile: false
                    }
                }
            }

            // ZSET SCORE
            RowLayout {
                visible: dialog.keyType === "zset"
                Text {
                    color: activePalette.text
                    text: "Score"
                }

                TextField {
                    Layout.fillWidth: true
                    id: zsetScore
                    text: ""
                    validator: DoubleValidator {
                        locale: "C"
                        decimals: 7
                        notation: DoubleValidator.StandardNotation
                    }
                }


            }

            // VALUE
            RowLayout {
                Rectangle {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    color: "transparent"
                    TextEditor {
                        id: keyValue
                        label: qsTr("%1 value").arg(JS.keyTypeMemberTitle(dialog.keyType, true))
                        text: ""
                        allowSaveToFile: false
                    }
                }
            }

            RowLayout {
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignLeft | Qt.AlignBottom
                Layout.fillWidth: true
                Layout.minimumHeight: 40

                Item {
                    Layout.fillWidth: true
                }

                Button {
                    text: qsTr("Save %1 and key to Redis").arg(JS.keyTypeMemberTitle(dialog.keyType))
                    onClicked: {
                        var v1
                        var v2
                        var keyType = dialog.keyType

                        if (keyType === "hash") {
                            v1 = hashKey.getText()
                            v2 = keyValue.getText()
                        }
                        else if (keyType === "zset") {
                            v1 = keyValue.getText()
                            v2 = zsetScore.text
                        }
                        else {
                            v1 = ""
                            v2 = keyValue.getText()
                        }

                        var message = dialog.key.addRow(v1, v2)
                        dialog.close()
                        if (message) {
                            notifyDialog.showError(message)
                        }
                    }
                }

                Button {
                    text: "Cancel"
                    onClicked: dialog.close()
                }



            }

        }

    }


}
