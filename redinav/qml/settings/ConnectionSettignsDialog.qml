import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 1.4
import QtQuick.Dialogs 1.2
import QtQuick.Controls.Styles 1.4
import Qt.labs.settings 1.0
import "."
import "../common/helper.js" as JS

Dialog {
    id: root
    title: !settings || !settings.name ? "New Connection Settings" : qsTr("Edit Connection Settings - %1").arg(settings.name)

    signal testConnection

    // Connection config
    property var settings

    property var items: []
    property var sshItems: []
    property var sslItems: []

    property var sshPrivateKeyContent
    property var sshPassword

    function cleanStyle() {

        if (root.sshPrivateKeyContent)
            root.sshPrivateKeyContent.borderColor = "transparent"
        if (root.sshPassword)
            root.sshPassword.style = validStyle.style

        function clean(items_array) {
            for (var index=0; index < items_array.length; index++) {
                if (items_array[index].itemType !== undefined && items_array[index].itemType === "textarea") {
                    if (items_array[index].borderColor !== undefined) {
                        items_array[index].borderColor = "transparent"
                    }
                }
                else {
                    items_array[index].style = validStyle.style
                }
            }
        }
        clean(items)
        clean(sshItems)
        clean(sslItems)

        validationWarning.visible = false
    }


    function saveConnection() {
        if (!root.settings.sshPassword && !root.settings.sshPrivateKeyContent) {
            root.settings.sshEnabled = false
        }
        if (!root.settings.sshEnabled) {
            //!!!! This is the way to tell RedisClient::Connection that SSH is NOT set (empty host)... not very good, but.. that is how it works for now
            root.settings.sshHost = "";
        }
        backend.updateConnection(root.settings)
    }


    function hideLoader() {
        uiBlocker.visible = false
    }

    function showLoader() {
        uiBlocker.visible = true
    }

    function showMsg(msg) {
        dialog_notification.showMsg(msg)
    }

    function showError(err) {
        dialog_notification.showError(err)
    }

    function validate() {

        cleanStyle()

        function checkItems(items_array) {
            var errors = 0
            for (var index=0; index < items_array.length; index++) {

                var value = undefined

                if (items_array[index].text !== undefined) {
                    value = items_array[index].text
                } else if (items_array[index].host !== undefined) {
                    value = items_array[index].host
                } else if (items_array[index].path !== undefined) {
                    value = items_array[index].path
                }

                if (value !== undefined && value.length === 0) {
                    errors++
                    items_array[index].style = invalidStyle
                }
            }
            return errors
        }


        var errors_count = checkItems(items)

        if (root.settings.sshEnabled) {
            errors_count += checkItems(sshItems)

            // Also, if BOTH Key & Pass is empty, this is invalid
            if (!root.settings.sshPassword && !root.settings.sshPrivateKeyContent) {
                root.sshPassword.style = invalidStyle
                root.sshPrivateKeyContent.borderColor = "red"
                errors_count++
            }

        }

        if (root.settings.sslEnabled)
            errors_count += checkItems(sslItems)

        return errors_count === 0
    }

    onVisibleChanged: {
        if (visible)
            settingsTabs.currentIndex = 0
    }

    Component {
        id: invalidStyle
        TextFieldStyle {
            background: Rectangle {
                color: activePalette.window
                radius: 2
                implicitWidth: 100
                implicitHeight: 24
                border.color: "red"
                border.width: 1
            }
        }
    }
    TextField { id: validStyle; visible: false}


    contentItem: Rectangle {
        implicitWidth: 700
        implicitHeight: JS.isOSX() ? 590 : 625

        color: activePalette.window

        ColumnLayout {
            id: mainContent
            anchors.fill: parent
            anchors.margins: 5

            TabView {
                id: settingsTabs
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumHeight: JS.isOSX() ? 490 : 490

                Tab {
                    id: mainTab
                    title: "Connection Settings"
                    anchors.margins: 10



                    ColumnLayout {
                        id: mainSettings
                        anchors.fill: parent

                        GroupBox {
                            title: "Main Settings"
                            Layout.fillWidth: true
                            Layout.alignment: Qt.AlignTop

                            GridLayout {
                                anchors.fill: parent
                                columns: 2

                                Label { text: "Name" }

                                TextField {
                                    textColor: activePalette.text
                                    id: connectionName
                                    objectName: "connection_settings_connectionName"
                                    Layout.fillWidth: true
                                    placeholderText: "Connection Name"
                                    text: root.settings ? root.settings.name : ""
                                    Component.onCompleted: root.items.push(connectionName)
                                    onTextChanged: root.settings.name = text
                                }

                                Label { text: "Address" }

                                AddressInput {
                                    id: connectionAddress
                                    placeholderText: "Redis server host/ip"
                                    host: root.settings ? root.settings.originalHost : ""
                                    port: root.settings ? root.settings.originalPort : 0
                                    Component.onCompleted: root.items.push(connectionAddress)
                                    onHostChanged: {
                                        if (root.settings) {
                                            root.settings.host = host
                                            root.settings.originalHost = host
                                        }
                                    }
                                    onPortChanged: {
                                        if (root.settings) {
                                            root.settings.port = port
                                            root.settings.originalPort = port
                                        }
                                    }
                                }

                                Label { text: "Auth" }

                                PasswordInput {
                                    id: connectionAuth
                                    Layout.fillWidth: true
                                    placeholderText: "(Optional) Redis plain text authentication password"
                                    text: root.settings ? root.settings.auth : ""
                                    onTextChanged: root.settings.auth = text
                                }
                            }
                        }

                        GroupBox {
                            id: securitySettings
                            title: "Security"

                            Layout.columnSpan: 2
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            Layout.alignment: Qt.AlignTop

                            ExclusiveGroup {
                                id: connectionSecurityExGroup
                                onCurrentChanged: {
                                    switch (current.objectName) {
                                    case "connection_settings_securitySshRadioButton":
                                        root.settings.sslEnabled = false;
                                        root.settings.sshEnabled = true;
                                        break;
                                    case "connection_settings_securitySslRadioButton":
                                        root.settings.sslEnabled = true;
                                        root.settings.sshEnabled = false;
                                        break;
                                    default:
                                        root.settings.sslEnabled = false;
                                        root.settings.sshEnabled = false;
                                        break;
                                    }
                                    root.cleanStyle()
                                }
                            }

                            GridLayout {
                                anchors.fill: parent
                                columns: 2

                                RadioButton {
                                    id: securityNoneRadioButton
                                    objectName: "connection_settings_securityNoneRadioButton"
                                    text: "None"
                                    checked: root.settings ? !root.settings.sslEnabled && !root.settings.sshEnabled : true
                                    exclusiveGroup: connectionSecurityExGroup
                                    Layout.columnSpan: 2
                                }

                                RadioButton {
                                    id: securitySslRadioButton
                                    objectName: "connection_settings_securitySslRadioButton"
                                    Layout.columnSpan: 2
                                    text: "SSL/TLS (supports up to latest secure versions of SSL and TLS)"
                                    exclusiveGroup: connectionSecurityExGroup
                                    checked: root.settings ? root.settings.sslEnabled : false
                                }


                                Item { Layout.preferredWidth: 20 }

                                ColumnLayout {
                                    enabled: securitySslRadioButton.checked
                                    visible: enabled
                                    Layout.fillWidth: true

                                    Label {
                                        text: "These keys and certificates are used for bidirectional handshaking only"
                                    }

                                    RowLayout {
                                        Layout.fillWidth: true
                                        Label { text: "Client Certificate (optional)" }
                                        Item { Layout.fillWidth: true }
                                        Button {
                                            text: "Import"
                                            onClicked: {
                                                if (JS.isOSX())
                                                    root.visible = false
                                                fileSelector.title = qsTr("Import client certificate in PEM format")
                                                fileSelector.nameFilters = ["Certificates (*.crt *.pem *.cer)", "All files (*)"]
                                                fileSelector.selectExisting = true
                                                fileSelector.callback = function(fileUrl) {
                                                    var path = backend.getPathFromUrl(fileUrl)
                                                    sslLocalCertContent.text = backend.getFileContent(path);
                                                }
                                                fileSelector.open()
                                                if (JS.isOSX())
                                                    root.visible = true
                                            }
                                        }
                                        Button {
                                            Layout.maximumWidth: 30
                                            text: "\u2716"
                                            onClicked: {
                                                sslLocalCertContent.text = ""
                                            }
                                        }

                                    }

                                    TextArea {
                                        id: sslLocalCertContent
                                        text: root.settings ? root.settings.sslLocalCertContent : ""
                                        Layout.fillWidth: true
                                        Layout.preferredHeight: 50
                                        onTextChanged: {
                                            root.settings.sslLocalCertContent = text
                                        }
                                    }


                                    RowLayout {
                                        Layout.fillWidth: true
                                        Label { text: "Client Private Key (optional)" }
                                        Item { Layout.fillWidth: true }
                                        Button {
                                            text: "Import"
                                            onClicked: {
                                                if (JS.isOSX())
                                                    root.visible = false
                                                fileSelector.title = qsTr("Import private key in PEM format")
                                                fileSelector.nameFilters = ["Keys (*.key *.pem)", "All files (*)"]
                                                fileSelector.selectExisting = true
                                                fileSelector.callback = function(fileUrl) {
                                                    var path = backend.getPathFromUrl(fileUrl)
                                                    sslPrivateKeyContent.text = backend.getFileContent(path);
                                                }
                                                fileSelector.open()
                                                if (JS.isOSX())
                                                    root.visible = true
                                            }
                                        }
                                        Button {
                                            Layout.maximumWidth: 30
                                            text: "\u2716"
                                            onClicked: {
                                                sslPrivateKeyContent.text = ""
                                            }
                                        }
                                    }


                                    TextArea {
                                        id: sslPrivateKeyContent
                                        text: root.settings ? root.settings.sslPrivateKeyContent : ""
                                        Layout.fillWidth: true
                                        Layout.preferredHeight: 50
                                        onTextChanged: {
                                            root.settings.sslPrivateKeyContent = text
                                        }
                                    }

                                    RowLayout {
                                        Layout.fillWidth: true
                                        Label { text: "CA Certificate (optional)" }
                                        Item { Layout.fillWidth: true }
                                        Button {
                                            text: "Import"
                                            onClicked: {
                                                if (JS.isOSX())
                                                    root.visible = false
                                                fileSelector.title = qsTr("Import Certificate Authority certificate in PEM format")
                                                fileSelector.nameFilters = ["Certificates (*.crt *.cer *.pem)", "All files (*)"]
                                                fileSelector.selectExisting = true
                                                fileSelector.callback = function(fileUrl) {
                                                    var path = backend.getPathFromUrl(fileUrl)
                                                    sslCaCertContent.text = backend.getFileContent(path);
                                                }
                                                fileSelector.open()
                                                if (JS.isOSX())
                                                    root.visible = true
                                            }
                                        }
                                        Button {
                                            Layout.maximumWidth: 30
                                            text: "\u2716"
                                            onClicked: {
                                                sslCaCertContent.text = ""
                                            }
                                        }

                                    }


                                    TextArea {
                                        id: sslCaCertContent
                                        text: root.settings ? root.settings.sslCaCertContent : ""
                                        Layout.fillWidth: true
                                        Layout.preferredHeight: 50
                                        onTextChanged: {
                                            root.settings.sslCaCertContent = text
                                        }
                                    }

                                }

                                RadioButton {
                                    id: securitySshRadioButton
                                    objectName: "connection_settings_securitySshRadioButton"
                                    Layout.columnSpan: 2
                                    text: "SSH Tunnel"
                                    exclusiveGroup: connectionSecurityExGroup
                                    checked: root.settings ? root.settings.sshEnabled : false
                                }


                                Item { Layout.preferredWidth: 20 }

                                ColumnLayout {
                                    id: sshSettings
                                    visible: securitySshRadioButton.checked
                                    RowLayout {
                                        Layout.fillWidth: true
                                        Label { text: "Host/IP"; Layout.preferredWidth: 100 }

                                        AddressInput {
                                            Layout.fillWidth: true
                                            id: sshAddress
                                            placeholderText: "Remote Host with SSH server"
                                            port: root.settings ? root.settings.sshPort : 22
                                            host: root.settings ? root.settings.sshHost : ""
                                            Component.onCompleted: {
                                                root.sshItems.push(sshAddress)
                                                root.settings.sshHost = host
                                                root.settings.sshPort = port
                                            }
                                            onHostChanged: root.settings.sshHost = host
                                            onPortChanged: root.settings.sshPort = port
                                        }

                                    }

                                    RowLayout {
                                        Layout.fillWidth: true
                                        Label { text: "User"; Layout.preferredWidth: 100  }
                                        TextField {
                                            id: sshUser
                                            Layout.fillWidth: true
                                            placeholderText: "Valid SSH User Name"
                                            text: root.settings ? root.settings.sshUser : ""
                                            Component.onCompleted: root.sshItems.push(sshUser)
                                            onTextChanged: root.settings.sshUser = text
                                        }
                                    }

                                    RowLayout {
                                        Layout.fillWidth: true
                                        Label { text: "Private key" }
                                        Item { Layout.fillWidth: true }
                                        Button {
                                            text: "Import"
                                            onClicked: {
                                                if (JS.isOSX())
                                                    root.visible = false
                                                fileSelector.title = qsTr("Import private key in PEM format")
                                                fileSelector.nameFilters = ["Keys (*.key *.pem)", "All files (*)"]
                                                fileSelector.selectExisting = true
                                                fileSelector.callback = function(fileUrl) {
                                                    var path = backend.getPathFromUrl(fileUrl)
                                                    sshPrivateKeyContent.text = backend.getFileContent(path);
                                                }
                                                fileSelector.open()
                                                if (JS.isOSX())
                                                    root.visible = true
                                            }
                                        }
                                        Button {
                                            Layout.maximumWidth: 30
                                            text: "\u2716"
                                            onClicked: {
                                                sshPrivateKeyContent.text = ""
                                            }
                                        }

                                    }

                                    RowLayout {
                                        Layout.fillWidth: true
                                        Rectangle {
                                            Layout.fillWidth: true
                                            Layout.preferredHeight: 100
                                            color: sshPrivateKeyContent.borderColor
                                            TextArea {
                                                anchors.fill: parent
                                                anchors.margins: 1
                                                id: sshPrivateKeyContent
                                                objectName: "connection_settings_sshPrivateKeyContent"
                                                text: root.settings ? root.settings.sshPrivateKeyContent : ""
                                                property string itemType: "textarea"
                                                property string borderColor: "transparent"
                                                Component.onCompleted:  {
                                                    root.sshPrivateKeyContent = sshPrivateKeyContent
                                                }
                                                onTextChanged: {
                                                    root.settings.sshPrivateKeyContent = text
                                                }
                                            }
                                        }
                                    }

                                    RowLayout {
                                        Label { text: "Password" }
                                    }

                                    RowLayout {
                                        PasswordInput {
                                            id: sshPassword
                                            objectName: "connection_settings_sshPassword"
                                            Layout.fillWidth: true
                                            placeholderText: "SSH User Password"
                                            text: root.settings ? root.settings.sshPassword : ""
                                            Component.onCompleted:  {
                                                root.sshPassword = sshPassword
                                            }
                                            onTextChanged: root.settings.sshPassword = text
                                        }
                                    }

                                }

                                Item {Layout.fillHeight: true;}

                            }
                        }
                    }
                }

                Tab {
                    title: "Advanced Settings"
                    anchors.margins: 10

                    GridLayout {
                        anchors.fill: parent

                        columns: 2

                        Label { text: "Namespace Separator" }

                        TextField
                        {
                            id: namespaceSeparator
                            Layout.fillWidth: true
                            objectName: "connection_settings_namespaceSeparator"
                            placeholderText: "Separator used for namespace extraction from keys"
                            text: root.settings ? root.settings.namespaceSeparator : ""
                            onTextChanged: root.settings.namespaceSeparator = text
                        }

                        Label { text: "Connection Timeout (sec)" }

                        SpinBox {
                            id: connectionTimeout
                            Layout.fillWidth: true
                            minimumValue: 5
                            maximumValue: 100000
                            value: {
                                return root.settings ? (root.settings.connectionTimeout / 1000.0) : 0
                            }
                            onValueChanged: root.settings.connectionTimeout = value * 1000
                        }

                        Label { text: "Execution Timeout (sec)" }

                        SpinBox {
                            id: executeTimeout
                            Layout.fillWidth: true
                            minimumValue: 60
                            maximumValue: 100000
                            value: {
                                return root.settings ? (root.settings.executeTimeout / 1000.0) : 0
                            }
                            onValueChanged: root.settings.executeTimeout = value * 1000
                        }

                        Label { Layout.fillWidth: true; text: "Use server side scripting to collect database keys (faster)"; }

                        CheckBox {
                            Layout.alignment: Qt.AlignTop
                            id: luaKeysLoading
                            checked: root.settings ? (root.settings.luaKeysLoading) : true
                            onCheckedChanged: root.settings.luaKeysLoading = checked

                        }

                        Label { Layout.fillWidth: true; text: "Read Only" }

                        CheckBox {
                            id: readOnly
                            checked: root.settings ? (root.settings.readOnly) : false
                            onCheckedChanged: root.settings.readOnly = checked

                        }

                        Item {
                            Layout.columnSpan: 2
                            Layout.fillHeight: true
                            Layout.fillWidth: true
                        }
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true

                Item {
                    visible: !validationWarning.visible
                    Layout.fillWidth: true
                }

                Rectangle {
                    id: validationWarning
                    visible: false
                    Layout.fillWidth: true
                    height: parent.height
                    color: activePalette.window
                    RowLayout {
                        anchors.centerIn: parent
                        Image {
                            Layout.preferredWidth: 25
                            Layout.preferredHeight: 25
                            source: qsTr("qrc:/resources/images/icons/themes/%1/alert.svg").arg(approot.theme)
                        }
                        Text {
                            text: "Invalid settings!"
                            color: activePalette.text
                        }
                    }
                }


                Button {
                    id: dialogOkButton
                    objectName: "connection_settings_dialogOkButton"
                    text: "OK"
                    onClicked: {
                        if (root.validate()) {
                            saveConnection()
                            root.close()
                        } else {
                            validationWarning.visible = true
                        }
                    }
                }

                Button {
                    text: "Cancel"
                    onClicked: root.close()
                }
            }
        }

        Rectangle {
            id: uiBlocker
            visible: false
            anchors.fill: parent
            color: Qt.rgba(0, 0, 0, 0.1)

            Item {
                anchors.fill: parent
                BusyIndicator { anchors.centerIn: parent; running: true }
            }

            MouseArea {
                anchors.fill: parent
            }
        }

        MessageDialog {
            id: dialog_notification
            visible: false
            icon: StandardIcon.Warning
            standardButtons: StandardButton.Ok

            function showError(msg) {
                icon = StandardIcon.Warning
                text = msg
                open()
            }

            function showMsg(msg) {
                icon = StandardIcon.Information
                text = msg
                open()
            }
        }
    }

    onTestConnection: {
        if (backend.testConnectionSettings(settings)) {
            hideLoader()
            showMsg("Successful connection to redis-server")
        } else {
            hideLoader()
            showError("Can't connect to redis-server")
        }
    }

}


