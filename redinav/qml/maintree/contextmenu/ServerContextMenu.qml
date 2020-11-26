import QtQuick.Controls 1.4
import "../../common"
import "../../common/helper.js" as JS
import org.redinav.qml 1.0

Menu {

    id: root
    property var index
    property var terminalActivated

    property var connSettings: backend.getConnectionConfigByIndex(index)

    MenuItem {
        text: "Reload Server" + JS.shortcutTipIf("reload")
        iconSource: qsTr("qrc:/resources/images/icons/themes/%1/ol_refresh.svg").arg(JS.themeFix(approot.theme))
        onTriggered: {
            mainDataArea.removeTabsByConnection(mainTreeModel.roleValue(index, "connectionpointer"))
            backend.disconnectConnection(index);
            backend.connect(index);
        }
    }
    MenuItem {
        text: "Disconnect"
        iconSource: qsTr("qrc:/resources/images/icons/themes/%1/ol_disconnect.svg").arg(JS.themeFix(approot.theme))
        onTriggered: {
            mainDataArea.removeTabsByConnection(mainTreeModel.roleValue(index, "connectionpointer"))
            backend.disconnectConnection(index);
        }
    }
    MenuItem {
        enabled: !limiter.isActive || limiter.canAccess(Limiter.ServerInfo)
        text: "Server Information" + JS.shortcutTipIf("serverinfo")
        iconSource: qsTr("qrc:/resources/images/icons/themes/%1/ol_info.svg").arg(JS.themeFix(approot.theme))
        onTriggered: {
            mainTreeView.serverStatsActivated(root.index)
        }
    }

    MenuItem {
        enabled: true
        text: "Terminal"
        iconSource: qsTr("qrc:/resources/images/icons/themes/%1/console.svg").arg(JS.themeFix(approot.theme))
        onTriggered: {
            terminalActivated(index, 0)
        }
    }

    MenuItem {
        text: "Edit Connection Settings" + JS.shortcutTipIf("editconn")
        iconSource: qsTr("qrc:/resources/images/icons/themes/%1/ol_settings.svg").arg(JS.themeFix(approot.theme))
        onTriggered: {
            JS.loader("qrc:/qml/settings/ConnectionSettignsDialog.qml", {
                "settings" : connSettings
            },
            function(loader) {
                loader.item.open()
            })
        }
    }

    MenuItem {
        text: "Clone Connection"
        iconSource: qsTr("qrc:/resources/images/icons/themes/%1/ol_clone.svg").arg(JS.themeFix(approot.theme))
        onTriggered: {
            var message = backend.cloneConnection(index);
        }
    }

    MenuItem {
        visible: !root.connSettings.readOnly
        text: "Set to Read Only"
        iconSource: qsTr("qrc:/resources/images/icons/themes/%1/ol_padlock_locked.svg").arg(JS.themeFix(approot.theme))
        onTriggered: {
            root.connSettings.readOnly = true
            backend.updateConnection(root.connSettings)
        }
    }

    MenuItem {
        visible: root.connSettings.readOnly
        text: "Set to Read/Write"
        iconSource: qsTr("qrc:/resources/images/icons/themes/%1/ol_padlock_unlocked.svg").arg(JS.themeFix(approot.theme))
        onTriggered: {
            root.connSettings.readOnly = false
            backend.updateConnection(root.connSettings)
        }
    }


    MenuSeparator { }

    MenuItem {
        enabled: !root.connSettings.readOnly
        text: "Delete Connection"
        iconSource: qsTr("qrc:/resources/images/icons/themes/%1/ol_delete.svg").arg(JS.themeFix(approot.theme))
        onTriggered: {
            yesno.text = qsTr("Delete connection?")
            yesno.yesCallback = function() {
                mainDataArea.removeTabsByConnection(mainTreeModel.roleValue(index, "connectionpointer"))
                backend.removeConnection(index.row);
            }
            yesno.open();
        }
    }


}
