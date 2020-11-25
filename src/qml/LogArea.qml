import QtQuick 2.9
import QtQuick.Controls 1.4


TextArea {
    id: root

    function clear() {
        text = ""
    }

    wrapMode: TextEdit.Wrap
    textFormat: TextEdit.PlainText
    backgroundVisible: false
}
