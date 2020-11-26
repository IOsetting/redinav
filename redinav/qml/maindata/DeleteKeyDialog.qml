import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 1.4
import QtQuick.Dialogs 1.2
import QtQuick.Controls 1.4
import "../common"

MessageDialog {


    property var selectedIndexes : []
    property var connection: null
    property int dbNumber: -1

    id: root
    title: {
        qsTr("Delete Key(s)")
    }
    text: {
        if (selectedIndexes.length > 1) {
            return qsTr("Are you sure you want to delete %1 key(s) ?").arg(selectedIndexes.length)
        }
        else  {
            return qsTr("Are you sure you want to delete this key?\n\n%1").arg(mainTreeModel.roleValue(selectedIndexes[0], "fullpath"))
        }
    }

    onYes: {
        if (selectedIndexes.length > 1) {
            // Ensure all selected items have the same parent!!!
            var firstParent = mainTreeModel.parent(selectedIndexes[0])
            var lastParent = mainTreeModel.parent(selectedIndexes[selectedIndexes.length-1])
            if (lastParent !== firstParent) {
                root.close()
                notifyDialog.showError("Cross branch selection not allowed!")
                return
            }
        }
        backend.deleteKeysCollectionByIndexes(connection, dbNumber, selectedIndexes)
    }


    visible: false
    icon: StandardIcon.Warning
    standardButtons: StandardButton.Yes | StandardButton.No


}
