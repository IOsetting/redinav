import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 1.4

RowLayout {
    id: root

    property alias placeholderText: textField.placeholderText
    property alias text: textField.text
    property alias style: textField.style

    TextField {
        id: textField
        Layout.fillWidth: true
        echoMode: passwordMask.checked ? TextInput.Normal : TextInput.Password
    }

    CheckBox {
        id: passwordMask
        text: qsTr("Show password")
    }
}
