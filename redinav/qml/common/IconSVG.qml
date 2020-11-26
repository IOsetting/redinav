import QtQuick 2.9

Image {
    id: root
    property int size: 12
    sourceSize.width: size
    sourceSize.height: size
    height: size
    width: size
    anchors.verticalCenter: parent.verticalCenter
    fillMode: Image.PreserveAspectFit || Image.Stretch
    mipmap: true
    smooth: true
    antialiasing: true

}
