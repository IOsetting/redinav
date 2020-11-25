import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Controls.Private 1.0

Item {
    id: root
    property string label
    property string description
    property bool value

    onValueChanged: {
        if (val.checked != root.value) {
            val.checked = root.value
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

        Switch {
            id: val

            style: SwitchStyle {
                handle: Rectangle {
                    opacity: control.enabled ? 1.0 : 0.5
                    implicitWidth: Math.round((parent.parent.width - padding.left - padding.right)/2)
                    implicitHeight: control.height - padding.top - padding.bottom
                    border.color: control.activeFocus ? Qt.darker(activePalette.highlight, 2) : Qt.darker(activePalette.button, 2)
                    property color bg: control.activeFocus ? Qt.darker(activePalette.highlight, 1.2) : activePalette.button
                    radius: 2
                }
            }


            onCheckedChanged: {
                root.value = checked
            }
        }
    }
}

