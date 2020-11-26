import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 1.4

Item {
    id: root
    property string label
    property string description
    property bool value
    property var gSettings

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

        Button {
            text: qsTr("Reset")
            onClicked: {
                backend.resetGlobalSettings()
                settingsDialog.close()
            }
        }
    }
}

