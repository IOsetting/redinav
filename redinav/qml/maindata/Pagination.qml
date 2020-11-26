import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 1.4

RowLayout {
    Layout.fillWidth: true
    spacing: 10

    signal pageChanged(int page)

    function pageChangeRequested(page) {
        if ((page > 0 && page <= table.pageCount) && (page !== key.currentPage)) {
            pageChanged(page)
        }
    }

    Text {
        color: activePalette.text
        text: qsTr("Page") + " "
        wrapMode: Text.WrapAnywhere
    }

    TextField {
        id: pageField;
        text: key.currentPage;
        Layout.maximumWidth: 60;
        readOnly: false
        Keys.onReturnPressed: {
            pageChangeRequested(parseInt(text))
        }
        validator: IntValidator {bottom: 1; top: table.pageCount}
    }

    Text {
        color: activePalette.text
        Layout.maximumWidth: 130
        text: " of " + table.pageCount
        wrapMode: Text.WrapAnywhere
    }

    Button {
        id: setPageButton
        Layout.maximumWidth: 100
        Layout.fillWidth: true
        text: qsTr("Set Page")
        onClicked: {
            pageChangeRequested(parseInt(pageField.text))
        }
    }

    Button {
        Layout.fillWidth: true
        text: "⇦"
        Layout.maximumWidth: 50
        onClicked: {
            pageChangeRequested(key.currentPage - 1)
        }
    }
    Button {
        Layout.fillWidth: true
        text: "⇨"
        Layout.maximumWidth: 50
        onClicked: {
            pageChangeRequested(key.currentPage + 1)
        }
    }
}
