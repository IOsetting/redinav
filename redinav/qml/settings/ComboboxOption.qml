import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 1.4

Item {
    id: root
    property string label
    property string description
    property string value
    property alias model: val.model

    onValueChanged: {        
        if (val.currentText != root.value) {
            val.currentIndex = val.find(root.value)
        }
    }

    RowLayout {
        anchors.fill: parent

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 1

            Label {
                Layout.fillWidth: true
                text: root.label
            }

            Text {
                color: activePalette.text
                text: root.description
            }
        }

        ComboBox {
            id: val

            Layout.minimumWidth: 80

            onActivated: {
                if (index >= 0)
                    root.value = textAt(index)
            }

            onCountChanged: {
                if (model) {
                    currentIndex = val.find(root.value)                    
                }
            }

            Component.onCompleted: {                
                if (model) {
                    currentIndex = val.find(root.value)
                }
            }
        }
    }
}

