import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Dialogs 1.2
import QtQuick.Window 2.3
import "../common"
import "../common/helper.js" as JS

Dialog {

    id: root
    title: "License key activation"
    width: 600
    height: 240
    visible: false


    function showDialog(props) {
        setProps(props)
        root.open();
    }

    function setProps(props) {
        title = props.title;
        confirmCallback = props.confirmCallback
        cancelCallback = props.cancelCallback
    }

    property var confirmCallback: null
    property var cancelCallback: null


    //standardButtons: StandardButton.NoButton
    standardButtons: StandardButton.Ok | StandardButton.Cancel

    onVisibilityChanged: {
        if (visible === true){
            licenseKey.focus = true
        }
    }

    onAccepted: {
        root.close()
        if (confirmCallback) {
            confirmCallback(licenseKey.text)
        }
    }

    onRejected: {
        root.close()
        if (cancelCallback) {
            cancelCallback()
        }
    }

    Item {
        anchors.fill: parent

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 5

            RowLayout {

                Layout.alignment: Qt.AlignTop

                Text {
                    color: activePalette.text
                    Layout.minimumWidth: 100
                    text: "License key"
                    Layout.alignment: Qt.AlignLeft
                }

                TextField {
                    id: licenseKey
                    Layout.alignment: Qt.AlignLeft
                    Layout.fillWidth: true
                }

            }


            RowLayout {

                Layout.fillHeight: true
                Layout.fillWidth: true

                Text {
                    color: activePalette.text
                    Layout.minimumWidth: 100
                    text: "Note"
                    Layout.alignment: Qt.AlignLeft
                }


                RichTextWithLinks {
                    Layout.fillWidth: true
                    color: activePalette.text
                    horizontalAlignment: Text.AlignLeft
                    html: qsTr("Enter a valid license key and click Ok to activate it on this device.<br>" +
                          "If you get a message about exceeded number of activations,<br>"+
                          "please contact us at <a href=\"%1/support\">Support page</a> and we will help you."+
                          "<br>Internet connection is required during activation!").arg(licenseManager.storeUrl())
                }


            }

            RowLayout {

                Layout.fillWidth: true

                Text {
                    color: activePalette.text
                    Layout.minimumWidth: 100
                    text: "No license?"
                    Layout.alignment: Qt.AlignLeft
                }


                RichTextWithLinks {
                    Layout.fillWidth: true
                    color: activePalette.text
                    horizontalAlignment: Text.AlignLeft
                    html: qsTr("Don't have a license? <a href=\"%1/buy\">BUY ONE!</a> and help us make this App even better!").arg(licenseManager.storeUrl())
                }


            }


        }

    }


}


