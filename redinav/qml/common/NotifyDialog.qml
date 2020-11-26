import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 1.4
import QtQuick.Dialogs 1.2
import QtQuick.Controls.Styles 1.4

MessageDialog {
    id: notifyDialog
    visible: false
    standardButtons: StandardButton.Ok

    function showError(msg, title) {
        if (title)
            this.title = title

        icon = StandardIcon.Warning
        text = msg
        open()
    }

    function showMsg(msg, title) {
        if (title)
            this.title = title

        icon = StandardIcon.Information
        text = msg
        open()
    }
}
