import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 1.4
import QtQuick.Dialogs 1.2

RowLayout {
    id: root

    property alias placeholderText: textField.placeholderText
    property alias path: textField.text
    property alias nameFilters: fileDialog.nameFilters
    property alias title: fileDialog.title
    property alias style: textField.style

    TextField {
        id: textField
        Layout.fillWidth: true
    }

    Button {
        text: "..."
        onClicked: fileDialog.open()
    }

    FileDialog {
        id: fileDialog
        folder: shortcuts.documents
        selectExisting: true
        onAccepted: textField.text = backend.getPathFromUrl(fileDialog.fileUrl)
    }
}
