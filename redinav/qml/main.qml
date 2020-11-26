import QtQuick.Window 2.3
import QtQuick 2.9
import QtQuick.Controls 1.4
import QtQml.Models 2.3
import Qt.labs.settings 1.0
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.3
import QtQuick.Controls.Styles 1.4
import "."
import "maintree"
import "maindata"
import "settings"
import "common"
import "common/helper.js" as JS

ApplicationWindow {


    function dbFilterDialog(index) {
        JS.loader("qrc:/qml/common/TextFieldDialog.qml",
                  {
                      title: "Filter this database keys",
                      textFieldContent: mainTreeModel.roleValue(index, "filter"),
                      fieldWidth: 200,
                      confirmCallback: function(text) {
                          mainTreeModel.setRoleValue(index, "filter", text);
                          backend.reloadKeys(index)
                      }
                  }, function(loader) {
                      loader.item.open()
                  }
                  );
    }

    function dbFilterClear(index) {
        mainTreeModel.setRoleValue(index, "filter", "");
        backend.reloadKeys(index)
    }

    function addKeyDialog(index) {
        JS.loader("qrc:/qml/maindata/AddKeyDialog.qml", {
                      dbItemIndex : index,
                      connection: mainTreeModel.roleValue(mainTreeModel.connectionParent(index), "connectionpointer"),
                      dbNumber: mainTreeModel.roleValue(index, "dbnumber"),
                      successCallback: function(newIndex) {
                          mainDataArea.activateKey(newIndex, true)
                      }
                  }, function(loader) {
                      loader.item.open()
                  });
    }

    function keyRenameOrCloneDialog(connection, dbNumber, keyFullPath, keyModelIndex, cloneMode) {
        JS.loader("qrc:/qml/maintree/RenameKeyDialog.qml", {
                      connection: connection,
                      dbNumber: dbNumber,
                      origKeyFullPath: keyFullPath,
                      keyFullPath: keyFullPath,
                      keyModelIndex: keyModelIndex,
                      cloneMode: cloneMode
                  }, function(loader) {
                      loader.item.open()
                  })
    }

    function keyDeleteDialog(connection, dbNumber, selectedIndexes, keyModelIndex) {
        JS.loader("qrc:/qml/maindata/DeleteKeyDialog.qml", {
                      selectedIndexes: selectedIndexes.length > 1 ? selectedIndexes : [keyModelIndex],
                                                                    connection: connection,
                                                                    dbNumber: dbNumber
                  },
                  function(loader) {
                      loader.item.open()
                  });
    }

    id: approot
    property string theme: "default"
    visible: true
    width: 1200
    height: 800
    title: "RediNav " + Qt.application.version
    color: activePalette.window

    // Adjust main window size and position
    Component.onCompleted: {
        var wRatio = (width * 1.0) / (Screen.width * 1.0)
        var hRatio = (height * 1.0) / (Screen.height * 1.0)

        if (hRatio > 1 || wRatio > 1) {
            width = Screen.width * 0.9
            height = Screen.height * 0.8
        }
        setX(Screen.width / 2 - width / 2)
        setY(Screen.height / 2 - height / 2)

        if (JS.isOSXRetina(Screen)) {
            logAreaWrapper.implicitHeight = 100
        }

    }

    Settings {
        category: "windows_settings"
        property alias x: approot.x
        property alias y: approot.y
        property alias width: approot.width
        property alias height: approot.height
    }


    Settings {
        category: "app"
        property alias appTheme: approot.theme
    }


    SystemPalette {
        id: sysPalette
    }

    FontLoader {
        id: monospacedFont
        Component.onCompleted: {
            source = "qrc:/resources/fonts/Inconsolata-Regular.ttf"
        }
    }

    GlobalSettings {
        id: settingsDialog
    }


    toolBar: AppToolBar {
    }

    statusBar: AppStatusBar {
    }

    // Main split view:  Tree on the left, data to the right
    SplitView {

        id: topSplitView
        width: 0
        height: 0
        anchors.fill: parent

        MainTreeView {
            id: mainTreeView
        }

        Rectangle {

            id: rightSide
            width: 300
            height: 0
            color: activePalette.window
            border.width: 0
            visible: true
            anchors.leftMargin: 0
            anchors.top: parent.top
            anchors.topMargin: 0
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 0
            anchors.right: parent.right
            anchors.rightMargin: 0

            SplitView {
                anchors.fill: parent
                id: rightSideSplit
                orientation: Qt.Vertical

                MainDataArea {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    id: mainDataArea
                    color: activePalette.window
                }

                Rectangle {
                    id: logAreaWrapper
                    height: { return approot.height * (12/100) }
                    Layout.fillWidth: true
                    color: activePalette.window
                    LogArea {
                        anchors.fill: parent
                        id: logArea
                        Connections {
                            target: appLogger
                            onEvent: logArea.append(msg)
                            Component.onCompleted: appLogger.getMessages()
                        }
                    }
                }
            }
        }

    }


    Rectangle {
        id: appBlocker
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


    NotifyDialog {
        id: notifyDialog
    }

    MyFileDialog {
        id: fileSelector
    }

    YesNoDialog {
        id: yesno
    }

    // Used by JS.loader() to load arbitrary component with params and onLoaded callback
    // Particularly good because the component is loaded every time (good to have global settings reloaded, for example)
    Loader {
        id: generalLoader
        property var onLoadedCallback: function() {}
        source: ""
        active: false
        onLoaded: {
            if (typeof onLoadedCallback === "function") {
                onLoadedCallback(generalLoader)
            }
        }
    }

    SystemPalette { id: activePalette; colorGroup: SystemPalette.Active }
    SystemPalette { id: disabledPalette; colorGroup: SystemPalette.Disabled }
    SystemPalette { id: inactivePalette; colorGroup: SystemPalette.Inactive }




}
