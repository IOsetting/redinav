#pragma once

#include "connection/connectionconf.h"
#include "maintree/treemodel.h"
#include "version.h"
#include <QJSValue>
#include <QModelIndex>
#include <QObject>
#include <QSharedPointer>
#include <connection/configmanager.h>
#include <functional>
#include "license/limiter.h"
#include <keyeditor/basekey.h>
#include <qredisclient/connection.h>

using namespace MainTree;

class Backend : public QObject {
    Q_OBJECT
    ADD_EXCEPTION

public:

    typedef std::function<void(const QList<RedisClient::Connection::NamespaceItems>&, const QString&)> NamespaceItemsListCallback;
    typedef QList<RedisClient::Connection::NamespaceItems> NamespaceItemsList;

    Backend(const QSharedPointer<ConfigManager> configManager, Limiter *limiter);

    ~Backend(void);

    Q_INVOKABLE QSharedPointer<RedisClient::Connection> addNewConnection(ServerConfig &config, bool saveToConfig = true);

    Q_INVOKABLE void updateConnection(ServerConfig config);

    Q_INVOKABLE bool removeConnection(int row);

    Q_INVOKABLE bool importConnections(const QString&);

    Q_INVOKABLE bool saveConnectionsConfigToFile(const QString targetFile="");

    Q_INVOKABLE bool testConnectionSettings(const ServerConfig& config);

    Q_INVOKABLE ServerConfig createEmptyConfig() const;

    Q_INVOKABLE QString cloneConnection(const QModelIndex& index);

    Q_INVOKABLE int size();

    Q_INVOKABLE bool disconnectConnection(QModelIndex index);

    QStringList getConnectionNames();

    QList<QSharedPointer<RedisClient::Connection> > getConnections() const;

    Q_INVOKABLE void loadKeys(const QModelIndex& selectedIndex, const bool invalidateKeysCache = false);

    Q_INVOKABLE MainTree::TreeModel* treeModel();

    Q_INVOKABLE bool connect(QModelIndex connectionIndex);

    void setTreemodel(MainTree::TreeModel* treemodel);

    Q_INVOKABLE ServerConfig getConnectionConfigByIndex(QModelIndex index);
    Q_INVOKABLE void reloadKeys(const QModelIndex& selectedIndex);
    Q_INVOKABLE void flushDb(QModelIndex index);

    Q_INVOKABLE QString humanSize(long size);
    Q_INVOKABLE long binaryStringLength(const QVariant& value);

    Q_INVOKABLE bool isBinaryString(const QVariant& value);

    Q_INVOKABLE void addNewKey(QSharedPointer<RedisClient::Connection> connection,
        int dbNumber, QString type, QString keyName,
        QString v1, QString v2, QJSValue jsCallback);

    Q_INVOKABLE bool keyExists(QSharedPointer<RedisClient::Connection> connection,
        int dbNumber, QString keyName);

    Q_INVOKABLE QString renameKey(QSharedPointer<RedisClient::Connection> connection,
        int dbNumber, QString oldName, QString newName);

    Q_INVOKABLE void deleteNamespaceKeys(
        QSharedPointer<RedisClient::Connection> connection, int dbNumber,
        QString keyNamespace, QString filter, QModelIndex treeModelIndex);

    Q_INVOKABLE void deleteKeysCollectionByKeys(
        QSharedPointer<RedisClient::Connection> connection, int dbNumber,
        QList<QString> keys);
    Q_INVOKABLE void deleteKeysCollectionByIndexes(
        QSharedPointer<RedisClient::Connection> connection, int dbNumber,
        QModelIndexList indexes);
    Q_INVOKABLE QString setTTL(QSharedPointer<RedisClient::Connection> connection,
        int dbNumber, QString keyFullPath, long long ttl);

    Q_INVOKABLE QString getPathFromUrl(const QUrl &url);
    Q_INVOKABLE QString cloneKey(QSharedPointer<RedisClient::Connection> connection, int dbNumber, QString oldName, QString newName);

    Q_INVOKABLE QString getType(QSharedPointer<RedisClient::Connection> connection, int dbNumber, QString keyFullPath);
    Q_INVOKABLE QString phpSerializedPretty(const QString &input);

    Q_INVOKABLE QString toPrintable(QByteArray string);

    Q_INVOKABLE QByteArray printableToBinary(QString string);

    Q_INVOKABLE void copyToClipboard(const QString &text);

    Q_INVOKABLE bool saveContentToFile(const QString targetFile, const QByteArray &content);
    Q_INVOKABLE void resetGlobalSettings();

    Q_INVOKABLE QString dumpKeysToFileByKeyIndexes(QSharedPointer<RedisClient::Connection> connection, int dbNumber, QModelIndexList indexes, QString targetFile, const QString dumpType="json");
    Q_INVOKABLE QString dumpKeysToFileByNamespace(QSharedPointer<RedisClient::Connection> connection, int dbNumber, QString keyNamespace, QString filter, QString targetFile);
    Q_INVOKABLE QString dumpKeysToFileByDatabase(QSharedPointer<RedisClient::Connection> connection, int dbNumber, QString filter, QString targetFile);

    Q_INVOKABLE QString importKeysFromJson(QSharedPointer<RedisClient::Connection> connection, int dbNumber, QString jsonFile, const QModelIndex index = QModelIndex());

    Q_INVOKABLE void cancelKeysDump();

    Q_INVOKABLE void cancelKeysImport();

    Q_INVOKABLE bool isNsOrDbDumpIsRunning();
    Q_INVOKABLE bool isKeysSelectionDumpIsRunning();
    Q_INVOKABLE bool isKeysImportIsRunning();
    Q_INVOKABLE QString getVersion();
    Q_INVOKABLE QString getFileContent(const QString &filePath);


signals:

    void connectionUpdated(
            QSharedPointer<RedisClient::Connection> connection = 0) const;

    void connectionRemoved(
        QSharedPointer<RedisClient::Connection> connection = 0) const;

    void connectionDisconnected(QModelIndex connectionIndex) const;
    void databasesLoaded(QModelIndex connectionIndex,
        RedisClient::DatabaseList databases);

    void dbItemsLoaded(const RedisClient::Connection::NamespaceItems& items,
        const QModelIndex& dbIndex, const ServerConfig& config);

    void openConsole(QSharedPointer<RedisClient::Connection> connection);

    void openServerStats(QSharedPointer<RedisClient::Connection> connection);

    void keysDeleted(const bool result, const QString message,
        QList<QString> keys);
    void keysDeleted(const bool result, const QString message,
        QModelIndexList indexes);

    void namespaceKeysDeletionFinished(bool result, QString message,
        QModelIndex index);

    void connectionFailed();

    void keysDumpFinished(QString message);
    void keysImportFinished(QString message, QModelIndex dbIndex);

    void keysDumpProgress(int current, int total);
    void keysImportProgress(int current, int total);

    void clusterModeNotSupported(const QString message);


public slots:
    void onKeysDumpWatcherFinished();
    void onKeysImportWatcherFinished();
    void onConnectionDisconnected(QModelIndex connectionIndex);
    void onConnectionRemoved(QSharedPointer<RedisClient::Connection> connection);

private:
    QSharedPointer<ConfigManager> m_configManager;
    QString m_configPath;
    QString m_configDir;

    QList<QSharedPointer<RedisClient::Connection> > m_connections;

    void getDatabases(QSharedPointer<RedisClient::Connection> connection,
        std::function<void(RedisClient::DatabaseList)> callback);

    void loadNamespaceItems(QSharedPointer<RedisClient::Connection> connection,
        const QString& filter,
        std::function<void(const QString&)> callback);

    void namespaceReducer(const RedisClient::Connection::RawKeysList& keylist,
        RedisClient::Connection::RawKeysList& reduced,
        int depth);

    TreeModel* m_treemodel;

    QSharedPointer<RedisClient::Connection> getConnectionFromIndex(
        QModelIndex index);

    void registerLogger(const RedisClient::Connection &connection);

    // Watcher for the Dump Select Keys future
    // Only one dump will be allowed at time
    QFutureWatcher<void> m_keysDumpWatcher;

    // Watcher for the Import future
    // Only one import will be allowed at time
    QFutureWatcher<void> m_keysImportWatcher;

    // Flag if namespace or database dumps is in progress (one at a time)
    bool m_DumpRunning = false;


    QJsonDocument readJsonFile(QString jsonFile);

    bool m_keysDumpCancelled;
    bool m_keysImportCancelled;

    void dumpKeysToJsonFileByKeys(QSharedPointer<RedisClient::Connection> connection, int dbNumber, QList<QByteArray> keyNames, QString targetFile);
    void dumpKeysToFileByPattern(QSharedPointer<RedisClient::Connection> connection, int dbNumber, QByteArray pattern, QString targetFile);
    QString buildPatternForNamespace(QSharedPointer<RedisClient::Connection> connection, QString prefix, QString filter);

    Limiter *m_limiter;

    RedisClient::Connection::NamespaceItemsCallback m_wrapper;
    void getClusterNamespaceItems(QSharedPointer<RedisClient::Connection> connection, RedisClient::Connection::NamespaceItemsCallback callback, const QString &pattern);


    void mergeNamespaceItems(RedisClient::Connection::RootNamespaces source, RedisClient::Connection::RootNamespaces &target);

    QModelIndex m_importerDbIndex;
    QModelIndex m_dumperDbIndex;

    QJsonObject loadConfigByUuid(const QString &configFilePath, QString uuid);


    RedisClient::Connection::NamespaceItems convertRawKeysListToNamespaceItems(RedisClient::Connection::RawKeysList keys, QString pattern, ServerConfig config);

    QSharedPointer<QMap<QString, RedisClient::Connection::RawKeysList>> m_preloadedKeys;

    QSharedPointer<QMap <QString, QMap<int, RedisClient::Connection::RawKeysList>>> m_keysCache;




protected:
    bool loadConnectionsConfigFromFile(const QString& configPath, bool saveChangesToFile = false, const bool import=false);
};
