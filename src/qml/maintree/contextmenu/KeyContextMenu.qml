import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 1.4
import QtQuick.Dialogs 1.2
import Qt.labs.settings 1.0
import "./../../common"
import "./../../maindata"
import "./../"
import "./../../common/helper.js" as JS
import org.redinav.qml 1.0

Menu {

    id: root

    property var index
    property var terminalActivated

    property var connectionIndex:  mainTreeModel.connectionParent(index)
    property var databaseIndex: mainTreeModel.databaseParent(index)
    property var connection: mainTreeModel.roleValue(connectionIndex, "connectionpointer")
    property var connSettings: backend.getConnectionConfigByIndex(connectionIndex)
    property bool readOnly: connSettings.readOnly
    property var dbNumber: mainTreeModel.roleValue(databaseIndex, "dbnumber")
    property string keyFullPath: mainTreeModel.roleValue(index, "fullpath")
    property var selectedIndexes : mainTreeView.selection.selectedIndexes.length > 0 ? mainTreeView.selection.selectedIndexes : [index]
    property var connectionMode: mainTreeView.model.roleValue(connectionIndex, "connection_mode")



    MenuItem {
        visible: connectionMode === 0
        enabled: !readOnly && (!limiter.isActive || limiter.canAccess(Limiter.CloneKey)) && !(selectedIndexes.length > 1)
        text: "Clone"
        iconSource: qsTr("qrc:/resources/images/icons/themes/%1/ol_clone.svg").arg(JS.themeFix(approot.theme))
        onTriggered: {
            approot.keyRenameOrCloneDialog(connection, dbNumber, root.keyFullPath, root.index, true)
        }
    }



    MenuItem {
        enabled: !(selectedIndexes.length > 1)
        text: "Open in new tab" + JS.shortcutTipIf("keyinnewtab")
        iconSource: qsTr("qrc:/resources/images/icons/themes/%1/ol_new_tab.svg").arg(JS.themeFix(approot.theme))
        onTriggered: {
            mainTreeView.redisKeyActivated(root.index, true)
        }
    }

    MenuItem {
        visible: connectionMode === 0
        enabled: !readOnly && !(selectedIndexes.length > 1)
        text: "Rename" + JS.shortcutTipIf("rename")
        iconSource: qsTr("qrc:/resources/images/icons/themes/%1/ol_rename.svg").arg(JS.themeFix(approot.theme))
        onTriggered: {
            approot.keyRenameOrCloneDialog(connection, dbNumber, root.keyFullPath, root.index, false)
        }
    }

    MenuItem {
        enabled: !limiter.isActive || limiter.canAccess(Limiter.DumpKeys)
        text: {
            if (selectedIndexes && selectedIndexes.length > 1) {
                return qsTr("Dump %1 keys to JSON file").arg(selectedIndexes.length)
            }
            else {
                return "Dump key to JSON file"
            }
        }
        iconSource: qsTr("qrc:/resources/images/icons/themes/%1/ol_export.svg").arg(JS.themeFix(approot.theme))
        onTriggered: {

            if (JS.someDumpIsRunning()) {
                notifyDialog.showMsg("Another dump is already running. Please try later!")
                return
            }

            function dumpGo() {

                fileSelector.title = qsTr("Dump key(s) to JSON")
                fileSelector.nameFilters = ["JSON Dump (*.json)", "All files (*.*)"]
                fileSelector.selectExisting = false
                fileSelector.callback = function(fileUrl) {
                    var message = backend.dumpKeysToFileByKeyIndexes(
                        connection,
                        dbNumber,
                        selectedIndexes.length > 1 ? selectedIndexes : [index],
                        backend.getPathFromUrl(fileUrl)
                    )
                    if (message) {
                        notifyDialog.showMsg(message)
                    }
                }
                fileSelector.open()

            }

            if (selectedIndexes.length <= 0) {
                return
            }

            if (selectedIndexes.length > 150) {
                // Warn and ask for confirmation to Go
                yesno.text = JS.dumpWarning;
                yesno.yesCallback = function() {
                    dumpGo()
                }
                yesno.open()
            } else {
                dumpGo();
            }


        }
    }

    MenuItem {
        enabled: true
        text: "Terminal"
        iconSource: qsTr("qrc:/resources/images/icons/themes/%1/console.svg").arg(JS.themeFix(approot.theme))
        onTriggered: {
            root.terminalActivated()
        }
    }


    MenuSeparator { }

    MenuItem {
        enabled: (!readOnly && (!limiter.isActive || limiter.canAccess(Limiter.DeleteKey))) && (selectedIndexes.length === 1 || connectionMode === 0)
        text: {
            if (selectedIndexes && selectedIndexes.length > 1) {
                qsTr("Delete %1 keys").arg(selectedIndexes.length) + JS.shortcutTipIf("deletekey")
            }
            else {
                return "Delete" + JS.shortcutTipIf("deletekey")
            }
        }
        iconSource: qsTr("qrc:/resources/images/icons/themes/%1/ol_delete.svg").arg(JS.themeFix(approot.theme))
        onTriggered: {
            if (selectedIndexes.length > 0) {
                approot.keyDeleteDialog(connection, dbNumber, selectedIndexes, index)
            }
        }
    }



}





