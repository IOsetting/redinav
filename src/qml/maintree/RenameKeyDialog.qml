import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 1.4
import QtQuick.Dialogs 1.2
import "../common"

Dialog {

    property var connection
    property int dbNumber
    property string keyFullPath
    property string origKeyFullPath
    property var keyModelIndex
    property bool cloneMode

    id: root
    title: cloneMode ? qsTr("Clone key") : qsTr("Rename key")

    width: 520

    onVisibilityChanged: {
        if (visible === true){
            newKeyName.focus = true
        }
    }


    ColumnLayout {
        implicitWidth: 500
        implicitHeight: 100
        width: 500

        TextField {
            implicitWidth: 450
            focus: true
            id: newKeyName;
            Layout.fillWidth: true;
            text: root.origKeyFullPath + (cloneMode ? "-copy" : "")
        }
    }

    onAccepted: {
        var newName = newKeyName.text
        var message = ""
        if (newName.length === "0" || (newName === origKeyFullPath)) {
            return;
        }

        if (cloneMode === true) {
            message  = backend.cloneKey(connection, dbNumber, keyFullPath, newName)
        }
        else {
            message  = backend.renameKey(connection, dbNumber, keyFullPath, newName)
        }

        if (message) {
            root.close()
            notifyDialog.showError(message)
        }
        else {
            if (cloneMode === true) {
                var dbItemIndex = mainTreeModel.databaseParent(keyModelIndex)
                var newIndex = mainTreeModel.addRowWithData(dbItemIndex, {
                    "name"      : newName,
                    "type"      : "key",
                    "dbnumber"  : "" + mainTreeModel.roleValue(dbItemIndex, "dbnumber"),
                    "fullpath"  : newName,
                    "filter"    : ""
                });
                mainTreeView.selection.setCurrentIndex(newIndex, 3) // clear && select
                //redisKeyActivated(newIndex, true)
            }
            else {
                mainTreeModel.setRoleValue(keyModelIndex, "fullpath", newName)
                mainTreeModel.setRoleValue(keyModelIndex, "name", newName)

                var path = mainTreeModel.connectionParent(keyModelIndex).row + "-" + mainTreeModel.databaseParent(keyModelIndex).row + "-" + origKeyFullPath
                mainDataArea.removeKeyTabs(path)
            }
        }
        return

    }

    visible: false
    standardButtons: StandardButton.Ok | StandardButton.Cancel



}
