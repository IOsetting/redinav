import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 1.4
import QtQuick.Dialogs 1.2

Dialog {


    id: root

    property var confirmCallback: null
    property string textFieldContent: ""
    property var fieldWidth: false

    function showDialog(props) {
        setProps(props)
        root.open();
    }

    function setProps(props) {
        title = props.title;
        textFieldContent = props.textFieldContent
        confirmCallback = props.confirmCallback
    }


    standardButtons: StandardButton.Ok | StandardButton.Cancel
    TextField  {
        implicitWidth: root.fieldWidth !== false ? root.fieldWidth : 100
        id: textFieldControl
        text: textFieldContent
        anchors.centerIn: parent
    }
    onVisibilityChanged: {
        if(visible === true){
            textFieldControl.focus = true
            textFieldControl.text = textFieldContent
        }
    }
    onAccepted: {
        if (confirmCallback)
            confirmCallback(textFieldControl.text)
    }
}
