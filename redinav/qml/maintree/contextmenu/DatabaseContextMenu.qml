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
    property var connection: mainTreeModel.roleValue(connectionIndex, "connectionpointer")
    property var dbNumber: mainTreeModel.roleValue(index, "dbnumber")
    property var filter: mainTreeView.model.roleValue(index, "filter")
    property var connSettings: backend.getConnectionConfigByIndex(connectionIndex)
    property bool readOnly: connSettings.readOnly


    Component.onCompleted: {
    }

    MenuItem {
        text: "Reload" + JS.shortcutTipIf("reload")
        iconSource: qsTr("qrc:/resources/images/icons/themes/%1/ol_refresh.svg").arg(JS.themeFix(approot.theme))
        onTriggered: {
            backend.reloadKeys(index)
            mainDataArea.removeAllTabs(["terminal", "serverinfo"]);
        }
    }
    MenuItem {
        enabled: !root.readOnly && (!limiter.isActive || limiter.canAccess(Limiter.AddNewKey))
        text: "Add Key" + JS.shortcutTipIf("addnewkey")
        iconSource: qsTr("qrc:/resources/images/icons/themes/%1/ol_add.svg").arg(JS.themeFix(approot.theme))
        onTriggered: {
            approot.addKeyDialog(index)
        }
    }


    MenuItem {
        enabled: !limiter.isActive || limiter.canAccess(Limiter.DumpDatabase)
        text: {
            var baseText = "Dump database #" + mainTreeModel.roleValue(root.index, "dbnumber") + " keys to file"
            return baseText + (root.filter && root.filter.length > 0 ? "  (filtered!)" : "")
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
                    var message = backend.dumpKeysToFileByDatabase(
                        root.connection,
                        root.dbNumber,
                        root.filter,
                        backend.getPathFromUrl(fileUrl)
                    )
                    if (message) {
                        notifyDialog.showMsg(message)
                    }
                }
                fileSelector.open()
            }

            // Warn and ask for confirmation to Go
            yesno.text = JS.dumpWarning;
            yesno.yesCallback = function() {
                dumpGo()
            }
            yesno.open()


        }
    }


    MenuItem {
        enabled: !root.readOnly && (!limiter.isActive || limiter.canAccess(Limiter.ImportKeys))
        text: "Import keys from JSON"
        iconSource: qsTr("qrc:/resources/images/icons/themes/%1/ol_import.svg").arg(JS.themeFix(approot.theme))
        onTriggered: {
            fileSelector.title = qsTr("Import keys from JSON")
            fileSelector.nameFilters = ["JSON Dump (*.json)", "All files (*.*)"]
            fileSelector.selectExisting = true
            fileSelector.callback = function(fileUrl) {
                var message = backend.importKeysFromJson(
                            root.connection,
                            root.dbNumber,
                            backend.getPathFromUrl(fileUrl),
                            index)
            }
            fileSelector.open()
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

    MenuItem {
        text: "Filter" + JS.shortcutTipIf("filter")
        iconSource: qsTr("qrc:/resources/images/icons/themes/%1/ol_filter.svg").arg(JS.themeFix(approot.theme))
        onTriggered: {
            approot.dbFilterDialog(index)
        }
    }
    MenuItem {
        text: "Clear Filter" + JS.shortcutTipIf("clearfilter")
        iconSource: qsTr("qrc:/resources/images/icons/themes/%1/ol_filter_cancel.svg").arg(JS.themeFix(approot.theme))
        onTriggered: {
            approot.dbFilterClear(index)
        }
    }

    MenuSeparator { }

    MenuItem {
        id: flushDbItem
        property bool disabled: true
        enabled: !disabled && !root.readOnly && (!limiter.isActive || limiter.canAccess(Limiter.FlushDatabase))
        text: qsTr("Flush database #%1 (dangerous)").arg(root.dbNumber)
        iconSource: qsTr("qrc:/resources/images/icons/themes/%1/ol_flush_red_broom.svg").arg(JS.themeFix(approot.theme))
        onTriggered: {
            yesno.text  = qsTr("Erase all keys in database # %1 ?").arg(root.dbNumber)
            yesno.text += "\n" + qsTr("This operation cannot be undone!")
            yesno.yesCallback = function() {
                backend.flushDb(index)
                mainDataArea.removeAllTabs(["terminal", "serverinfo"])
            }
            yesno.open();
        }
    }

    Settings {
        id: globalSettings
        category: "app"
        property alias disableFlushDb: flushDbItem.disabled
    }



}




