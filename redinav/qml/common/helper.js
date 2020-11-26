var dumpWarning = qsTr("This operation is not meant to be a replacement of dump/restore tools available from other Redis related packages. Please use with caution as it might be very slow! " +
                  "On some servers, especially remote ones, dumping 1000 keys may take more than 10 minutes! " +
                  "\n\nYou can always cancel the dump process by clicking right mouse button on status bar dump progress message!" +
                  "\n\nDo you want to continue?");


function getSupportedKeyTypes() {
    return ["string", "list", "set", "zset", "hash"]
}

function isOSX() {
    return Qt.platform.os == "osx"
}

function isOSXRetina(screen) {
    return isOSX() && screen.devicePixelRatio> 1
}

function validateAddKeyValues(keyName, keyType, value1, value2) {
    if (keyType === "")
        return false
    return true
}

function getKeysListFromIndexes(indexes) {

    var keysList = []
    if (indexes.length > 0) {
        for (var i=0; i < indexes.length; i++) {
            keysList.push(mainTreeModel.roleValue(indexes[i], "fullpath"))
        }
    }

    return keysList
}

function shortcuts(shortname) {
    switch(shortname) {
        case "addnewkey"    : return (isOSX() ? ["Meta+N"] : ["Ctrl+N"]);
        case 'reload'       : return (isOSX() ? ["Meta+R", "F5"] : ["F5", "Ctrl+R"]);
        case 'filter'       : return (isOSX() ? ["Meta+F"] : ["Ctrl+F"]);
        case 'clearfilter'  : return (isOSX() ? ["Meta+Shift+F"] : ["Ctrl+Shift+F"]);
        case 'rename'       : return (isOSX() ? ["Meta+E"] : ["F2"]);
        case 'keyinnewtab'  : return (isOSX() ? ["Meta+K"] : ["Ctrl+K"]);
        case 'deletekey'    : return (isOSX() ? ["Meta+D", "Del"] : ["Del"]);
        case 'editconn'     : return (isOSX() ? ["Meta+E"] : ["Ctrl+E"]);
        case 'serverinfo'   : return (isOSX() ? ["Meta+I"] : ["Ctrl+I"]);
        case 'savetoredis'  : return (isOSX() ? ["Ctrl+S"] : ["Ctrl+S"]);

    }
}

function shortcutTip(shortname) {
    return (shortcuts(shortname)[0] !== "") ? shortcuts(shortname)[0] : "";
}


function shortcutTipIf(shortname) {
    var tip = (shortcuts(shortname)[0] !== "") ? shortcuts(shortname)[0] : ""
    if (tip === "")
        return "";
    if (isOSX())
        return "\t" + tip
    else
        //return ""
        return " [" + tip + "]"

}

function someDumpIsRunning() {
    if (backend.isKeysSelectionDumpIsRunning())
        return true
    if (backend.isNsOrDbDumpIsRunning())
        return true

    return false;
}


function loader(source, params, callback) {
    generalLoader.active = false
    generalLoader.onLoadedCallback = callback
    generalLoader.setSource(source, params)
    generalLoader.active = true
}

/**
 * MacOs native menus are always white/light, hence we have to use icons from "default" theme for MacOs
 */
function themeFix(theme) {
    return isOSX() ? "default" : theme;
}

function requestLicenseActivation() {
    licenseKeyDialog.showDialog({
        "title": "License key acivation",
        "confirmCallback": function(licenseKey) {
            licenseManager.setLicenseKey(licenseKey)
            licenseManager.activate()
        },
        "cancelCallback": function() {
        }
    })
}


function keyTypeMemberTitle(type, capitalize) {
    var title
    if (type === "hash")
        title = "field"
    else
        title = "element"

    if (capitalize) {
        title = title.replace(/^\w/, function (chr) {
            return chr.toUpperCase();
          });
    }

    return title
}

function getRandomInt(max) {
  return Math.floor(Math.random() * Math.floor(max));
}
