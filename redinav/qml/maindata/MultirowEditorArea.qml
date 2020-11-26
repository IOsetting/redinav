import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 1.4
import QtQuick.Dialogs 1.2
import QtQuick.Controls.Styles 1.4
import org.redinav.qml 1.0
import "."
import "../common"
import "../common/helper.js" as JS
import "../common/json-tools.js" as Json

Rectangle {

    id: keyRoot
    width: 1000
    height: 600
    color: "transparent"
    border.width: 0


    // hash, set, zset, list (set upon QML loading)
    property string keyType: ""

    // The model index in the TreeView  (set upon QML loading)
    property var    keyModelIndex: null

    // C++ class (QT Object), providing key type specific operations  (set upon QML loading)
    property var    keyEd: null


    property int tableMinHeight: 170
    property int paginationMinHeight: 40
    property int v1MinHeight: 100
    property int v2MinHeight: 100
    property int zsetScoreMinHeight: 35
    property int saveButtonMinHeight: 50
    property int operationsWidth: 180

    property var connSettings: backend.getConnectionConfigByIndex(mainTreeModel.connectionParent(keyModelIndex))
    property bool readOnly: keyRoot.connSettings.readOnly

    // Hold what is currently shown in the key member/rows table
    property var currentRowKey: null
    property var currentRowValue: null

    // After some operations we must position the selection on some member/row in the table, based on field name or member content
    property var v1ToFind: null
    property var v2ToFind: null

    /**
      * Find a table row model (JS object key/value pair) by key/value content, incl. undefined etc.
      */
    function findPairIndex(key, val) {
        var rowIndex = -1;
        var result = table.model.filter(function( obj, index ) {
            // HASHes
            if (key !== false && val !== false) {
                if (obj.key === key && obj.value === val) {
                    rowIndex = index;
                }
            }
            // Not really happens but here for the sake of compleatenes
            else if (key !== false) {
                if (obj.key === key) {
                    rowIndex = index;
                }
            }
            // LIST, ZSET, SET
            else if (val !== false) {
                if (obj.value === val) {
                    rowIndex = index;
                }
            }
        });
        return rowIndex;
    }

    /**
      * Return QML URL for given key type
      *
      */
    function getKeyEditorByType(keyType) {

        switch (keyType) {
        case "hash":
            return "qrc:/qml/maindata/HashKey.qml"
        case "list":
            return "qrc:/qml/maindata/ListKey.qml"
        case "set":
            return "qrc:/qml/maindata/SetKey.qml"
        case "zset":
            return "qrc:/qml/maindata/ZsetKey.qml"
        }

    }

    /**
      * Calculate page count based on item count and page size
      *
      */
    function getPageCount(itemCount, pageSize) {
        return parseInt(((itemCount + pageSize - 1)/pageSize))
    }


    /**
      * Reload Key data
      *
      */
    function reloadData(key, page, search) {
        kedRoot.busy = true;
        key.loadRawKeyData(page, search);
        keyEd.loadKeyTtl()
    }

    /**
      * Delete list/set/zset key members or Hash field/value pair
      *
      */
    function deleteSelectedRows() {
        var rowNumbers = []
        var rowKeys = []
        var rowValues = []
        table.selection.forEach( function(rowIndex) {
            rowKeys.push(table.model[rowIndex]["key"])
            rowValues.push(table.model[rowIndex]["value"])
            rowNumbers.push(rowIndex)
        })
        if (rowNumbers.length > 0) {
            key.removeRows(rowNumbers, rowKeys, rowValues)
            if (keyType === "list") {
                reloadData(key, key.currentPage, searchField.text)
            }
        }
    }


    function saveKeyToRedis() {

        yesno.text = qsTr("Save key to Redis?")
        yesno.yesCallback = function() {
            var v1Val = v1.getText()
            var v2Val = (keyType === "zset" ? zsetScore.text : v2.getText())
            var message = key.updateKey(v1Val, v2Val, currentRowKey, currentRowValue);
            if (message) {
                yesno.close()
                notifyDialog.showError(message)
            }
            else {
                // Tell the callback (see at the top) to find this row after data is reloaded
                v1ToFind = v1Val
                v2ToFind = v2Val
                reloadData(key, key.currentPage, searchField.text)
            }
        }
        yesno.open()
    }

    signal keyDataLoaded();

    // Load Key type specific QML
    Loader {
        id: keyBackend
        source: ""
        onLoaded: {
            key = keyBackend.item
            reloadData(key, 1)
        }
    }

    // Listen ....
    Connections {
        target: key
        onListDataChanged: {
            kedRoot.busy = false;
            keyDataLoaded()
        }
    }

    // Listen on "keyDataLoaded" event, fired by backend
    onKeyDataLoaded: {
        // If there is specific ROW to find and position onto (like after Update row)
        if (v1ToFind || v2ToFind) {
            var i = (keyType === "hash") ? findPairIndex(v1ToFind, v2ToFind) : findPairIndex(false, v2ToFind)
            i = (i !== -1) ? i : 0
            table.selection.clear()
            table.selection.select(i)
            table.currentRow = i
            table.selection.selectionChanged()
            table.positionViewAtRow(i, ListView.Center)
            v1ToFind = null
            v2ToFind = null
        }
        else if (table.currentRow !== -1) {
            table.selection.select(table.currentRow)
        }
    }

    Component.onCompleted: {
        kedRoot.busy = true;
        // Set Loader's source, triggering its actual loading
        keyBackend.setSource(getKeyEditorByType(keyType), {});
    }

    // Define shortcuts
    Shortcut {
        enabled: !keyRoot.readOnly && (table.activeFocus && key.rowsCount && table.currentRow !== -1)
        sequences: JS.shortcuts("deletekey")
        onActivated: {
            deleteRowConfirmation.open()
        }
    }



    // OUTERMOST ROW
    RowLayout {
        id: outerRow
        anchors.fill: parent
        spacing: 3

        // DATA
        Rectangle {
            id: dataRectangle
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignLeft
            color: "transparent"
            border.width: 0

            ColumnLayout  {
                id: dataColumn
                anchors.fill: parent
                spacing: 3

                // KEY DATA
                SplitView {
                    id: dataSplitView
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    orientation: Qt.Vertical

                    Connections {
                        target: table.selection
                        onSelectionChanged: {

                            var hasRow   = (typeof table.model[table.currentRow] !== "undefined") && (table.model[table.currentRow] !== null)
                            if (!hasRow) {
                                return;
                            }

                            currentRowKey     = table.model[table.currentRow]["key"]
                            currentRowValue   = table.model[table.currentRow]["value"]

                            if (typeof currentRowKey !== "undefined") {
                                v1.setText(currentRowKey)
                            }
                            if (typeof currentRowValue !== "undefined") {
                                v2.setText(currentRowValue)
                            }
                            zsetScore.text = currentRowValue ? parseFloat(Number(currentRowValue)) : ""
                        }
                    }

                    // TABLE and Pagination
                    Rectangle {
                        id: tableAndPagination
                        color: "transparent"
                        border.width: 0
                        Layout.fillHeight: true
                        Layout.minimumHeight: { return tableMinHeight + paginationMinHeight; }

                        ColumnLayout {
                            anchors.fill: parent
                            spacing: 0

                            // TABLE
                            Rectangle {
                                id: rowKeyTable
                                color: "transparent"
                                border.width: 0
                                Layout.fillHeight: true
                                Layout.fillWidth: true
                                Layout.minimumHeight: tableMinHeight


                                // Key table itself
                                TableView {
                                    id: table
                                    anchors.fill: parent
                                    selectionMode: SelectionMode.ExtendedSelection
                                    model: key.listData

                                    property int pageCount: getPageCount(key.rowsCount, key.pageSize)
                                    property bool twoValues: (["hash", "zset", "list"].indexOf(keyType) !== -1)
                                    property int rowsCount: key.rowsCount
                                    property int currentStart: (key.currentPage-1) * key.pageSize

                                    sortIndicatorVisible: (keyType == "hash") || (keyType == "set") || (keyType == "zset")

                                    onSortIndicatorColumnChanged: {
                                        key.buildListData(sortIndicatorColumn, sortIndicatorOrder);
                                    }
                                    onSortIndicatorOrderChanged: {
                                        key.buildListData(sortIndicatorColumn, sortIndicatorOrder);
                                    }

                                    TableViewColumn {
                                        visible: table.twoValues
                                        title: {
                                            if (keyType === "list")
                                                return "Index"
                                            else if (keyType === "hash")
                                                return "Key"
                                            else if (keyType === "zset")
                                                return "Member"

                                            return "";
                                        }
                                        role: "key"
                                        width: table.twoValues ? 350 : 0
                                    }

                                    TableViewColumn{
                                        title: keyType === "zset" ? "Score" : "Value"
                                        role: "value"
                                        width: table.twoValues ? table.width - 370 : table.width - 20
                                    }


                                    itemDelegate: Item {
                                        Text {
                                            anchors.fill: parent
                                            color: styleData.textColor
                                            elide: Text.ElideRight
                                            text: {
                                                if (!table.twoValues && styleData.column === 0) {
                                                    return "";
                                                }
                                                if (styleData.column === 1 && keyType === "zset") {
                                                    return styleData.value !== null ? parseFloat(Number(styleData.value).toFixed(7)) : "";
                                                }
                                                return styleData.value !== null ? styleData.value : "";
                                            }
                                            wrapMode: Text.WrapAnywhere
                                            maximumLineCount: 1
                                        }

                                    }

                                }


                            }

                            // Pagination
                            Rectangle {
                                id: rowPagination
                                color: "transparent"
                                Layout.minimumHeight: paginationMinHeight
                                Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom
                                Layout.fillHeight: false
                                Layout.fillWidth: true
                                border.width: 0

                                // Stats and pagination below the table
                                RowLayout {
                                    anchors.fill: parent
                                    Text {
                                        color: activePalette.text
                                        text: {
                                            if (searchField.text === "")
                                                return "Total Rows: " + key.rowsCount
                                            else if ((searchField.text !== "") && (key.rowsCount >= key.pageSize))
                                                return "Results: " + key.rowsCount + " (possibly partial, please narrow down the search)"
                                            else if (searchField.text !== "")
                                                return "Results: " + key.rowsCount
                                            else
                                                return ""
                                        }
                                    }

                                    Pagination {
                                        visible: key.isPaginated && table.pageCount > 1
                                        id: pagination
                                        Layout.fillWidth: false

                                        onPageChanged: {
                                            reloadData(key, page)
                                        }

                                    }

                                }
                            }


                        }

                    }

                    // V1 + [zsetScore]
                    // v1= hash key || zset member
                    Rectangle {
                        id: rowV1
                        visible: (keyType !== "list") && (keyType !== "set")
                        color: "transparent"
                        border.width: 0

                        Layout.minimumHeight: {
                            if (keyType === "zset")
                                return v1MinHeight + zsetScoreMinHeight;
                            else
                                return v1MinHeight;
                        }


                        ColumnLayout {
                            anchors.fill: parent
                            spacing: 0

                            // V1
                            Rectangle {
                                id: v1Rectangle
                                color: "transparent"
                                Layout.minimumHeight: v1MinHeight
                                Layout.fillHeight: true
                                Layout.fillWidth: true
                                border.width: 0

                                TextEditor {
                                    id: v1
                                    showSize: true
                                    label: keyType === "hash" ? "Field" : ""
                                    formattersButtons: {
                                        return ["phpunserialize", "json-pretty", "minify", "original"]
                                    }
                                    topSpacer: true
                                    bottomSpacer: true
                                }

                            }

                            // Score
                            Rectangle {
                                id: zetScoreRectangle
                                visible: keyType === "zset"
                                color: "transparent"
                                Layout.minimumHeight: zsetScoreMinHeight
                                Layout.maximumHeight: zsetScoreMinHeight
                                Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom
                                Layout.fillHeight: true
                                Layout.fillWidth: true
                                border.width: 0

                                RowLayout {
                                    anchors.right: parent.right
                                    anchors.left: parent.left
                                    spacing: 5

                                    Text {
                                        color: activePalette.text
                                        text: "Score"
                                    }
                                    TextField {
                                        id: zsetScore
                                        text: ""
                                        Layout.fillWidth: true
                                        validator: DoubleValidator {
                                            locale: "C"
                                            decimals: 7
                                            notation: DoubleValidator.StandardNotation
                                        }
                                    }

                                }


                            }
                        }


                    }

                    // V2
                    // v2 = hash value || set member || list member
                    Rectangle {
                        visible: keyType !== "zset"
                        id: rowV2
                        color: "transparent"
                        border.width: 0
                        Layout.minimumHeight: v2MinHeight

                        TextEditor {
                            id: v2
                            showSize: true
                            label: keyType === "hash" ? "Value" : ""
                            formattersButtons: {
                                return ["phpunserialize", "json-pretty", "minify", "original"]
                            }
                            topSpacer: true
                            bottomSpacer: false
                        }


                    }

                }

                // SAVE TO REDIS
                Rectangle {
                    id: saveToRedisRectangle
                    color: "transparent"
                    Layout.minimumHeight: saveButtonMinHeight
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom
                    Layout.fillHeight: false
                    Layout.fillWidth: true
                    border.width: 0

                    Button {
                        id: editorSaveButton
                        anchors.right: parent.right
                        anchors.bottom: parent.bottom
                        iconSource: qsTr("qrc:/resources/images/icons/themes/%1/ol_save.svg").arg(approot.theme)
                        text: " " + qsTr("Save key to Redis")
                        enabled: !readOnly
                        onClicked: {
                            saveKeyToRedis();
                        }
                    }

                }

            }

        }

        // OPERATIONS
        Rectangle {
            id: operationsRectangle
            width: operationsWidth
            color: "transparent"
            border.width: 0
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter

            ColumnLayout {
                width: 0
                anchors.left: parent.left
                anchors.leftMargin: 0
                anchors.right: parent.right
                anchors.rightMargin: 0
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop

                RowLayout {
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

                    TextField {
                        id: searchField
                        Layout.fillWidth: true
                        placeholderText: {
                            return qsTr("Search")
                        }
                        Keys.onReturnPressed: {
                            reloadData(key, 1, searchField.text)
                        }
                    }

                    Button {
                        Layout.maximumWidth: 35
                        text: "\u2716"
                        onClicked: {
                            searchField.text = "";
                            reloadData(key, key.currentPage, searchField.text)
                        }
                    }

                }

                Button {
                    Layout.preferredWidth: 195
                    text: " " + qsTr("Reload/Search")
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    Layout.fillWidth: true
                    iconSource: qsTr("qrc:/resources/images/icons/themes/%1/ol_refresh.svg").arg(approot.theme)
                    action: reLoadAction

                    Action {
                        id: reLoadAction
                        onTriggered: {
                            reloadData(key, 1, searchField.text)
                        }
                    }
                }

                Button {
                    enabled: !readOnly
                    Layout.preferredWidth: 195
                    text: " Add " + JS.keyTypeMemberTitle(keyType)
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    Layout.fillWidth: true
                    iconSource: qsTr("qrc:/resources/images/icons/themes/%1/ol_add.svg").arg(approot.theme)
                    onClicked: {
                        addRowDialog.setProps(key, kedRoot.keyType)
                        addRowDialog.open()
                    }

                    AddRowDialog {
                        id: addRowDialog
                        visible: false
                    }


                }
                Button {
                    Layout.preferredWidth: 195
                    text: " Delete " + JS.keyTypeMemberTitle(keyType) + "(s)"
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    Layout.fillWidth: true
                    iconSource: qsTr("qrc:/resources/images/icons/themes/%1/ol_delete.svg").arg(approot.theme)
                    enabled: {
                        return !keyRoot.readOnly && (key.rowsCount && table.currentRow != -1)
                    }

                    onClicked: {
                        if (table.selection.count > 0)
                            deleteRowConfirmation.open()
                    }

                    MessageDialog {
                        id: deleteRowConfirmation
                        title: " Delete " + JS.keyTypeMemberTitle(keyType) + "(s)"
                        text: {
                            return qsTr("Do you really want to delete %1 %2(s)?").arg(table.selection.count).arg(JS.keyTypeMemberTitle(keyType))
                        }
                        onYes: {
                            deleteSelectedRows()
                        }
                        visible: false
                        icon: StandardIcon.Warning
                        standardButtons: StandardButton.Yes | StandardButton.No
                    }
                }
            }


        }




    }



}


/*##^## Designer {
    D{i:9;anchors_height:100;anchors_width:100;anchors_x:285;anchors_y:288}
}
 ##^##*/
