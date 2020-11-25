import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 1.4
import org.redinav.qml 1.0
import "../common"

Rectangle {
    color: activePalette.window
    id: keyRoot
    property var connSettings: backend.getConnectionConfigByIndex(mainTreeModel.connectionParent(keyRoot.keyModelIndex))
    property bool readOnly: keyRoot.connSettings.readOnly

    property var    keyModelIndex: null
    property var    key: null
    property var    keyEd: null // key editor wrapper -> KeyEditor

    StringKey {
        id: key
        connection: kedRoot.connection
        keyFullPath: kedRoot.keyFullPath
        dbNumber: kedRoot.dbNumber
    }

    Component.onCompleted: {
        keyRoot.key = key
        key.loadRawKeyData();
    }

    Connections {
        target: key
        onValueChanged: {
            editorRoot.setText(key.value)
        }
    }

    ColumnLayout {
        anchors.fill: parent

        RowLayout {
            Rectangle {
                Layout.fillHeight: true
                Layout.fillWidth: true
                color: "transparent"
                TextEditor {
                    id: editorRoot
                    Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                    formattersButtons: {
                        return ["phpunserialize", "json-pretty", "minify", "original"]
                    }
                    bottomSpacer: true;
                }
            }
        }

        RowLayout {
            Layout.alignment: Qt.AlignRight | Qt.AlignBottom
            Layout.fillWidth: true
            Button {
                iconSource: qsTr("qrc:/resources/images/icons/themes/%1/ol_save.svg").arg(approot.theme)
                text: " " + qsTr("Save key to Redis")
                enabled: !editorRoot.isReadOnly && !keyRoot.readOnly
                onClicked: {
                    yesno.text = qsTr("Save key to Redis?")
                    yesno.yesCallback = function() {
                        var message = key.updateKey("", editorRoot.getText(),"", "");
                        if (message) {
                            yesno.close()
                            notifyDialog.showError(message)
                        }
                    }
                    yesno.open()
                }

            }
        }

    }




}
