import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Dialogs 1.2
import QtQuick.Window 2.3
import QtQml.Models 2.3
import Qt.labs.settings 1.0
import "contextmenu"
import "../common"
import "../common/helper.js" as JS
import "../maindata"

TreeView {

    function reload(currentIndex) {

        if (!mainTreeModel.isValid(currentIndex)) {
            return
        }

        var index = currentIndex
        var type = mainTreeModel.roleValue(index, "type")
        var name = ""
        var reloadableIndex = index

        if (type === "key" || type === "namespace") {
            reloadableIndex = mainTreeModel.databaseParent(index)
        }

        if (reloadableIndex) {
            if (type !== "server")
                backend.reloadKeys(reloadableIndex)
            else {
                mainDataArea.removeTabsByConnection(mainTreeModel.roleValue(reloadableIndex, "connectionpointer"))
                backend.disconnectConnection(reloadableIndex);
                backend.connect(reloadableIndex);
            }
            mainTreeView.selection.setCurrentIndex(reloadableIndex, 3) // clear && select
            mainDataArea.removeAllTabs(["terminal", "serverinfo"])
        }

    }


    signal redisKeyActivated(var index, var newTab)
    signal serverStatsActivated(var index)
    signal terminalActivated(var index, var dbNumber)

    id: mainTreeView

    property bool doubleClickTreeItemActivation: false;

    width: 300
    antialiasing: true
    headerVisible: false

    style: TreeViewStyle {

        activateItemOnSingleClick: !mainTreeView.doubleClickTreeItemActivation

        branchDelegate: Item {
            width: 16
            height: 16
            Text {
                visible: styleData.column === 0 && styleData.hasChildren
                text: styleData.isExpanded ? "\u25bc" : "\u25b6"
                color: styleData.selected ? activePalette.highlightedText : activePalette.text
                font.pointSize: 8
                renderType: Text.NativeRendering
                anchors.centerIn: parent
                anchors.verticalCenterOffset: styleData.isExpanded ? 1 : 0
            }
        }

        backgroundColor: activePalette.window
        alternateBackgroundColor: activePalette.alternateBase

    }


    TableViewColumn {
        id: nameColumn
        role: "name"
        resizable: true
    }

    itemDelegate: FocusScope {

        function getSource(type, isLocked) {

            if (isLocked) {
                return "qrc:/resources/images/icons/wait.svg"
            }

            switch (type) {
            case "server":
                if (styleData.hasChildren)
                    return "qrc:/resources/images/icons/redis_online.svg"
                else
                    return "qrc:/resources/images/icons/redis_offline.svg"
            case "database":
                if (styleData.hasChildren)
                    return "qrc:/resources/images/icons/ol_database_red.svg"
                else
                    return "qrc:/resources/images/icons/ol_database_grey.svg"
            case "namespace":
                if (styleData.isExpanded)
                    return "qrc:/resources/images/icons/folder_opened.svg"
                else
                    return "qrc:/resources/images/icons/folder_closed.svg"

            }
            return "";
        }


        function getHeightScale(type) {
            return 80
        }


        Item {

            id: wrapper

            // Every item
            property var  type                      : mainTreeView.model.roleValue(styleData.index, "type")

            // Server items only
            property var  connectionIndex           : type === "server" ? styleData.index : null
            property var  connSettings              : type === "server" ? backend.getConnectionConfigByIndex(connectionIndex) : null
            property bool readOnly                  : type === "server" ? connSettings.readOnly : false
            property string connectionFilter        : type === "server" ? connSettings.defaultFilter : ""
            property var  connectionMode            : type === "server" ? mainTreeView.model.roleValue(connectionIndex, "connection_mode") : null

            // DB items only
            property var  filter                    : type === "database" ? mainTreeView.model.roleValue(styleData.index, "filter") : null


            anchors.fill: parent

            IconSVG {
                size: 15
                visible: wrapper.type !== "key"
                id: itemIcon
                source: getSource(parent.type, mainTreeView.model.isLocked(styleData.index))
            }

            Text {
                id: itemText

                anchors.left: {
                    if (wrapper.type !== "key")
                        return itemIcon.right
                    else
                        return wrapper.left
                }
                text: {
                    var countInfo = ""
                    var totalKeys = mainTreeView.model.roleValue(styleData.index, "totalkeys")
                    if (["database", "namespace"].indexOf(wrapper.type) !== -1 && !((wrapper.connectionMode === 2) && totalKeys === 0)) {
                        countInfo = " (" + totalKeys + ")"
                    }
                    var result = styleData.value + countInfo
                    if (wrapper.type === "database" && wrapper.filter !== "") {
                        result = result + " [" + wrapper.filter + "]"
                    }

                    if (wrapper.type === "server") {
                        return " " + result + (wrapper.connectionFilter ? " [" + wrapper.connectionFilter + "]"  : "")
                    }
                    else {
                        return " " + result
                    }
                }
                color: styleData.selected ? activePalette.highlightedText : activePalette.text
            }


            RowLayout {
                id: serverAdditionalIcons
                anchors.right: parent.right
                anchors.rightMargin: 30
                height: parent.height
                spacing: 2
                property int iconWidthCorrection: JS.isOSX() ? 5 : 10
                property int iconPercentageHeight: JS.isOSX() ? 70 : 50

                function iconMinWidth(pHeight) {
                    return Math.round(pHeight * iconPercentageHeight/100)
                }

                Rectangle {
                    id: clusterIcon
                    visible: wrapper.type === "server" && (wrapper.connectionMode === 2)
                    Layout.fillHeight: true
                    Layout.minimumWidth: serverAdditionalIcons.iconMinWidth(parent.height)
                    color: "transparent"
                    IconSVG {
                        size: parent.height - serverAdditionalIcons.iconWidthCorrection
                        source: qsTr("qrc:/resources/images/icons/themes/%1/ol_cluster.svg").arg(approot.theme)
                    }

                }

                Rectangle {
                    id: encryptedIcon
                    visible: wrapper.type === "server" && wrapper.connSettings.isEncrypted
                    Layout.fillHeight: true
                    Layout.minimumWidth: serverAdditionalIcons.iconMinWidth(parent.height)
                    color: "transparent"
                    IconSVG {
                        size: parent.height - serverAdditionalIcons.iconWidthCorrection
                        source: qsTr("qrc:/resources/images/icons/themes/%1/ol_key.svg").arg(approot.theme)
                    }

                }

                Rectangle {
                    id: lockedIcon
                    visible: wrapper.type === "server" && wrapper.readOnly
                    Layout.fillHeight: true
                    Layout.minimumWidth: serverAdditionalIcons.iconMinWidth(parent.height)
                    color: "transparent"
                    IconSVG {
                        size: parent.height - serverAdditionalIcons.iconWidthCorrection
                        source: qsTr("qrc:/resources/images/icons/themes/%1/ol_padlock_locked.svg").arg(approot.theme)
                    }

                }

            }


            MouseArea {

                anchors.fill: parent
                acceptedButtons: Qt.RightButton
                onClicked : {
                    mouse.accepted = true
                    var index = styleData.index
                    var itemType = wrapper.type
                    var dbNumber

                    if (index.valid && !mainTreeModel.isLocked(index)) {

                        switch (itemType) {

                        // SERVER
                        case "server":
                            JS.loader(
                                "qrc:/qml/maintree/contextmenu/ServerContextMenu.qml",
                                {
                                    index               : index,
                                    terminalActivated   : function() { terminalActivated(index, 0) },
                                }, function(loader) {
                                    loader.item.popup()
                                }
                            );
                            break;

                        // DATABASE
                        case "database":
                            dbNumber = mainTreeModel.roleValue(styleData.index, "dbnumber")
                            JS.loader(
                                "qrc:/qml/maintree/contextmenu/DatabaseContextMenu.qml",
                                {
                                    index               : index,
                                    terminalActivated   : function() { terminalActivated(mainTreeModel.connectionParent(index), dbNumber) }
                                }, function(loader) {
                                    loader.item.popup()
                                }
                            );
                            break;

                        // KEY
                        case "key":
                            dbNumber = mainTreeModel.roleValue(mainTreeModel.databaseParent(index), "dbnumber")
                            JS.loader(
                                "qrc:/qml/maintree/contextmenu/KeyContextMenu.qml",
                                {
                                    index               : index,
                                    terminalActivated   : function() { terminalActivated(mainTreeModel.connectionParent(index), dbNumber) }
                                }, function(loader) {
                                    loader.item.popup()
                                }
                            );
                            break;

                        // NAMESPACE
                        case "namespace":
                            dbNumber = mainTreeModel.roleValue(mainTreeModel.databaseParent(index), "dbnumber")
                            JS.loader(
                                "qrc:/qml/maintree/contextmenu/NamespaceContextMenu.qml",
                                {
                                    index                   : index,
                                    terminalActivated       : function() { terminalActivated(mainTreeModel.connectionParent(index), dbNumber) }
                                }, function(loader) {
                                    loader.item.popup()
                                }
                            );
                            break;
                        }

                    }

                }

            }



            focus: true


        }
    }

    selectionMode: SelectionMode.ExtendedSelection

    selection: ItemSelectionModel {
        property var firstParent
        id: treeSelectionModel
        model: mainTreeModel
        onSelectionChanged: {
            var firstParent = mainTreeModel.parent(selectedIndexes[0])
            var lastParent = mainTreeModel.parent(selectedIndexes[selectedIndexes.length-1])
            if (lastParent !== firstParent) {
                clearSelection()
                return
            }
        }
    }

    model: mainTreeModel


    onActivated: {

        if (mainTreeModel.isLocked(index)) {
            return;
        }

        var type = mainTreeModel.roleValue(index, "type")

        switch (type) {

        // SERVER
        case "server":
            if (mainTreeModel.rowCount(index) <= 0) {
                backend.connect(index);
            }
            break;

        // DATABASE | NAMESPACE
        case "namespace":
        case "database":
            if (mainTreeModel.rowCount(index) > 0) {
                return
            }
            backend.loadKeys(index)
            break

        case "key":
            redisKeyActivated(index, false)
            break

        }


    }



    Connections {
        target: mainTreeModel
        onLoadKeysFinished: {
            expand(index);
        }
        onDatabasesLoaded: {
            expand(index)
        }

        onDataChanged: {
        }
    }

    Connections {
        target: backend
        onKeysDeleted: {  // result, message, indexes

            if (!result) {
                notifyDialog.showError(message)
                return
            }
            if (indexes.length <= 0) {
                return;
            }

            var dbIndex = mainTreeModel.databaseParent(indexes[0])
            var dbRow   = dbIndex.row
            var connRow = mainTreeModel.connectionParent(indexes[0]).row
            for (var i=0; i < indexes.length; i++) {
                mainDataArea.removeKeyTabs(connRow + "-" + dbRow + "-" + mainTreeModel.roleValue(indexes[i], "fullpath"))
            }

            if (indexes.length > 500) {
                backend.reloadKeys(mainTreeModel.databaseParent(indexes[0]))
            }
            else {
                mainTreeModel.removeIndexes(indexes)
            }

            mainTreeView.selection.select(dbIndex, 3) // Clear && Select


        }

        onClusterModeNotSupported: {
            notifyDialog.showError(message, "Cluster mode")
        }

    }

    RenameKeyDialog {
        id: renameKeyDialog
    }

    AddKeyDialog {
        id: addNewKeyDialog
    }

    DeleteKeyDialog {
        id: deleteKeyDialog
    }


    // --------------
    // Shortcuts are NOT defined in MenuItems because they are dynamic, global and key sequences might be duplicated, like Ctrl+R
    // --------------

    Shortcut {
        enabled: mainTreeView.activeFocus
        sequences: JS.shortcuts("reload")
        onActivated: {
            if (!mainTreeView.selection.hasSelection) {
                return
            }
            reload(mainTreeView.selection.currentIndex)
        }
    }

    Shortcut {
        enabled: mainTreeView.activeFocus && mainTreeModel.isOfType(mainTreeView.selection.currentIndex, ["server"])
        sequences: JS.shortcuts("editconn")
        onActivated: {
            JS.loader("qrc:/qml/settings/ConnectionSettignsDialog.qml", {
                "settings" : backend.getConnectionConfigByIndex(mainTreeView.selection.currentIndex)
            },
            function(loader) {
                loader.item.open()
            })
        }
    }


    Shortcut {
        enabled: mainTreeView.activeFocus && mainTreeModel.isOfType(mainTreeView.selection.currentIndex, ["server"])
        sequences: JS.shortcuts("serverinfo")
        onActivated: {
            serverStatsActivated(mainTreeView.selection.currentIndex)
        }
    }

    Shortcut {
        enabled: {
            return mainTreeView.activeFocus
                    && mainTreeModel.isOfType(mainTreeView.selection.currentIndex, ["key"])
                    && !backend.getConnectionConfigByIndex(mainTreeModel.connectionParent(mainTreeView.selection.currentIndex)).readOnly
        }
        sequences: JS.shortcuts("rename")
        onActivated: {
            if (mainTreeView.selection.selectedIndexes.length === 1) {
                approot.keyRenameOrCloneDialog(
                    mainTreeModel.getConnection(mainTreeView.selection.currentIndex),
                    mainTreeModel.getDbNumber(mainTreeView.selection.currentIndex),
                    mainTreeModel.roleValue(mainTreeView.selection.currentIndex, "fullpath"),
                    mainTreeView.selection.currentIndex,
                    false
                )
            }
        }
    }

    Shortcut {
        enabled: mainTreeView.activeFocus && mainTreeModel.isOfType(mainTreeView.selection.currentIndex, ["database", "namespace", "key"])
        sequences: JS.shortcuts("filter")
        onActivated: {
            var index = mainTreeModel.databaseParent(mainTreeView.selection.currentIndex)
            if (index) {
                approot.dbFilterDialog(index)
            }
        }
    }

    Shortcut {
        enabled: mainTreeView.activeFocus && mainTreeModel.isOfType(mainTreeView.selection.currentIndex, ["database", "namespace","key"])
        sequences: JS.shortcuts("clearfilter")
        onActivated: {
            var dbIndex = mainTreeModel.databaseParent(mainTreeView.selection.currentIndex)
            approot.dbFilterClear(dbIndex)
        }
    }

    Shortcut {
        enabled: {
            return mainTreeView.activeFocus
                    && mainTreeModel.isOfType(mainTreeView.selection.currentIndex, ["database", "key", "namespace"])
                    && !backend.getConnectionConfigByIndex(mainTreeModel.connectionParent(mainTreeView.selection.currentIndex)).readOnly
        }
        sequences: JS.shortcuts("addnewkey")
        onActivated: {
            var dbIndex = mainTreeModel.databaseParent(mainTreeView.selection.currentIndex)
            approot.addKeyDialog(dbIndex)
        }
    }

    Shortcut {
        enabled: mainTreeView.activeFocus && mainTreeModel.isOfType(mainTreeView.selection.currentIndex, ["key"])
        sequences: JS.shortcuts("keyinnewtab")
        onActivated: {
            if (mainTreeView.selection.selectedIndexes.length === 1) {
                redisKeyActivated(mainTreeView.selection.currentIndex, true)
            }
        }
    }

    Shortcut {
        enabled: {
            return mainTreeView.activeFocus
                    && (mainTreeView.selection.selectedIndexes.length > 0)
                    && mainTreeModel.isOfType(mainTreeView.selection.currentIndex, ["key"])
                    && !backend.getConnectionConfigByIndex(mainTreeModel.connectionParent(mainTreeView.selection.currentIndex)).readOnly
        }
        sequences: JS.shortcuts("deletekey")
        onActivated: {
            if (mainTreeView.selection.selectedIndexes.length > 0) {
                approot.keyDeleteDialog(
                    mainTreeModel.getConnection(mainTreeView.selection.selectedIndexes[0]),
                    mainTreeModel.getDbNumber(mainTreeView.selection.selectedIndexes[0]),
                    mainTreeView.selection.selectedIndexes,
                    mainTreeView.selection.selectedIndexes[0]
                )
            }
        }
    }

    Settings {
        id: globalSettings
        category: "app"
        property alias doubleClickTreeItemActivation: mainTreeView.doubleClickTreeItemActivation
    }


}
