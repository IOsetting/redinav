import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Dialogs 1.2
import "../common/json-tools.js" as Json
import "../common"

ColumnLayout {
    id: root
    spacing: 0
    anchors.fill: parent

    property string text   // Printable string
    property var original  // Can be string or object (ArrayBuffer for bunary values)
    property string currentFormat: "original"

    property bool showSize: true
    property int maxSize: 150000
    property string label: ""
    property var formattersButtons: []
    property bool isBinary: backend.isBinaryString(root.original);
    property bool isReadOnly: false
    property bool allowSaveToFile: true
    property bool topSpacer: false
    property bool bottomSpacer: false

    // Input CAN BE be:  QByteArray (JS:ArrayBuffer)  or QString (JSLstring)
    function setText(input) {
        root.text       = backend.toPrintable(input)
        root.original   = input
        // Attempt to do PHP-unserialization; if OK, use it
        var tmp = backend.phpSerializedPretty(root.text);
        if (tmp !== "***INVALID PHP SERIALIZATION***" ) {
            textArea.text = tmp
        }
        else {
            textArea.text   = root.text
        }
        var textLength = root.text.length
        var originaLength = (typeof root.original === "object") ? root.original.byteLength : root.original.length
    }

    function getText() {
        return textArea.text
    }

    Rectangle { visible: topSpacer; height: 5; Layout.fillWidth: true; color: "transparent";}

    // Status, Buttons, etc.
    Rectangle {
        id: rectangle
        Layout.fillWidth: true
        color: "transparent"
        Layout.minimumHeight: 30
        Layout.fillHeight: false
        border.width: 0
        Layout.alignment: Qt.AlignHCenter | Qt.AlignTop

        RowLayout {
            anchors.right: parent.right
            anchors.left: parent.left
            anchors.leftMargin: 0
            spacing: 5
            Layout.fillWidth: true

            Button {
                iconSource: qsTr("qrc:/resources/images/icons/themes/%1/ol_save.svg").arg(approot.theme)
                id: saveButton
                visible: root.allowSaveToFile
                text: " " + qsTr("File")
                tooltip: "Save value to local file"
                onClicked: {
                    fileSelector.title = qsTr("Select file to save")
                    fileSelector.nameFilters = ["Text (*.txt)", "All files (*.*)"]
                    fileSelector.selectExisting = false
                    fileSelector.callback = function(fileUrl) {
                        backend.saveContentToFile(backend.getPathFromUrl(fileUrl), root.original)
                    }
                    fileSelector.open()
                }
            }

            Label {
                text: label
            }

            Text {
                visible: root.showSize
                text:"  " + backend.humanSize(backend.binaryStringLength(getText()));
                color: activePalette.text
            }

            Text {
                text: "[CHANGED]"
                visible: backend.toPrintable(root.original) !== textArea.text
                color: "red";

            }

            Text {
                text: "[Binary]"
                visible: isBinary
                color: "green";
            }

            Text {
                text: "[Read only]"
                visible: isReadOnly
                color: "red";
            }

            //--
            Item { Layout.fillWidth: true; }

            Button {
                visible: { formattersButtons.indexOf("json-pretty") !== -1 }
                text: "JSON"
                onClicked: {
                    textArea.text = Json.prettyPrint(textArea.text)
                }
            }

            Button {
                visible: { formattersButtons.indexOf("minify") !== -1 }
                text: "Minify"
                onClicked: {
                    textArea.text = Json.minify(textArea.text)
                }
            }

            Button {
                // visible: { !isBinary && formattersButtons.indexOf("phpunserialize") !== -1 }
                visible: { formattersButtons.indexOf("phpunserialize") !== -1 }
                text: "PHP Pretty"
                onClicked: {
                    var tmp = backend.phpSerializedPretty(textArea.text)
                    if (tmp !== "***INVALID PHP SERIALIZATION***") {
                        textArea.text = backend.phpSerializedPretty(textArea.text)
                    }
                }
            }

            Button {
                visible: { formattersButtons.indexOf("original") !== -1 }
                text: "ORIGINAL"
                onClicked: {
                    textArea.text = backend.toPrintable(root.original)
                }
            }


        }
    }

    // TEXT
    Rectangle {
        border.width: 0
        Layout.fillHeight: true
        Layout.fillWidth: true
        //color: "transparent"
        color: activePalette.alternateBase

        TextArea {
            anchors.fill: parent
            readOnly: root.isReadOnly
            textFormat: TextEdit.PlainText
            id: textArea
            text: root.text
            wrapMode: Text.WrapAnywhere

            // Without the following, TextArea flickers in Dark theme !!!!! (showing first white background then going dark)
            backgroundVisible: false
        }



    }

    Rectangle { visible: bottomSpacer; height: 5; Layout.fillWidth: true; color: "transparent";}


}
