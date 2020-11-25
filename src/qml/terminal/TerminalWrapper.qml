import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Window 2.3
import org.redinav.qml 1.0
import Qt.labs.settings 1.0
import "../common"
import "../common/helper.js" as JS


Item {

    id: root
    property string tabType: "terminal"
    property string tabIdentifier: ""
    property var    connectionModelIndex: null
    property int dbNumber: 0
    property var    connection: mainTreeModel.roleValue(connectionModelIndex, "connectionpointer")
    property bool busy: terminal.busy
    property int historyIndexPointer: 0
    property int currentCommandMode: 1;   // 1: Single command,  2: Multi command   3: LUA

    property int singleCommandMode: 1
    property int multiCommandMode: 2
    property int luaScriptMode: 3


    function onLoaded() {
        terminal.connect()
    }


    function clearCurrentArea() {
        switch (currentCommandMode) {
            case singleCommandMode:
                commandText.text = ""
                return;
            case multiCommandMode:
                commandTextArea.text = "";
                return;
            case luaScriptMode:
                luaTextArea.text = ""
                luaParamsField.text = ""
                return
        }
    }

    function modeName(mode) {
        switch (mode) {
            case singleCommandMode:
                return "Single";
            case multiCommandMode:
                return "Multi";
            case luaScriptMode:
                return "Lua"
        }

        return "?";
    }

    function modeHint(mode) {
        switch (mode) {
            case singleCommandMode:
                return "ENTER=execute, UP/DOWN = history";
            case multiCommandMode:
                return "SHIFT+ENTER=execute";
            case luaScriptMode:
                return "SHIFT+ENTER=execute"
        }

        return "";
    }

    function executeCommandsRequested(content, skipHistory, clear) {
        if (clear) {
            commandText.text = "";
        }
        commandText.text = "";
        if (content.trim().length > 0) {
            root.busy = true
            terminal.execCmdText(content, skipHistory);
            root.busy = false
        }
    }

    function executeLuaScriptRequested(content, params) {
        terminal.execLua(content, params)
    }


    anchors.fill: parent;

    // Qt Object
    Terminal {
        id: terminal
        db: root.dbNumber
        connection: root.connection
        backend: backend
    }

    Connections {
        target: terminal

        onHistoryChanged: {
            root.historyIndexPointer = terminal.history.length;
        }

        onConnectionFailed: {
            var tab = mainDataArea.getTabByIdentifier(root.tabIdentifier)
            dataTabsView.removeTab(tab.index)
        }

    }

    Connections {
        target: commandText
        onHistoryIndexPointerChanged: {
            if (root.historyIndexPointer > terminal.history.length - 1) {
                commandText.text = ""
            }
            else {
                commandText.text = terminal.history[root.historyIndexPointer]
            }
        }
    }


    Rectangle {
        id: wrapper
        anchors.fill: parent;
        color: activePalette.window

        ColumnLayout {
            anchors.fill: parent;

            Rectangle {
                Layout.margins: 2
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: activePalette.window

                ColumnLayout {
                    anchors.fill: parent;


                    RowLayout {
                        Layout.fillWidth: true

                        Rectangle {
                            visible: terminal.connectionMode === 2
                            Layout.preferredHeight: clusterIcon.implicitHeight
                            Layout.preferredWidth: clusterIcon.implicitWidth
                            color: "transparent"

                            IconSVG {
                                id: clusterIcon
                                source: qsTr("qrc:/resources/images/icons/themes/%1/ol_cluster.svg").arg(approot.theme)
                            }
                        }

                        Text {
                            color: activePalette.windowText
                            text: {
                                var prefix = ""
                                if (terminal.config) {
                                    return prefix + terminal.config.name + " (" +  terminal.config.original_host + ":" + terminal.config.original_port + ")"
                                }
                                else {
                                    return ""
                                }
                            }
                        }
                    }

                    RowLayout {
                        Layout.topMargin: 5
                        Layout.fillWidth: true
                        Layout.leftMargin: 5

                        Label {
                            visible: terminal.connectionMode === 2
                            text: "Nodes (" + terminal.masterNodes.length + ") "
                        }

                        ComboBox {
                            id: nodesList
                            property int modelWidth
                            Layout.minimumWidth: modelWidth
                            visible: terminal.connectionMode === 2 && terminal.masterNodes.length > 0
                            currentIndex: 0

                            model: {
                                var list = ["Default"];
                                for (var i = 0; i < terminal.masterNodes.length; i++) {
                                    list.push(terminal.masterNodes[i].host + ":" + terminal.masterNodes[i].port)
                                }
                                return list;
                            }
                            onCurrentIndexChanged: {
                                terminal.reConnectTo(currentIndex)
                            }

                            onModelChanged: {
                                t_metrics.font.pointSize = 12
                                for(var i = 0; i < nodesList.model.length; i++){
                                    t_metrics.text = "-" + nodesList.model[i] + "-"
                                    nodesList.modelWidth = Math.max(t_metrics.tightBoundingRect.width, nodesList.modelWidth)
                                }
                            }

                            TextMetrics {
                                elide   : Qt.ElideNone
                                id      : t_metrics
                                text    : ""
                                font    : modeHintText.font
                            }
                        }


                        Label {
                            text: "DB# " + terminal.db
                            font.bold: true
                            Layout.rightMargin: 20
                        }

                        Button {
                            id: execButton
                            Layout.maximumWidth: 40
                            text: "\u25B6"
                            tooltip: "Execute command(s)"
                            onClicked: {
                                if (currentCommandMode === singleCommandMode) {
                                    executeCommandsRequested(commandText.text, false, true)
                                }
                                else if (currentCommandMode === multiCommandMode) {
                                    executeCommandsRequested(commandTextArea.text, true, false)
                                }
                                else if (currentCommandMode === luaScriptMode) {
                                    root.busy = true
                                    executeLuaScriptRequested(luaTextArea.text, luaParamsField.text)
                                    root.busy = false
                                }
                                else {
                                    console.log("Unknown mode")
                                }
                            }
                        }

                        Button {
                            id: clearButton
                            Layout.maximumWidth: 40
                            text: "\u00D7"
                            tooltip: "Clear command area"
                            onClicked: {
                                clearCurrentArea()
                            }
                        }



                        ExclusiveGroup { id: commandModeGroup }
                        RadioButton {
                            text: "Single"
                            checked: true
                            exclusiveGroup: commandModeGroup
                            onClicked: {
                                currentCommandMode = singleCommandMode
                            }
                        }
                        RadioButton {
                            text: "Multi"
                            exclusiveGroup: commandModeGroup
                            onClicked: {
                                currentCommandMode = multiCommandMode
                            }
                        }
                        RadioButton {
                            text: "Lua"
                            exclusiveGroup: commandModeGroup
                            onClicked: {
                                currentCommandMode = luaScriptMode
                            }
                        }

                        Item {Layout.fillWidth: true;}

                        Text {
                            id: modeHintText
                            Layout.rightMargin: 5
                            Layout.alignment: Qt.AlignRight
                            horizontalAlignment: Text.AlignRight
                            text: modeHint(currentCommandMode)
                            color: activePalette.windowText
                        }



                    }


                    SplitView {

                        orientation: Qt.Horizontal
                        Layout.fillHeight: true
                        Layout.fillWidth: true


                        ColumnLayout {

                            RowLayout {
                                Layout.fillWidth: true
                                Layout.topMargin: 2

                                // Command text field and area
                                Rectangle {
                                    id: rectangle
                                    Layout.minimumHeight: (currentCommandMode === singleCommandMode ? 30 : (currentCommandMode === multiCommandMode ? 200 : 200))
                                    Layout.fillHeight: false
                                    Layout.fillWidth: true
                                    Layout.rightMargin: 5
                                    color: "transparent"

                                    RowLayout {
                                        anchors.fill: parent

                                        // SINGLE
                                        ExTextField {
                                            id: commandText
                                            radius: 0
                                            visible: currentCommandMode === singleCommandMode
                                            Layout.fillWidth: true
                                            Layout.alignment: Qt.AlignTop

                                            signal historyIndexPointerChanged

                                            Keys.onPressed: {
                                                if ((event.key === Qt.Key_Enter) || (event.key === Qt.Key_Return)) {
                                                    event.accepted = true
                                                    executeCommandsRequested(commandText.text, false, true)
                                                }
                                                else if (event.key === Qt.Key_Up) {
                                                    root.historyIndexPointer--;
                                                    root.historyIndexPointer = Math.max(0, root.historyIndexPointer)
                                                    historyIndexPointerChanged()
                                                    event.accepted = true
                                                }
                                                else if (event.key === Qt.Key_Down) {
                                                    root.historyIndexPointer++;
                                                    root.historyIndexPointer = Math.min(terminal.history.length, root.historyIndexPointer)
                                                    historyIndexPointerChanged()
                                                    event.accepted = true
                                                }
                                            }
                                        }

                                        // MULTI
                                        Rectangle {
                                            visible: currentCommandMode === multiCommandMode
                                            Layout.fillWidth: true
                                            Layout.fillHeight: true
                                            //color: activePalette.alternateBase
                                            color: "transparent"

                                            ColumnLayout {
                                                anchors.fill: parent

                                                Rectangle {
                                                    Layout.fillWidth: true
                                                    Layout.fillHeight: true
                                                    color: activePalette.alternateBase

                                                    TextArea {
                                                        anchors.fill: parent
                                                        id: commandTextArea
                                                        backgroundVisible: false
                                                        Keys.onPressed: {
                                                            if ( (event.modifiers & Qt.ShiftModifier) && ((event.key === Qt.Key_Enter) || (event.key === Qt.Key_Return))) {
                                                                event.accepted = true
                                                                executeCommandsRequested(commandTextArea.text, true, false)
                                                            }
                                                        }
                                                    }
                                                }

                                                RowLayout {
                                                    Button {
                                                        text: "Load from file"
                                                        onClicked: {
                                                            fileSelector.title = qsTr("Select file")
                                                            fileSelector.nameFilters = ["*.redis", "*.*"]
                                                            fileSelector.selectExisting = true
                                                            fileSelector.callback = function(fileUrl) {
                                                                commandTextArea.text = terminal.loadFile(backend.getPathFromUrl(fileUrl))
                                                            }
                                                            fileSelector.open()
                                                        }
                                                    }

                                                    Button {
                                                        text: "Execute commands from file"
                                                        onClicked: {
                                                            fileSelector.title = qsTr("Select file")
                                                            fileSelector.nameFilters = ["*.redis", "*.*"]
                                                            fileSelector.selectExisting = true
                                                            fileSelector.callback = function(fileUrl) {
                                                                root.busy = true
                                                                terminal.execCommandsFile(backend.getPathFromUrl(fileUrl))
                                                                root.busy = false
                                                            }
                                                            fileSelector.open()
                                                        }
                                                    }
                                                }


                                            }


                                        }

                                        // LUA
                                        Rectangle {
                                            visible: currentCommandMode === luaScriptMode
                                            Layout.fillWidth: true
                                            Layout.fillHeight: true
                                            color: "transparent"

                                            ColumnLayout {
                                                anchors.fill: parent
                                                Rectangle {
                                                    Layout.fillWidth: true
                                                    Layout.fillHeight: true
                                                    color: activePalette.alternateBase
                                                    TextArea {
                                                        anchors.fill: parent
                                                        id: luaTextArea
                                                        backgroundVisible: false
                                                        Keys.onPressed: {
                                                            if ( (event.modifiers & Qt.ShiftModifier) && ((event.key === Qt.Key_Enter) || (event.key === Qt.Key_Return))) {
                                                                event.accepted = true
                                                                root.busy = true
                                                                executeLuaScriptRequested(luaTextArea.text, luaParamsField.text)
                                                                root.busy = false
                                                            }
                                                        }
                                                    }
                                                }

                                                RowLayout {

                                                    Button {
                                                        text: "Load from file"
                                                        onClicked: {
                                                            fileSelector.title = qsTr("Select file")
                                                            fileSelector.nameFilters = ["*.lua", "*.*"]
                                                            fileSelector.selectExisting = true
                                                            fileSelector.callback = function(fileUrl) {
                                                                luaTextArea.text = terminal.loadFile(backend.getPathFromUrl(fileUrl))
                                                            }
                                                            fileSelector.open()
                                                        }
                                                    }

                                                    Button {
                                                        text: "Execute Lua file"
                                                        onClicked: {
                                                            fileSelector.title = qsTr("Select file")
                                                            fileSelector.nameFilters = ["*.lua", "*.*"]
                                                            fileSelector.selectExisting = true
                                                            fileSelector.callback = function(fileUrl) {
                                                                root.busy = true
                                                                terminal.execLuaFile(backend.getPathFromUrl(fileUrl), luaParamsField.text)
                                                                root.busy = false
                                                            }
                                                            fileSelector.open()
                                                        }
                                                    }


                                                    Label {
                                                        text: "Lua Parameters"
                                                    }

                                                    ExTextField {
                                                        radius: 0
                                                        id: luaParamsField
                                                        Layout.fillWidth: true
                                                        text: ""
                                                        Keys.onPressed: {
                                                            if ( (event.modifiers & Qt.ShiftModifier) && ((event.key === Qt.Key_Enter) || (event.key === Qt.Key_Return))) {
                                                                event.accepted = true
                                                                executeLuaScriptRequested(luaTextArea.text, luaParamsField.text)
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
                                Layout.topMargin: 0
                                Layout.rightMargin: 5
                                Layout.bottomMargin: 5
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                color: activePalette.alternateBase

                                TextArea {
                                    id: outputArea
                                    Connections {
                                        target: terminal
                                        onResponseTaken: {
                                            outputArea.append(resp + "\n");
                                        }

                                    }
                                    readOnly: false
                                    anchors.fill: parent
                                    backgroundVisible: false
                                }

                            }

                        }
                    }


                    Rectangle {
                        id: rectangle1
                        Layout.rightMargin: 5
                        Layout.fillWidth: true
                        Layout.minimumHeight: 35
                        color: "transparent"

                        Button {
                            iconSource: qsTr("qrc:/resources/images/icons/themes/%1/ol_save.svg").arg(approot.theme)
                            id: saveButton
                            text: " " + qsTr("Save output to file")
                            anchors.verticalCenter: parent.verticalCenter
                            tooltip: "Save to local file"
                            onClicked: {
                                fileSelector.title = qsTr("Select file to save")
                                fileSelector.nameFilters = ["*.txt", "*.*"]
                                fileSelector.selectExisting = false
                                fileSelector.callback = function(fileUrl) {
                                    backend.saveContentToFile(backend.getPathFromUrl(fileUrl), outputArea.text)
                                }
                                fileSelector.open()
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

/*##^## Designer {
    D{i:0;autoSize:true;height:480;width:640}D{i:13;invisible:true}
}
 ##^##*/
