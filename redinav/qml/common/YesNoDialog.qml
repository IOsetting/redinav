import QtQuick 2.9
import QtQuick.Dialogs 1.2

MessageDialog {
    id: yesNoDialog
    property var yesCallback: null
    property var noCallback: null
    property string defaultText: qsTr("Do you really want to perform this operation?")
    title: qsTr("Confirmation")
    text: defaultText
    onYes: {
        if (yesCallback)
            yesCallback()
        yesCallback = null
        noCallback = null
    }
    onNo: {
        if (noCallback)
            noCallback()
        yesCallback = null
        noCallback = null
    }
    visible: false
    icon: StandardIcon.Warning
    standardButtons: StandardButton.Yes | StandardButton.No
}
