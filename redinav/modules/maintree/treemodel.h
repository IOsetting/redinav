#pragma once
#include "roles.h"
#include "treeitem.h"
#include <QAbstractItemModel>
#include <QSharedPointer>
#include <QtCore>
#include "modules/connection/connectionconf.h"
#include <qredisclient/connection.h>

namespace MainTree {

class TreeModel : public QAbstractItemModel {
    Q_OBJECT

public:
    TreeModel(QList<QSharedPointer<RedisClient::Connection> > connections, QObject* parent = 0);
    ~TreeModel();
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant& value, int role = Qt::EditRole) override;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    Q_INVOKABLE int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& /*parent*/) const override;
    bool hasChildren(const QModelIndex& parent = QModelIndex()) const override;
    bool canFetchMore(const QModelIndex& parent) const override;
    void fetchMore(const QModelIndex& parent) override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    bool insertRows(int position, int count, const QModelIndex& parent) override;
    Q_INVOKABLE bool removeRows(int position, int count, const QModelIndex& parent) override;
    QHash<int, QByteArray> roleNames() const override;
    Q_INVOKABLE QMap<int, QVariant> itemData(const QModelIndex& index) const override;
    Q_INVOKABLE const QVariant roleValue(QModelIndex index, QByteArray roleName);
    void setupModelConnectionsData(const QList<QSharedPointer<RedisClient::Connection> > connections);

    Q_INVOKABLE QModelIndex connectionParent(const QModelIndex& index) const;
    Q_INVOKABLE QModelIndex typeParent(const QModelIndex& index, QString type) const;
    Q_INVOKABLE QModelIndex databaseParent(const QModelIndex& index) const;
    Q_INVOKABLE QModelIndex namespaceParent(const QModelIndex& index) const;

    Q_INVOKABLE void setRoleValue(QModelIndex index, QByteArray roleName, QVariant value);

    TreeItem* getItem(const QModelIndex& index) const;

    Q_INVOKABLE void lock(const QModelIndex& index);
    Q_INVOKABLE void unlock(const QModelIndex& index);
    Q_INVOKABLE bool isLocked(const QModelIndex& index);
    Q_INVOKABLE void setRemoved(const QModelIndex& index);
    Q_INVOKABLE void setUnRemoved(const QModelIndex& index);
    Q_INVOKABLE bool isRemoved(const QModelIndex& index);

    Q_INVOKABLE QModelIndex addRowWithData(const QModelIndex& parent, const QJsonObject& item);

    Q_INVOKABLE void removeIndexes(QModelIndexList keysList);


    Q_INVOKABLE bool isOfType(const QModelIndex &index, const QList<QString> &types);
    Q_INVOKABLE QString type(const QModelIndex &index);
    Q_INVOKABLE QVariant getConnection(const QModelIndex &index);
    Q_INVOKABLE int getDbNumber(const QModelIndex &index);
    Q_INVOKABLE bool isValid(const QModelIndex &index);

signals:
    void loadKeysFinished(QModelIndex index);
    void databasesLoaded(QModelIndex index);

public slots:
    void onConnectionUpdated(QSharedPointer<RedisClient::Connection> connection = 0);
    void onConnectionRemoved(QSharedPointer<RedisClient::Connection> connection = 0);
    void onDatabasesLoaded(QModelIndex connectionIndex, RedisClient::DatabaseList databases);
    void onConnectionDisconnected(QModelIndex connectionIndex);
    void onDbItemsLoaded(const RedisClient::Connection::NamespaceItems& items, const QModelIndex& selectedIndex, const ServerConfig& config);

private:
    int roleId(QByteArray roleName);
    const QModelIndex rootIndex();
    TreeItem* rootItem;

    QModelIndex findConnectionIndex(QSharedPointer<RedisClient::Connection> connection);
};
}
