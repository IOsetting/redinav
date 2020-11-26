import QtQuick 2.9

Text {
    property string html
    property string styleString: ""
    textFormat: Qt.RichText
    text: styleString + html
    wrapMode: Text.Wrap
    onLinkActivated: Qt.openUrlExternally(link)
    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.NoButton // we don't want to eat clicks on the Text
        cursorShape: parent.hoveredLink ? Qt.PointingHandCursor : Qt.ArrowCursor
    }

}
