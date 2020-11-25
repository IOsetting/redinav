import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import org.redinav.qml 1.0

Item {


    id: root
    property string tabType: "serverinfo"
    property string tabIdentifier: ""
    property var    connectionModelIndex: null
    property var    connection: mainTreeModel.roleValue(connectionModelIndex, "connectionpointer")
    property bool   busy: server ? server.busy : false

    function onLoaded() {
        server.init()
    }


    // serverinfo.cpp
    ServerInfo {
        id: server
        connection: root.connection
    }

    // Connect to Server info backend, listen for "loaded" and other signals
    Connections {
        id: connection1
        target: server

        onError: {
        }

        onInitialized: {
        }

        onConnectionFailed: {
            var tab = mainDataArea.getTabByIdentifier(root.tabIdentifier)
            dataTabsView.removeTab(tab.index)
        }

    }

    Component.onCompleted:  {
    }

    Component.onDestruction: {
    }


    Rectangle {
        id: wrapper

        color: activePalette.window
        anchors.fill: parent

        ColumnLayout {

            id: grid
            property int fontPointSize: 10
            property int fontPointSize2: 12

            anchors.fill: parent
            anchors.margins: 0

            RowLayout {
                // Freeze button and checkbox
                Button {
                    text: " " + (server.frozen ? qsTr("Unfreeze data") : qsTr("Freeze data")) + " "
                    onClicked: {
                        server.frozen = ! server.frozen;
                    }
                }

                Label {
                    text: {
                        if (server.config) {
                            return server.config.name + " (" +  server.config.original_host + ":" + server.config.original_port + ")"
                        }
                        else {
                            return ""
                        }


                    }
                }

            }


            RowLayout {
                Layout.fillWidth: true
                spacing: 30

                ColumnLayout {
                    // VERSION
                    Text {
                        text: qsTr("Redis Version")
                        font.pointSize: grid.fontPointSize
                        color: activePalette.text
                    }
                    Label { id: redisVersionLabel; text: "N/A"; font.pointSize: grid.fontPointSize2  }
                }

                ColumnLayout {
                    // Memory
                    Text {
                        text: qsTr("Used memory")
                        font.pointSize: grid.fontPointSize
                        color: activePalette.text
                    }
                    Label { id: usedMemoryLabel; text: "N/A"; font.pointSize: grid.fontPointSize2 }

                }
                ColumnLayout {
                    // CLIENTS
                    Text {
                        text: qsTr("Clients")
                        font.pointSize: grid.fontPointSize
                        color: activePalette.text
                    }
                    Label { id: connectedClientsLabel; text: "N/A"; font.pointSize: grid.fontPointSize2 }

                }
                ColumnLayout {
                    // Commands processed
                    Text {
                        text: qsTr("Commands Processed")
                        font.pointSize: grid.fontPointSize
                        color: activePalette.text
                        wrapMode: Text.WordWrap
                    }
                    Label { id: totalCommandsProcessedLabel; text: "N/A"; font.pointSize: grid.fontPointSize2 }

                }
                ColumnLayout {
                    // Uptime
                    Text {
                        text: qsTr("Uptime")
                        font.pointSize: grid.fontPointSize
                        color: activePalette.text
                    }
                    Label { id: uptimeLabel; text: "N/A"; font.pointSize: grid.fontPointSize2 }

                }


                Item {Layout.fillWidth: true}

                ColumnLayout {
                    Layout.alignment: Qt.AlignRight

                    CheckBox {
                        text: qsTr("Fetch clients list data")
                        id: cb2
                        checked: false
                        onClicked: {
                            server.includeClientsList = checked;
                        }
                    }

                    Item {Layout.fillWidth: true;}

                    Connections {
                        id: connection2
                        target: server
                        onIncludeClientsListChanged: {
                            cb2.checked = server.includeClientsList
                        }
                    }


                }

                // Data feed for top row
                Connections {
                    id: connection3
                    target: server

                    onServerDataChanged: {
                        if (server && server.serverData) {
                            redisVersionLabel.text = server.serverData["server"]["redis_version"]
                            usedMemoryLabel.text = server.serverData["memory"]["used_memory_human"]
                            connectedClientsLabel.text = server.serverData["clients"]["connected_clients"]
                            totalCommandsProcessedLabel.text = server.serverData["stats"]["total_commands_processed"]
                            uptimeLabel.text = server.serverData["server"]["uptime_in_days"] + " days"
                        }
                    }
                }


            }


            // Tabs & Tables
            RowLayout {
                Layout.fillHeight: true
                Layout.fillWidth: true

                TabView {
                    id: serverInfoTabs
                    Layout.fillHeight: true
                    Layout.fillWidth: true

                    // Clients list related
                    property var clientsListModel: []
                    property var knownClientsListProps : [
                        "id", "addr", "db", "cmd", "fd", "name", "age", "idle",
                        "flags", "sub", "psub", "multi", "qbuf",
                        "qbuf-free", "obl", "oll", "omem", "events"
                    ];


                    // All other server info groups
                    property var keyspace: []
                    property var cluster: []
                    property var ssl: []
                    property var commandstats: []
                    property var cpu: []
                    property var replication: []
                    property var stats: []
                    property var persistence: []
                    property var memory: []
                    property var server: []

                    property var tabsDescriptor: [
                        {dataGroup: "keyspace",         title: "Keyspace",              model:  serverInfoTabs.keyspace},
                        {dataGroup: "cluster",          title: "Cluster",               model:  serverInfoTabs.cluster},
                        {dataGroup: "ssl",              title: "SSL",                   model:  serverInfoTabs.ssl},
                        {dataGroup: "commandstats",     title: "Coomand Stats",         model:  serverInfoTabs.commandstats},
                        {dataGroup: "cpu",              title: "CPU",                   model:  serverInfoTabs.cpu},
                        {dataGroup: "replication",      title: "Replication",           model:  serverInfoTabs.replication},
                        {dataGroup: "stats",            title: "Stats",                 model:  serverInfoTabs.stats},
                        {dataGroup: "persistence",      title: "Persistence",           model:  serverInfoTabs.persistence},
                        {dataGroup: "memory",           title: "Memory",                model:  serverInfoTabs.memory},
                        {dataGroup: "server",           title: "Server",                model:  serverInfoTabs.server}
                    ]

                    // Repeat for all defined/described tabs and info groups
                    Repeater {
                        model: serverInfoTabs.tabsDescriptor.length
                        Tab {
                            title: {
                                return serverInfoTabs.tabsDescriptor[index].title
                            }
                            TableView {
                                TableViewColumn {
                                    role: "name"
                                    title: qsTr("Property")
                                    width: 300
                                }
                                TableViewColumn {
                                    role: "value"
                                    title: qsTr("Value")
                                    width: serverInfoTabs.width - 330
                                }
                                model: serverInfoTabs.tabsDescriptor[index].model
                            }
                        }
                    }

                    // Upon server data change - update above Tabs
                    Connections {
                        id: connection4
                        target: server
                        onServerDataChanged: {
                            var tabInfo
                            var info
                            for (var i=0; i < serverInfoTabs.tabsDescriptor.length; i++) {
                                tabInfo = serverInfoTabs.tabsDescriptor[i]
                                info = []
                                for (var key in server.serverData[tabInfo.dataGroup])
                                {
                                    info.push({"name": key, "value": server.serverData[tabInfo.dataGroup][key]})
                                }
                                // Set PROPERTY!! (triggers table view resfresh)
                                serverInfoTabs[tabInfo.dataGroup] = info
                            }
                        }
                    }

                    // One TAB only, but in a Repeater; Otherwise it always goes first in the list, visually
                    Repeater {
                        model: 1
                        // Clients list
                        Tab {
                            title: "Clients List"
                            id: clientsListTab

                            TableView {
                                Layout.fillHeight: true
                                model: server.clientsList

                                TableViewColumn { role: "id"; title: "id"; width: 100; }
                                TableViewColumn { role: "addr"; title: "addr"; }
                                TableViewColumn { role: "db"; title: "db"; width: 25; }
                                TableViewColumn { role: "cmd"; title: "cmd"; width: 100; }
                                TableViewColumn { role: "fd"; title: "fd"; width: 50; }
                                TableViewColumn { role: "name"; title: "name"; }
                                TableViewColumn { role: "age"; title: "age"; }
                                TableViewColumn { role: "idle"; title: "idle"; }
                                TableViewColumn { role: "flags"; title: "flags"; width: 50; }
                                TableViewColumn { role: "sub"; title: "sub"; width: 50; }
                                TableViewColumn { role: "psub"; title: "psub"; width: 50; }
                                TableViewColumn { role: "multi"; title: "multi"; width: 50; }
                                TableViewColumn { role: "qbuf"; title: "qbuf"; width: 50; }
                                TableViewColumn { role: "qbuf-free"; title: "qbuf-free"; width: 70; }
                                TableViewColumn { role: "obl"; title: "obl"; width: 50; }
                                TableViewColumn { role: "oll"; title: "oll"; width: 50; }
                                TableViewColumn { role: "omem"; title: "omem"; width: 50; }
                                TableViewColumn { role: "events"; title: "events"; width: 50; }

                                itemDelegate: Item {
                                    Text {
                                        text: {
                                            if (typeof styleData.value !== "string")
                                                return "";
                                            return (styleData.value !== undefined) ? styleData.value : ""
                                        }
                                        color: activePalette.text
                                    }
                                }
                            }
                        }
                    }


                }
            }
        }


    }


    Rectangle {
        id: uiBlocker
        visible: root.busy
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
