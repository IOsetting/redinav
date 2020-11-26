import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Window 2.3
import org.redinav.qml 1.0
import Qt.labs.settings 1.0

TextField {
    id: root
    property int radius: 2
    style: TextFieldStyle {
        textColor: activePalette.windowText
        renderType: Settings.isMobile ? Text.QtRendering : Text.NativeRendering
        background: Rectangle {
            radius: root.radius
            implicitWidth: 100
            implicitHeight: 24
            border.color: control.activeFocus ? "#47b" : activePalette.mid
            border.width: 1
            color: activePalette.alternateBase
        }
    }
}
