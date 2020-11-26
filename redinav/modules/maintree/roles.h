#pragma once
#include <QObject>
#include <Qt>

enum ItemRoles {
    ItemDcorationRole = Qt::DecorationRole,
    ItemName = Qt::UserRole + 1, // 257; holds server name, db name (e.g. #1), namespace name (last part), key
    ItemOriginalName,
    ItemType,
    ItemFullPath, // 260
    ItemIsInitiallyExpanded,
    ItemDepth,
    ItemState,
    ItemDbNumber, // Holds the DB ID if the item is database
    ItemFilter, // 265            // Holds a filter string, if any
    ItemNamespaceFullpath, // Holds namespace fullpath, if it is a namespace
    ItemConnectionPointer,
    ItemTotalKeys, // Total number of keys, if this is a Database or Namespace item
    ItemLocked,
    ItemRemoved, // 270
    ItemConnectionMode,
    ItemConnectionUuid
};
