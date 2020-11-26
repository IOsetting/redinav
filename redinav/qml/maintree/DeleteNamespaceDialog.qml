import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 1.4
import QtQuick.Dialogs 1.2
import QtQuick.Controls 1.4
import "../common"

MessageDialog {

    property string filter: ""
    property var index: null
    property string prefix: ""
    property var connection
    property var dbNumber

    id: root
    title: {
        if (root.filter)
            return "Delete Namespace Keys (filter: " + root.filter + ")"
        else
            return "Delete Namespace Keys"
    }
    text: {
        return qsTr("Please confirm deletion of %1 key(s)").arg(mainTreeModel.roleValue(root.index, "totalkeys"))
    }

    onYes: {
        if (root.prefix === "" || root.prefix === null) {
            notifyDialog.showError("Empty prefix found. Operations cancelled (dangerous)!")
            return
        }
        backend.deleteNamespaceKeys(root.connection,  root.dbNumber, root.prefix, root.filter, root.index)
    }
    visible: false
    icon: StandardIcon.Warning
    standardButtons: StandardButton.Yes | StandardButton.No


}
