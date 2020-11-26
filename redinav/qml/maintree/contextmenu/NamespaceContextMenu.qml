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
    property var filter: mainTreeView.model.roleValue(databaseIndex, "filter")
    property var prefix: mainTreeModel.roleValue(index, "namespacefullpath")
    property var connectionMode: mainTreeView.model.roleValue(connectionIndex, "connection_mode")



    MenuItem {
        enabled: true
        text: "Terminal"
        iconSource: qsTr("qrc:/resources/images/icons/themes/%1/console.svg").arg(JS.themeFix(approot.theme))
        onTriggered: {
            root.terminalActivated()
        }
    }

    MenuItem {
        enabled: !limiter.isActive || limiter.canAccess(Limiter.DumpNamespaces)
        text: {
            var baseText = "Dump '" + mainTreeModel.roleValue(root.index, "namespacefullpath") + "' to file"
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
                fileSelector.nameFilters = ["Connections (*.json)", "All files (*.*)"]
                fileSelector.selectExisting = false
                fileSelector.callback = function(fileUrl) {
                    var message = backend.dumpKeysToFileByNamespace(
                        root.connection,
                        root.dbNumber,
                        root.prefix,
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

    MenuSeparator { }

    MenuItem {
        id: deleteNamespaceItem
        property bool disabled: true
        enabled: !disabled && !root.readOnly && (root.connectionMode === 0)
        text: "Delete keys" + (root.filter !== "" ? " (filtered!)" : "")
        iconSource: qsTr("qrc:/resources/images/icons/themes/%1/ol_delete").arg(JS.themeFix(approot.theme))
        onTriggered: {
            JS.loader("qrc:/qml/maintree/DeleteNamespaceDialog.qml", {
                filter: root.filter,
                connection: root.connection,
                dbNumber: root.dbNumber,
                index: root.index,
                prefix: root.prefix
            }, function(loader) {
                loader.item.open()
            })
        }
    }


    Connections {
        target: backend
        onNamespaceKeysDeletionFinished : {
            if (result) {
                mainTreeModel.removeRows(index.row, 1, mainTreeModel.parent(index))
            }
            else {
                notifyDialog.showError(message)
            }
        }
    }


    Settings {
        id: globalSettings
        category: "app"
        property alias disableNamespaceDelete: deleteNamespaceItem.disabled
    }



}
