import QtQuick 2.2
import QtQuick.Layouts 1.3
import QtQuick.Controls 1.4
import QtQuick.Dialogs 1.2
import "helper.js" as JS

FileDialog {
    visible: false
    id: root
    property var callback: null
    folder: shortcuts.documents
    title: "Select file"
    nameFilters: ["*.*"]
    selectExisting: false

    onAccepted: {
        if (fileUrl && (typeof callback === "function")) {
            callback(backend.getPathFromUrl(fileUrl))
        }
    }
}
