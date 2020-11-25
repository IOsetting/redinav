import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import Qt.labs.settings 1.0
import "."
import "../"
import "../common"
import "../terminal"
import "../common/helper.js" as JS

Rectangle {
    id: dataArea
    color: activePalette.window

    function removeKeyTabs(path) {
        var tabs = getTabsByKeyGlobalFullPath(path)
        while (tabs.length > 0) {
            dataTabsView.removeTab(tabs[0].index)
            tabs = getTabsByKeyGlobalFullPath(path)
        }
    }

    function removeTabsByConnection(connection) {
        var tabs = getTabsByConnection(connection)
        while (tabs.length > 0) {
            dataTabsView.removeTab(tabs[0].index)
            tabs = getTabsByConnection(connection)
        }

    }

    function getTabByIdentifier(identifier) {
        var tab
        for (var i=0; i < dataTabsView.count; i++) {
            tab = dataTabsView.getTab(i)
            if (tab.item.tabIdentifier === identifier) {
                return {tab: tab, index:i}
            }
        }
        return false
    }

    function getTabsByKeyGlobalFullPath(path) {
        var tabs = []
        var tab
        for (var i=0; i < dataTabsView.count; i++) {
            tab = dataTabsView.getTab(i)
            if (tab.item.keyGlobalFullPath === path && (tab.item.tabType === "keyeditor")) {
                tabs.push({tab: tab, index:i})
            }
        }
        return tabs
    }

    function getTabsByConnection(connection) {
        var tabs = []
        var tab
        for (var i=0; i < dataTabsView.count; i++) {
            tab = dataTabsView.getTab(i)
            if (tab.item.connection === connection && (tab.item.tabType === "keyeditor")) {
                tabs.push({tab: tab, index:i})
            }
        }
        return tabs
    }

    function getAllTabs(skipTypes) {
        var tabs = []

        for (var i=0; i < dataTabsView.count; i++) {
            if (skipTypes) {
                var tabType = dataTabsView.getTab(i).item.tabType
                if (skipTypes.indexOf(tabType) === -1) {
                    tabs.push({tab: dataTabsView.getTab(i), index:i})
                }
            }
            else {
                tabs.push({tab: dataTabsView.getTab(i), index:i})
            }
        }
        return tabs
    }

    function removeAllTabs(skipTypes) {
        var tabs = getAllTabs(skipTypes)
        while (tabs.length > 0) {
            dataTabsView.removeTab(tabs[0].index)
            tabs = getAllTabs(skipTypes)
        }
    }

    Welcome {
        id: welcome
        visible: dataTabsView.count <= 0
    }


    Menu {
        id: tabActions
        property int tabIndex: 0

        MenuItem {
            id: closeThis
            text: "Close"
            onTriggered: {
                dataTabsView.removeTab(tabActions.tabIndex)
            }
        }
        MenuItem {
            id: closeAll
            text: "Close All"
            onTriggered: {
                removeAllTabs();
            }
        }


    }

    TabView {

        id: dataTabsView
        visible: count > 0
        anchors.fill: parent
        anchors.margins: 5

        function getTabTypeIconSource(styleData) {

            if (!dataTabsView.getTab(styleData.index) || !dataTabsView.getTab(styleData.index).item) {
                return "";
            }

            // we need WHITE icons (i.e. from dark theme) when tab is selected
            var theme = styleData.selected ? "dark" : approot.theme;
            var type = dataTabsView.getTab(styleData.index).item.tabType
            var source = "";

            switch (type) {
            case "terminal":
                source = qsTr("qrc:/resources/images/icons/themes/%1/console.svg").arg(theme)
                break;
            case "keyeditor":
                source = ""
                break;
            case "serverinfo":
                source = qsTr("qrc:/resources/images/icons/themes/%1/ol_info.svg").arg(theme)
                break;
            }


            return source

        }


        function tabTypeIconVisible(styleData) {
            if (!dataTabsView.getTab(styleData.index)) {
                return false;
            }
            var item = dataTabsView.getTab(styleData.index).item
            var visible = (item !== null) && (item !== undefined) && (item.tabType !== "keyeditor");
            return visible;
        }

        style: TabViewStyle {

            tabOverlap: -5
            frameOverlap: -15

            tab: Rectangle {

                radius: 4
                border.color: styleData.selected ? activePalette.highlight : activePalette.dark
                border.width: 2
                color: styleData.selected ? activePalette.highlight : activePalette.button
                scale: control.tabPosition === Qt.TopEdge ? 1 : -1

                property int totalOverlap: tabOverlap * (control.count - 1)
                property real maxTabWidth: control.count > 0 ? (styleData.availableWidth + totalOverlap) / control.count : 0

                implicitWidth: Math.round(Math.min(maxTabWidth, textitem.implicitWidth + closeitem.implicitWidth + tabTypeIcon.implicitWidth + 20))
                implicitHeight: Math.round(closeitem.implicitHeight * 1.8)


                Text {
                    id: closeitem
                    anchors.left: parent.left
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter

                    text: "  \u2716"
                    font.bold: true
                    renderType: Settings.isMobile ? Text.QtRendering : Text.NativeRendering
                    scale: control.tabPosition === Qt.TopEdge ? 1 : -1
                    color: styleData.selected ? "white" : activePalette.text

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            if (!dataTabsView.getTab(styleData.index).item.busy) {
                                dataTabsView.removeTab(styleData.index)
                            }
                        }
                    }

                }

                Rectangle {
                    id: tabTypeIconRect
                    anchors.left: closeitem.right
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    anchors.leftMargin: 5
                    color: "transparent"
                    visible: dataTabsView.tabTypeIconVisible(styleData)
                    width: visible ? tabTypeIcon.implicitWidth : 0
                    property string iconSource: dataTabsView.getTabTypeIconSource(styleData)
                    IconSVG {
                        size: parent.visible ? 14 : 0
                        id: tabTypeIcon
                        source: parent.iconSource
                    }
                    MouseArea {
                        anchors.fill: parent
                        acceptedButtons: Qt.RightButton
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            mouse.accepted = true
                            tabActions.tabIndex = styleData.index
                            tabActions.popup()
                        }
                    }

                }

                Text {
                    id: textitem
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    anchors.left: dataTabsView.tabTypeIconVisible(styleData) ? tabTypeIconRect.right : closeitem.right
                    anchors.right: parent.right
                    width: parent.width - closeitem.implicitWidth - tabTypeIconRect.implicitWidth
                    anchors.leftMargin: 2
                    anchors.rightMargin: 4
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter

                    text: styleData.title
                    elide: Text.ElideMiddle
                    renderType: Settings.isMobile ? Text.QtRendering : Text.NativeRendering
                    scale: control.tabPosition === Qt.TopEdge ? 1 : -1
                    color: styleData.selected ? "white" : activePalette.windowText

                    MouseArea {
                        anchors.fill: parent
                        acceptedButtons: Qt.RightButton
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            mouse.accepted = true
                            tabActions.tabIndex = styleData.index
                            tabActions.popup()
                        }
                    }

                }

            }

            frame: Rectangle {
                color: activePalette.window
            }

        }




    }

    function activateKey(index, newTab) {
        // Must be a string!!
        var keyFullPath = '' + mainTreeModel.roleValue(index, "fullpath")
        var keyName     = '' + mainTreeModel.roleValue(index, "name")
        var connection = mainTreeModel.roleValue(mainTreeModel.connectionParent(index), "connectionpointer")
        var dbNumber   = mainTreeModel.roleValue(mainTreeModel.databaseParent(index), "dbnumber")

        if (!backend.keyExists(connection, dbNumber, keyFullPath)) {
            yesno.text = "Key does not exist or expired. Reload database keys?"
            yesno.yesCallback = function() {
                backend.reloadKeys(mainTreeModel.databaseParent(index))
            }
            yesno.open();
            return
        }

        // Apply logic for "Edit Key in a common Tab | Edit key in NEW tab"
        var tabIdentifier
        var tabIndex

        // Look for "common" tab and "already available tab for this Key"
        var tabIdentifierCommon = "keycommontab"
        var tabIdentifierKey    = mainTreeModel.connectionParent(index).row + "-" + mainTreeModel.databaseParent(index).row + "-" + keyFullPath

        var tabInfoCommon = mainDataArea.getTabByIdentifier(tabIdentifierCommon)
        var tabInfoKey    = mainDataArea.getTabByIdentifier(tabIdentifierKey)
        var tab

        // If KEY tab is found and NO NEW tab is requested
        if (tabInfoKey && !tabInfoKey.tab.item.busy && !newTab) {
            tab = tabInfoKey.tab
            tabIndex = tabInfoKey.index
            tabIdentifier = tabIdentifierKey
        }
        // OR If common tab is found and NO NEW tab is requested
        else if (tabInfoCommon && !tabInfoCommon.tab.item.busy && !newTab) {
            tab = tabInfoCommon.tab
            tabIndex = tabInfoCommon.index
            tabIdentifier = tabIdentifierCommon
        }
        // OR NO currently open tab is FOUND OR New tab is requested
        else {
            dataTabsView.addTab(keyFullPath);
            tabIndex = dataTabsView.count-1
            if (!newTab) {
                tabIdentifier = tabIdentifierCommon
            }
            else {
                tabIdentifier = tabIdentifierKey
            }
            tab = dataTabsView.getTab(tabIndex)


        }

        tab.title = keyName

        // This is very important!
        tab.asynchronous = true;
        tab.setSource("KeyEditorWrapper.qml", {
            "keyModelIndex" : index,
            "tabIdentifier" : tabIdentifier,
            "keyGlobalFullPath": tabIdentifierKey
        })


        // Focus on new tab and unlock TabView
        dataTabsView.currentIndex = tabIndex

    }

    Connections {

        target: mainTreeView

        onRedisKeyActivated: {
            activateKey(index, newTab)
        }

        onServerStatsActivated: {

            var tabIdentifier
            var tabIndex
            var tab

            var tabIdentifierServer    = "server-connection-" + index.row
            var tabInfoServer = mainDataArea.getTabByIdentifier(tabIdentifierServer)
            var title = mainTreeModel.roleValue(index, "name")

            if (tabInfoServer) {
                tab = tabInfoServer.tab
                tabIndex = tabInfoServer.index
                tabIdentifier = tabIdentifierServer
            }
            else {
                dataTabsView.addTab(title);
                tabIndex = dataTabsView.count-1
                tabIdentifier = tabIdentifierServer
                tab = dataTabsView.getTab(tabIndex)
            }

            tab.title = title

            // This is very important!
            tab.asynchronous = true;
            tab.setSource("ServerInfoWrapper.qml", {
                "tabType"           : "serverinfo",
                "tabIdentifier"     : tabIdentifier,
                "connectionModelIndex": index
            })


            dataTabsView.currentIndex = tabIndex

            tab.loaded.connect(function() {
                tab.item.onLoaded();
            });



        }

        onTerminalActivated: {

            var tabIdentifier
            var tabIndex
            var tab

            var tabIdentifierTerminal    = "server-terminal-" + index.row + "-" + JS.getRandomInt(1000)

            var title = mainTreeModel.roleValue(index, "name")

            dataTabsView.addTab(title);
            tabIndex = dataTabsView.count-1
            tabIdentifier = tabIdentifierTerminal
            tab = dataTabsView.getTab(tabIndex)

            tab.title = title

            // This is very important!
            tab.asynchronous = true;
            tab.setSource("../terminal/TerminalWrapper.qml", {
                "tabType"           : "terminal",
                "tabIdentifier"     : tabIdentifier,
                "connectionModelIndex": index,
                "dbNumber"           : dbNumber
            })

            dataTabsView.currentIndex = tabIndex

            tab.loaded.connect(function() {
                tab.item.onLoaded();
            });



        }

    }


}


