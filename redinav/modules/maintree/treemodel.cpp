#include <QDebug>
#include <QString>
#include <easylogging++.h>

#include <qredisclient/connectionconfig.h>

#include "backend.h"
#include "roles.h"
#include "treeitem.h"
#include "treemodel.h"

using namespace MainTree;

TreeModel::TreeModel(QList<QSharedPointer<RedisClient::Connection> > connections, QObject* parent)
    : QAbstractItemModel(parent)
{

    //-- TEST to be removed
    //--- using example string of tree data
    //--- Here all configured servers should be added
    QFile file(":/resources/default.txt");
    file.open(QIODevice::ReadOnly);
    QString inputData = file.readAll();
    file.close();
    //-- TEST to be removed

    QMap<int, QVariant> rootData;
    rootData[ItemName] = "Name";
    rootData[ItemType] = "Type";
    rootItem = new TreeItem(rootData);

    //-- TEST to be removed
    // setupModelData_OLD(inputData.split(QString("\n")), rootItem);
    //-- TEST to be removed

    setupModelConnectionsData(connections);
    //--
}

TreeModel::~TreeModel()
{
}

QVariant TreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->data(section);

    return QVariant();
}

bool TreeModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant& value, int role)
{
    if (value != headerData(section, orientation, role)) {
        bool result = rootItem->setData(section, value);
        if (result) {
            emit headerDataChanged(orientation, section, section);
            return true;
        }
    }
    return false;
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex& parent) const
{
    if (parent.isValid() && parent.column() != 0)
        return QModelIndex();

    TreeItem* parentItem = getItem(parent);

    TreeItem* childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex TreeModel::parent(const QModelIndex& index) const
{
    if (!index.isValid())
        return QModelIndex();

    TreeItem* item = getItem(index);
    TreeItem* parentItem = item->parent();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->position(), 0, parentItem);
}

int TreeModel::rowCount(const QModelIndex& parent) const
{
    TreeItem* parentItem = getItem(parent);
    return parentItem->childCount();
}

int TreeModel::columnCount(const QModelIndex& /*parent*/) const
{
    return 1;
}

bool TreeModel::hasChildren(const QModelIndex& parent) const
{
    if (!parent.isValid())
        return 0;

    TreeItem* parentItem = getItem(parent);

    return parentItem->childCount() > 0;
}

bool TreeModel::canFetchMore(const QModelIndex& parent) const
{

    if (!parent.isValid())
        return false;

    TreeItem* item = getItem(parent);

    return item && item->canFetchMore();
}

void TreeModel::fetchMore(const QModelIndex& parent)
{
    if (!parent.isValid())
        return;

    TreeItem* item = getItem(parent);

    if (!item)
        return;

    item->fetchMore();
}

QVariant TreeModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    TreeItem* item = getItem(index);
    return item->data(role);
}

bool TreeModel::setData(const QModelIndex& index, const QVariant& value, int role)
{

    TreeItem* item = getItem(index);
    bool result = item->setData(role, value);

    if (result)
        emit dataChanged(index, index);

    return result;
}

Qt::ItemFlags TreeModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    Qt::ItemFlags result = QAbstractItemModel::flags(index);

    return result;
}

bool TreeModel::insertRows(int position, int count, const QModelIndex& parent)
{

    TreeItem* parentItem = getItem(parent);
    bool success;

    beginInsertRows(parent, position, position + count - 1);
    success = parentItem->insertChildren(position, count);
    endInsertRows();

    return success;
}

bool TreeModel::removeRows(int position, int count, const QModelIndex& parent)
{
    TreeItem* parentItem = getItem(parent);
    bool success = true;

    beginRemoveRows(parent, position, position + count - 1);
    success = parentItem->removeChildren(position, count);
    endRemoveRows();

    return success;
}

void TreeModel::removeIndexes(QModelIndexList indexes)
{

    if (indexes.count() <= 0)
        return;

    QModelIndex startIndex = index(0, 0, indexes.at(0).parent());
    QModelIndexList toDelete;
    QModelIndex commonParent = indexes.at(0).parent();

    for (QModelIndex current : indexes) {
        toDelete.append(match(startIndex, ItemFullPath, data(current, ItemFullPath)));
    }
    if (toDelete.count() > 0) {
        while (toDelete.count() > 0) {
            removeRows(toDelete.last().row(), 1, commonParent);
            toDelete.pop_back();
        }
    }
}

TreeItem* TreeModel::getItem(const QModelIndex& index) const
{
    if (index.isValid()) {
        TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
        if (item)
            return item;
    }
    return rootItem;
}

QHash<int, QByteArray> TreeModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[ItemName] = "name";
    roles[ItemOriginalName] = "origname";
    roles[ItemType] = "type";
    roles[ItemFullPath] = "fullpath";
    roles[ItemIsInitiallyExpanded] = "expanded";
    roles[Qt::DecorationRole] = "icon";
    roles[ItemState] = "state";
    roles[ItemDepth] = "depth";
    roles[ItemDbNumber] = "dbnumber";
    roles[ItemFilter] = "filter";
    roles[ItemNamespaceFullpath] = "namespacefullpath";
    roles[ItemConnectionPointer] = "connectionpointer";
    roles[ItemTotalKeys] = "totalkeys";
    roles[ItemLocked] = "locked";
    roles[ItemRemoved] = "removed";
    roles[ItemConnectionMode] = "connection_mode";
    roles[ItemConnectionUuid] = "connection_uuid";

    return roles;
}

int TreeModel::roleId(QByteArray roleName)
{
    return roleNames().key(roleName, false);
}

/**
 *
 * @brief TreeModel::itemData
 * @param index
 * @return
 */
QMap<int, QVariant> TreeModel::itemData(const QModelIndex& index) const
{
    TreeItem* item = getItem(index);
    return item->itemData();
}

const QModelIndex TreeModel::rootIndex()
{
    return QModelIndex();
}

const QVariant TreeModel::roleValue(QModelIndex index, QByteArray roleName)
{
    int role = roleId(roleName);
    if (role != false)
        return data(index, role);
    return 0;
}

void TreeModel::setRoleValue(QModelIndex index, QByteArray roleName, QVariant value)
{
    int role = roleId(roleName);
    if (role != false) {
        setData(index, value, role);
    }
    return;
}

void TreeModel::setupModelConnectionsData(const QList<QSharedPointer<RedisClient::Connection> > connections)
{
    LOG(INFO) << connections.count() << " connections loaded";

    const QModelIndex root = rootIndex();

    if (connections.count() <= 0)
    {
        return;
    }
    // Insert rows (children first); it is like adding EMPTY items at this moment
    // After this, the TreeView will KNOW how many rows WILL show (as soon as there is data, set later)
    insertRows(0, connections.count(), root);

    // Now fill those items with data
    QModelIndex currentIndex;
    for (int i = 0; i < connections.count(); ++i) {
        //RedisClient::ConnectionConfig config = connections[i]->getConfig();
        ServerConfig config = connections[i]->getConfig();
        QMap<int, QVariant> itemData = {
            { ItemName, config.name() },
            { ItemType, "server" },
            { Qt::DecorationRole, "" },
            { ItemConnectionPointer, QVariant::fromValue(connections[i]) },
            { ItemFilter, config.getDefaultFilter() },
            { ItemConnectionMode, (int) connections[i]->mode()},
            { ItemConnectionUuid, config.getUuid()}
        };
        currentIndex = index(i, 0, root);
        setItemData(currentIndex, itemData);
    }
}

void TreeModel::onConnectionUpdated(QSharedPointer<RedisClient::Connection> connection)
{

    QModelIndex connectionIndex = findConnectionIndex(connection);

    const QModelIndex root = rootIndex();

    if (!connectionIndex.isValid()) {
        insertRows(rowCount(root), 1, root);
        connectionIndex = index(rowCount(root) - 1, 0, root);
    }

    //RedisClient::ConnectionConfig config = connection->getConfig();
    ServerConfig config = connection->getConfig();

    QMap<int, QVariant> itemData = {
        { ItemName, config.name() },
        { ItemType, "server" },
        { Qt::DecorationRole, "qrc:/resources/images/icons/db.svg" },
        { ItemConnectionPointer, QVariant::fromValue(connection) },
        { ItemFilter, "" },
        { ItemConnectionMode, (int) connection->mode()},
        { ItemConnectionUuid, config.getUuid()}
    };
    setItemData(connectionIndex, itemData);
}

void TreeModel::onConnectionRemoved(QSharedPointer<RedisClient::Connection> connection)
{
    QModelIndex index = findConnectionIndex(connection);
    if (index.isValid())
        removeRow(index.row(), rootIndex());
}

void TreeModel::onDatabasesLoaded(QModelIndex connectionIndex, RedisClient::DatabaseList databases)
{

    RedisClient::DatabaseList::const_iterator dbIterator = databases.constBegin();

    // First, remove all rows of this connection
    int rCount = rowCount(connectionIndex);
    if (rCount > 0)
        removeRows(0, rowCount(connectionIndex), connectionIndex);

    // Insert rows
    insertRows(0, databases.size(), connectionIndex);

    int i = 0;
    QModelIndex currentIndex;
    while (dbIterator != databases.constEnd()) {
        QMap<int, QVariant> itemData = {
            { ItemName, QString("db%1").arg(dbIterator.key()) },
            { ItemType, "database" },
            { ItemDbNumber, dbIterator.key() },
            { Qt::DecorationRole, "qrc:/resources/images/icons/db.svg" },
            { ItemFilter, data(connectionIndex, ItemFilter) },
            { ItemTotalKeys, dbIterator.value() }
        };
        currentIndex = index(i, 0, connectionIndex);
        setItemData(currentIndex, itemData);

        ++i;
        ++dbIterator;
    }

    emit databasesLoaded(connectionIndex);
}

void TreeModel::onConnectionDisconnected(QModelIndex connectionIndex)
{
    removeRows(0, rowCount(connectionIndex), connectionIndex);
}

void TreeModel::onDbItemsLoaded(const RedisClient::Connection::NamespaceItems& items, const QModelIndex& selectedIndex, const ServerConfig& config)
{

    QVariant dbNumber;
    if (data(selectedIndex, ItemType) == "database") {
        dbNumber = data(selectedIndex, ItemDbNumber);
    } else {
        dbNumber = data(databaseParent(selectedIndex), ItemDbNumber);
    }

    int totalNumberOfKeys = 0;
    QModelIndex currentIndex;
    char separator = config.namespaceSeparator().at(0).toLatin1();
    QList<QByteArray> nsParts;
    QByteArray fullpath;
    int i;

    // Plain Keys, if any (THEY GO BELOW Namespaces, if any)
    if (items.second.count() > 0)
    {
        insertRows(0, items.second.count(), selectedIndex);
        i = 0;
        for (QByteArray fullKey : items.second) {
            nsParts = fullKey.split(separator);
            QMap<int, QVariant> itemData = {
                { ItemName, nsParts.last() },
                { ItemType, "key" },
                { ItemDbNumber, dbNumber },
                { ItemFullPath, fullKey },
                { ItemFilter, "" }

            };
            currentIndex = index(i, 0, selectedIndex);
            setItemData(currentIndex, itemData);
            ++i;
            totalNumberOfKeys++;
        }
    }

    // Namespaces, if any
    if (items.first.count() > 0)
    {
        insertRows(0, items.first.count(), selectedIndex);
        i = 0;
        for (QPair<QByteArray, ulong> ns : items.first) {
            fullpath = ns.first;
            nsParts = fullpath.split(separator);
            QMap<int, QVariant> itemData = {
                { ItemName, nsParts.last() },
                { ItemType, "namespace" },
                { ItemNamespaceFullpath, fullpath },
                { ItemDbNumber, dbNumber },
                { ItemTotalKeys, QVariant::fromValue(ns.second) }
            };

            currentIndex = index(i, 0, selectedIndex);
            setItemData(currentIndex, itemData);
            ++i;
            totalNumberOfKeys += ns.second;
        }
    }
    setData(selectedIndex, totalNumberOfKeys, ItemTotalKeys);

    emit loadKeysFinished(selectedIndex);
}

QModelIndex TreeModel::connectionParent(const QModelIndex& index) const
{
    return typeParent(index, "server");
}

QModelIndex TreeModel::databaseParent(const QModelIndex& index) const
{
    return typeParent(index, "database");
}

QModelIndex TreeModel::namespaceParent(const QModelIndex& index) const
{
    return typeParent(index, "namespace");
}

QModelIndex TreeModel::typeParent(const QModelIndex& index, QString type) const
{

    if (data(index, ItemType) == type) {
        return index;
    }

    QModelIndex p = parent(index);

    if (!p.isValid())
        return QModelIndex();

    if (data(p, ItemType) == type) {
        return p;
    }

    return typeParent(p, type);
}

QModelIndex TreeModel::findConnectionIndex(QSharedPointer<RedisClient::Connection> connection)
{
    QModelIndexList matches = match(index(0, 0), ItemConnectionPointer, QVariant::fromValue(connection), 1, Qt::MatchRecursive);
    if (matches.size() <= 0)
        return QModelIndex();
    return matches.at(0);
}

void TreeModel::lock(const QModelIndex& index)
{
    setData(index, true, ItemLocked);
}

void TreeModel::unlock(const QModelIndex& index)
{
    setData(index, false, ItemLocked);
}

bool TreeModel::isLocked(const QModelIndex& index)
{
    return data(index, ItemLocked).toBool();
}

void TreeModel::setRemoved(const QModelIndex& index)
{
    setData(index, true, ItemRemoved);
}

void TreeModel::setUnRemoved(const QModelIndex& index)
{
    setData(index, false, ItemRemoved);
}

bool TreeModel::isRemoved(const QModelIndex& index)
{
    return data(index, ItemRemoved).toBool();
}

QModelIndex TreeModel::addRowWithData(const QModelIndex& parent, const QJsonObject& item)
{

    insertRows(rowCount(parent), 1, parent);
    QModelIndex newIndex = index(rowCount(parent) - 1, 0, parent);

    QJsonObject::const_iterator i;

    for (i = item.begin(); i != item.end(); i++) {
        setRoleValue(newIndex, i.key().toUtf8(), i.value().toString());
    }

    // Not sure if it required
    emit dataChanged(parent, parent);

    return newIndex;
}

bool TreeModel::isOfType(const QModelIndex &index, const QList<QString> &types) {
    return types.indexOf(data(index, ItemType).toString()) != -1;
}

QString TreeModel::type(const QModelIndex &index) {
    return data(index, ItemType).toString();
}

QVariant TreeModel::getConnection(const QModelIndex &index)
{
    if (isOfType(index, {"server"}))
        return data(index, ItemConnectionPointer);

    if (isOfType(index, {"database", "namespace", "key"})) {
        QModelIndex pIndex = connectionParent(index);
        return data(pIndex, ItemConnectionPointer);
    }

    return QVariant();
}

int TreeModel::getDbNumber(const QModelIndex &index)
{
    if (isOfType(index, {"database", "namespace", "key"})) {
        QModelIndex pIndex = databaseParent(index);
        return data(pIndex, ItemDbNumber).toInt();
    }

    return -1;
}

bool TreeModel::isValid(const QModelIndex &index)
{
    return index.isValid();
}
