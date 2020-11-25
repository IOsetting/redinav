import "../common/helper.js" as JS
import "../common"
import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 1.4
import QtQuick.Dialogs 1.2
import QtQuick.Controls.Styles 1.4

import org.redinav.qml 1.0

Rectangle  {

    id: kedRoot

    //color: activePalette.alternateBase
    color: "transparent"

    property var connSettings: backend.getConnectionConfigByIndex(mainTreeModel.connectionParent(kedRoot.keyModelIndex))
    property bool readOnly: kedRoot.connSettings.readOnly

    // Properties set MainDataArea when THIS QML is loaded
    property string tabType: "keyeditor"
    property string tabIdentifier: ""
    property var    keyModelIndex: null
    property string keyGlobalFullPath: ""
    property string keyName: mainTreeModel.roleValue(keyModelIndex, "name")
    property string keyFullPath: mainTreeModel.roleValue(keyModelIndex, "fullpath")
    property var    connection: mainTreeModel.roleValue(mainTreeModel.connectionParent(keyModelIndex), "connectionpointer")
    property var connectionMode: mainTreeModel.roleValue(mainTreeModel.connectionParent(keyModelIndex), "connection_mode")
    property var    dbNumber:  mainTreeModel.roleValue(mainTreeModel.databaseParent(keyModelIndex), "dbnumber")
    property var    key: null
    property string keyType: ""
    property bool   busy: false
    property var connectionConfig: null
    property bool isMultirowEditor


    function getEditorByType() {

        switch (keyType) {
        case "string":
            kedRoot.isMultirowEditor = false
            return "qrc:/qml/maindata/StringKeyEditor.qml"
        case "set":
        case "list":
        case "hash":
        case "zset":
            kedRoot.isMultirowEditor = true
            return "qrc:/qml/maindata/MultirowEditorArea.qml"
        }

    }

    // See keyeditor.cpp
    KeyEditor {
        id: keyEd
        connection: kedRoot.connection
        keyFullPath: kedRoot.keyFullPath
        dbNumber: kedRoot.dbNumber
        onKeyEditorReady: {
            kedRoot.keyType = keyType;
            keyEditorUi.setSource(getEditorByType(), {
                "keyType" : keyType,
                "keyModelIndex" : keyModelIndex,
                "keyEd": keyEd
            })
            keyEditorUi.active = true;
            wrapper.visible = true
            kedRoot.connectionConfig = keyEd.getConnectionConfig()
        }
    }

    // When THIS QML is completed, call keyeditor.cpp method to load key Type
    // It will emit keyEditorReady
    Component.onCompleted:  {
        keyEd.loadKeyType()
        keyEd.loadKeyTtl()
    }

    // ---------------
    Item {
        id: wrapper
        visible: false


        anchors.fill: parent
        anchors.margins: 0

        ColumnLayout {
            anchors.fill: parent
            spacing: 1

            RowLayout {
                Layout.fillWidth: true
                Layout.maximumHeight: 18

                Image {
                    visible: kedRoot.readOnly
                    Layout.maximumHeight: 18
                    Layout.maximumWidth: 18
                    Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                    fillMode: Image.PreserveAspectFit || Image.Stretch
                    source: qsTr("qrc:/resources/images/icons/themes/%1/padlock3_red_locked.svg").arg(approot.theme)
                    mipmap: true
                    smooth: true
                }

                Text {
                    color: activePalette.text
                    text: kedRoot.connectionConfig ?
                        kedRoot.connectionConfig.name + " (" + kedRoot.connectionConfig.host + ":" + kedRoot.connectionConfig.port + ")"  + ", Database #" + kedRoot.dbNumber
                        : ""
                }
            }

            RowLayout {
                Layout.fillWidth: true
            }


            RowLayout {
                Layout.preferredHeight: 40
                Layout.minimumHeight: 40
                Layout.fillWidth: true
                spacing: 5

                Text {
                    text: keyType.toUpperCase() + " "
                    color: activePalette.text
                    font.bold: true
                }

                TextField {
                    id: keyNameField
                    Layout.fillWidth: true
                    text: {
                        return kedRoot.keyFullPath
                    }
                    readOnly: true
                }

                Item { Layout.preferredWidth: 5}
                Text { color: activePalette.text; text: qsTr("TTL:"); font.bold: true }
                Text { color: activePalette.text; text: keyEd.ttl < 0 ? "-" : keyEd.ttl }
                Item { Layout.preferredWidth: 5}

                Button {
                    visible: connectionMode === 0
                    enabled: !readOnly
                    text: ""
                    tooltip: qsTr("Rename")
                    iconSource: qsTr("qrc:/resources/images/icons/themes/%1/ol_rename.svg").arg(approot.theme)
                    onClicked: {
                        approot.keyRenameOrCloneDialog(kedRoot.connection, kedRoot.dbNumber, kedRoot.keyFullPath, kedRoot.keyModelIndex, false)
                    }
                }

                Button {
                    visible: connectionMode === 0
                    enabled: !readOnly && (!limiter.isActive || limiter.canAccess(Limiter.CloneKey))
                    text: ""
                    tooltip: qsTr("Clone")
                    iconSource: qsTr("qrc:/resources/images/icons/themes/%1/ol_clone.svg").arg(approot.theme)
                    onClicked: {
                        approot.keyRenameOrCloneDialog(kedRoot.connection, kedRoot.dbNumber, kedRoot.keyFullPath, kedRoot.keyModelIndex, true)
                    }
                }

                Button {
                    enabled: !readOnly && (!limiter.isActive || limiter.canAccess(Limiter.DeleteKey))
                    text: ""
                    tooltip: qsTr("Delete")
                    iconSource: qsTr("qrc:/resources/images/icons/themes/%1/ol_delete.svg").arg(approot.theme)

                    MessageDialog {
                        id: deleteConfirmation
                        title: qsTr("Delete key")
                        text: qsTr("Do you really want to delete this key?")
                        onYes: {
                            backend.deleteKeysCollectionByIndexes(kedRoot.connection, kedRoot.dbNumber, [kedRoot.keyModelIndex]);
                        }
                        visible: false
                        icon: StandardIcon.Warning
                        standardButtons: StandardButton.Yes | StandardButton.No
                    }

                    onClicked: {
                        deleteConfirmation.open()
                    }
                }

                Button {
                    text: ""
                    tooltip: qsTr("Reload")
                    action: reLoadAction
                    visible: keyType === "string"
                    iconSource: qsTr("qrc:/resources/images/icons/themes/%1/ol_refresh.svg").arg(approot.theme)

                    Action {
                        id: reLoadAction
                        onTriggered: {
                            keyEditorUi.item.key.loadRawKeyData();
                            keyEd.loadKeyTtl()
                        }
                    }

                }

                Button {
                    enabled: !readOnly
                    text: ""
                    tooltip: qsTr("Set Expiration Time (TTL)")
                    iconSource: qsTr("qrc:/resources/images/icons/themes/%1/ol_timer.svg").arg(approot.theme)

                    Dialog {
                        id: setTTLConfirmation
                        title: qsTr("Set key TTL")

                        width: 520

                        onVisibilityChanged: {
                            if (visible === true){
                                newTTL.focus = true
                            }
                        }


                        RowLayout {
                            implicitWidth: 500
                            implicitHeight: 100
                            width: 500

                            Text {
                                text: qsTr("New TTL:")
                                color: activePalette.text
                            }
                            TextField {
                                id: newTTL;
                                Layout.fillWidth: true;
                                validator: IntValidator {
                                    bottom: -1
                                    locale: "C"
                                }
                                text: keyEd.ttl < 0 ? "" : keyEd.ttl
                            }
                        }

                        onAccepted: {

                            function setNewTtl() {
                                var message = backend.setTTL(kedRoot.connection, kedRoot.dbNumber, kedRoot.keyFullPath, newTTL.text)
                                if (message) {
                                    notifyDialog.showError(message)
                                }
                                else {
                                    keyEd.ttl = newTTL.text
                                }
                            }

                            if (newTTL.text.trim().length <= 0) {
                                newTTL.text = "-1"
                            }


                            // Check for some very short expire time and ask for confirmation
                            if (newTTL.text && (parseInt(newTTL.text) < 120) && (parseInt(newTTL.text) !== -1)) {
                                yesno.text = qsTr("You are setting a very short expire time. Key will be removed soon! Are you sure?")
                                yesno.yesCallback = function() {
                                    setNewTtl();
                                }
                                yesno.open()
                            }
                            else {
                                setNewTtl()
                            }

                        }

                        visible: false
                        standardButtons: StandardButton.Ok | StandardButton.Cancel
                    }

                    onClicked: {
                        setTTLConfirmation.open()
                    }
                }

                Button {
                    enabled: !readOnly
                    text: ""
                    tooltip: qsTr("Make Persistent (TTL=-1)")
                    iconSource: qsTr("qrc:/resources/images/icons/themes/%1/ol_infinity.svg").arg(approot.theme)
                    onClicked: {
                        var message = backend.setTTL(kedRoot.connection, kedRoot.dbNumber, kedRoot.keyFullPath, -1)
                        if (message) {
                            notifyDialog.showError(message)
                        }
                        else {
                            keyEd.ttl = "-1"
                        }
                    }
                }

            }

            RowLayout {
                Layout.fillWidth: true
            }


            RowLayout {
                Layout.fillHeight: true
                Layout.fillWidth: true


                // Load Key type specific QML
                Loader {
                    id: keyEditorUi
                    active: false
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.minimumHeight: 180
                    source: ""

                    onLoaded: {

                        if (keyEditorUi.item.key) {
                            keyEditorUi.item.key.keyRemoved.connect(function() {
                                var path = mainTreeModel.connectionParent(keyModelIndex).row + "-" + mainTreeModel.databaseParent(keyModelIndex).row + "-" + keyFullPath
                                mainDataArea.removeKeyTabs(path)
                                mainTreeModel.removeRows(keyModelIndex.row, 1, mainTreeModel.parent(keyModelIndex))

                            });
                            kedRoot.key = keyEditorUi.item.key;

                        }

                    }

                }


            }
        }
    }


    Rectangle {
        id: uiBlocker
        visible: kedRoot.busy
        anchors.fill: parent
        color: Qt.rgba(0, 0, 0, 0.1)

        Item {
            anchors.fill: parent
            BusyIndicator { anchors.centerIn: parent; running: true }
        }

        MouseArea {
            anchors.fill: parent
        }
    }



}

