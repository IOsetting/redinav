import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Dialogs 1.2
import QtQuick.Window 2.3
import "../common"
import "../common/helper.js" as JS

Dialog {

    property var dbItemIndex
    property var connection
    property int dbNumber

    id: root
    title: " new key"
    width: 550
    height: 500
    visible: false

    standardButtons: StandardButton.NoButton

    onVisibilityChanged: {
        if (visible === true){
            keyName.focus = true
        }
    }

    property var successCallback

    Item {
        anchors.fill: parent

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 5

            RowLayout {

                Text {
                    color: activePalette.text
                    Layout.minimumWidth: 100
                    text: "Key Name"
                    Layout.alignment: Qt.AlignLeft
                }

                TextField {
                    id: keyName
                    Layout.alignment: Qt.AlignLeft
                    Layout.fillWidth: true
                }

            }

            RowLayout {
                Text {
                    color: activePalette.text
                    Layout.minimumWidth: 100
                    text: "Key Type"
                    Layout.alignment: Qt.AlignLeft
                }

                ComboBox {
                    id: typeSelector
                    Layout.alignment: Qt.AlignLeft
                    model: JS.getSupportedKeyTypes()
                    Layout.fillWidth: true
                }
            }

            // HASH KEY
            RowLayout {
                visible:  typeSelector.currentText === "hash"
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
                visible: typeSelector.currentText === "zset"
                Text {
                    color: activePalette.text
                    Layout.minimumWidth: 100
                    text: "Score"
                    Layout.alignment: Qt.AlignLeft
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
                    TextEditor  {
                        id: keyValue
                        label: typeSelector.currentText === "string" ?  "String value" : JS.keyTypeMemberTitle(typeSelector.currentText, true) + " value"
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
                    text: "Save"
                    onClicked: {
                        var v1
                        var v2
                        var keyType = typeSelector.currentText

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

                        if (!JS.validateAddKeyValues(keyName.text, typeSelector.currentText, v1, v2)) {
                            notifyDialog.showError("Invalid data")
                            return
                        }

                        backend.addNewKey(root.connection, root.dbNumber, typeSelector.currentText, keyName.text, v1, v2, function(result, message) {
                            if (result) {
                                var newIndex = mainTreeModel.addRowWithData(dbItemIndex, {
                                                                                "name"      : keyName.text,
                                                                                "type"      : "key",
                                                                                "dbnumber"  : "" + root.dbNumber,
                                                                                "fullpath"  : keyName.text,
                                                                                "filter"    : ""
                                                                            });
                                mainTreeView.selection.setCurrentIndex(newIndex, 3) // clear && select
                                if (typeof root.successCallback === "function") {
                                    root.successCallback(newIndex)
                                }
                                root.close()
                            }
                            else {
                                notifyDialog.showError(message)
                                root.close()
                            }
                        })
                    }
                }

                Button {
                    text: "Cancel"
                    onClicked: root.close()
                }



            }

        }

    }


}
