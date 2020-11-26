#include "backend.h"
#include "app.h"
#include "maintree/roles.h"
#include "phpserializedpretty.h"
#include <QAbstractItemModel>
#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QDir>
#include <QDoubleValidator>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QtConcurrent>
#include <asyncfuture.h>
#include <easylogging++.h>
#include <keyeditor/basekey.h>
#include <keyeditor/hashkey.h>
#include <keyeditor/listkey.h>
#include <keyeditor/setkey.h>
#include <keyeditor/stringkey.h>
#include <keyeditor/zsetkey.h>
#include <qredisclient/utils/text.h>
#include <qredisclient/utils/sync.h>
#include <qredisclient/utils/compat.h>
#include <qredisclient/transporters/defaulttransporter.h>
#include "sshtransporter.h"

/**
 * @brief Backend::Backend
 * @param configPath Path to configuration file (JSON)
 */
Backend::Backend(const QSharedPointer<ConfigManager> configManager, Limiter *limiter)
    :
      m_configManager(configManager),
      m_configPath(configManager->getConfigFilepath()),
      m_configDir(configManager->getConfigDir()),
      m_limiter(limiter)

{
    if (!m_configPath.isEmpty() && QFile::exists(m_configPath)) {
        loadConnectionsConfigFromFile(m_configPath);
    }

    // Initialize an empty keys cache
    m_keysCache = QSharedPointer<QMap<QString, QMap<int, RedisClient::Connection::RawKeysList>>> (new QMap<QString, QMap<int, RedisClient::Connection::RawKeysList>>());

    // Connect Dump and Import Future watchers to respective Signals and Slots
    // Doing this once for the whole app run. Only one dump/import is allowed to run at the same time
    QObject::connect(&m_keysImportWatcher, SIGNAL(finished()), this, SLOT(onKeysImportWatcherFinished()));
    QObject::connect(&m_keysDumpWatcher, SIGNAL(finished()), this, SLOT(onKeysDumpWatcherFinished()));

    QObject::connect(this,SIGNAL(connectionRemoved(QSharedPointer<RedisClient::Connection>)),
                     this, SLOT(onConnectionRemoved(QSharedPointer<RedisClient::Connection>)));

    QObject::connect(this,SIGNAL(connectionDisconnected(QModelIndex)),
                     this, SLOT(onConnectionDisconnected(QModelIndex)));


}

Backend::~Backend(void)
{
}

/**
 * @brief Backend::addNewConnection
 * @param config Server/Connection config
 * @param saveToConfig (default = true)
 * @return
 */
QSharedPointer<RedisClient::Connection> Backend::addNewConnection(ServerConfig& config, bool saveToConfig)
{
    QSharedPointer<RedisClient::Connection> connection(new RedisClient::Connection(config));
    config.setOwner(connection.toWeakRef());
    connection->setConnectionConfig(config);
    m_connections.push_back(connection);

    registerLogger(*connection);

    if (saveToConfig) {
        saveConnectionsConfigToFile();
    }

    return connection;
}

/**
 * Update (or add new) connection
 *
 * @brief Backend::updateConnection
 * @param config
 */
void Backend::updateConnection(ServerConfig config)
{
    QSharedPointer<RedisClient::Connection> connection;

    config.saveSecurityContentItemsToFiles(m_configManager);

    QString uuid;

    if (config.getUuid().isEmpty()) {
        uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);
        config.setUuid(uuid);
        connection = addNewConnection(config, false);
    } else {
        connection = config.getOwner().toStrongRef();
        connection->setConnectionConfig(config);
    }

    saveConnectionsConfigToFile();

    // Contact Main Tree Model
    emit connectionUpdated(connection);
}

bool Backend::importConnections(const QString& configPath)
{
    if (loadConnectionsConfigFromFile(configPath, true, true)) {
        return true;
    }
    return false;
}

bool Backend::loadConnectionsConfigFromFile(const QString& configPath, bool saveChangesToFile, const bool import)
{
    LOG(INFO) << "Loading connections from configuration file";
    LOG(INFO) << configPath;

    m_connections.clear();

    QJsonArray serverConfigsArray;

    QFile confFile(configPath);

    if (!confFile.open(QIODevice::ReadOnly))
        return false;

    QByteArray data = confFile.readAll();
    confFile.close();

    QJsonDocument jsonConfig = QJsonDocument::fromJson(data);

    if (jsonConfig.isEmpty())
        return true;

    if (!jsonConfig.isArray()) {
        return false;
    }

    serverConfigsArray = jsonConfig.array();

    QJsonObject o;
    QString uuid;
    bool needStructureUpgrade = false;
    for (QJsonValue configJson : serverConfigsArray) {
        if (!configJson.isObject())
            continue;

        o = configJson.toObject();
        if (!o.contains("ssh"))
        {
            needStructureUpgrade = true;
            if (!o.contains("ssh_host"))
            {
                o.insert("ssh_host", "");
            }
            o.insert("ssh", (bool) (o.value("ssh_host") != ""));
        }
        if (!o.contains("uuid"))
        {
            uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);
            needStructureUpgrade = true;
            o.insert("uuid", uuid);
        }
        else
        {
            uuid = o.value("uuid").toString();
        }

        if (o.value("port").toInt() <= 0)
        {
            o.insert("port", (int) 6379);
            needStructureUpgrade = true;
        }


        if (o.value("host").toString().isEmpty())
        {
            o.insert("host", "localhost");
            needStructureUpgrade = true;
        }

        ServerConfig config(QJsonObjectToVariantHash(o));

        if (config.isNull())
            continue;

        addNewConnection(config, false);
    }


    if (needStructureUpgrade)
    {
        LOG(INFO) << "Old configuration format detected - missing 'ssh' parameter: upgrading and saving back";
    }

    if (saveChangesToFile || needStructureUpgrade) {
        saveConnectionsConfigToFile();
    }

    if (import) {
        m_treemodel->removeRows(0, m_treemodel->rowCount(), QModelIndex());
        m_treemodel->setupModelConnectionsData(m_connections);
    }

    return true;
}


QJsonObject Backend::loadConfigByUuid(const QString& configFilePath, QString uuid)
{
    Q_UNUSED(uuid);

    QJsonObject result;
    QJsonArray serverConfigsArray;
    QFile confFile(configFilePath);
    if (!confFile.open(QIODevice::ReadOnly))
        return result;

    QByteArray data = confFile.readAll();
    confFile.close();
    QJsonDocument jsonConfig = QJsonDocument::fromJson(data);
    //qDebug() << jsonConfig;

    return result;
}


bool Backend::saveConnectionsConfigToFile(const QString targetFile)
{
    ConfigManager cm = ConfigManager();

    QJsonArray connections;
    for (auto c : m_connections) {
        QJsonObject cfg = c->getConfig().toJsonObject();

        // Resolve orgiinal host/port issue: if they are availabloe, use them to save
        if (cfg.contains("original_host"))
        {
            cfg.insert("host", cfg.value("original_host").toString());
        }
        cfg.remove("original_host");

        if (cfg.contains("original_port"))
        {
            cfg.insert("port", cfg.value("original_port").toInt());
        }
        cfg.remove("original_port");

        connections.push_back(QJsonValue(cfg));
    }

    return cm.saveConnectionsToFile(connections, targetFile);
}


bool Backend::saveContentToFile(const QString targetFile, const QByteArray &content)
{
    QFile aFile(targetFile);

    if (aFile.open(QIODevice::WriteOnly)) {
        QTextStream outStream(&aFile);
        outStream.setCodec("UTF-8");
        outStream << content;
        aFile.close();
    }

    return true;
}


bool Backend::testConnectionSettings(const ServerConfig& config)
{
    RedisClient::Connection testConnection(config);

    try {
        return testConnection.connect();
    } catch (const RedisClient::Connection::Exception&) {
        return false;
    }
}

ServerConfig
Backend::createEmptyConfig() const
{
    QSettings settings;
    ServerConfig config = ServerConfig();
    int timeout = settings.value("app/defaultConnectionTimeout", -1).toInt();

    if (timeout >= 0)
    {
        config.setConnectionTimeout(timeout*1000);
    }

    return config;
}

QString Backend::cloneConnection(const QModelIndex &index)
{
    QSharedPointer<RedisClient::Connection> connection = getConnectionFromIndex(index);
    ServerConfig config = connection->getConfig();
    config.setName(QString("%1-copy").arg(config.name()));
    config.setUuid("");
    updateConnection(config);
    return "";
}

ServerConfig
Backend::getConnectionConfigByIndex(QModelIndex index)
{
    QSharedPointer<RedisClient::Connection> connection = getConnectionFromIndex(index);

    if (!connection.isNull()) {
        ServerConfig conf = connection->getConfig();
        return conf;
    }
    return createEmptyConfig();
}

int Backend::size()
{
    return m_connections.length();
}

QStringList
Backend::getConnectionNames()
{
    QStringList result;

    for (QSharedPointer<RedisClient::Connection> c : m_connections) {
        result.append(c->getConfig().name());
    }

    return result;
}

QList<QSharedPointer<RedisClient::Connection> >
Backend::getConnections() const
{
    return m_connections;
}

bool Backend::removeConnection(int row)
{
    QSharedPointer<RedisClient::Connection> connection = m_connections.at(row);
    connection->disconnect();
    m_connections.removeAt(row);
    saveConnectionsConfigToFile();
    emit connectionRemoved(connection);
    return true;
}

bool Backend::disconnectConnection(QModelIndex index)
{
    QSharedPointer<RedisClient::Connection> connection = getConnectionFromIndex(index);
    connection->disconnect();
    emit connectionDisconnected(index);
    return true;
}

bool Backend::connect(QModelIndex connectionIndex)
{
    QSharedPointer<RedisClient::Connection> connection = getConnectionFromIndex(connectionIndex);

    LOG(INFO) << "Connecting: " << connection->getConfig().name() << " ("
              << connection->getConfig().host() << ")";

    if (m_treemodel->isLocked(connectionIndex)) {
        LOG(INFO) << "Item is locked!!";
        return false;
    }

    m_treemodel->lock(connectionIndex);

    try {
        bool connected = connection->isConnected();
        if (!connected) {
            try {
                connected = connection->connect(true);
            } catch (const RedisClient::Connection::Exception& e) {
                throw RedisClient::Connection::Exception(
                    QObject::tr("Connection error: ") + QString(e.what()));
            }
        }

        if (!connected) {
            throw RedisClient::Connection::Exception(
                QObject::tr("Cannot connect to server '%1'. Check log for details.")
                    .arg(connection->getConfig().name()));
        }

        std::function<void(RedisClient::DatabaseList)> callback =
            [this, connectionIndex, connection](RedisClient::DatabaseList databases) {
                if (databases.size() == 0) {
                    m_treemodel->unlock(connectionIndex);
                    return;
                }
                m_treemodel->setRoleValue(connectionIndex, "connection_mode", (int) connection->mode());
                emit databasesLoaded(connectionIndex, databases);
                m_treemodel->unlock(connectionIndex);
            };
        getDatabases(connection, callback);
        return true;

    } catch (const RedisClient::Connection::Exception& e) {
        m_treemodel->unlock(connectionIndex);
        LOG(ERROR) << QString(e.what());
        return false;
    }
}

void Backend::getDatabases(QSharedPointer<RedisClient::Connection> connection,
    std::function<void(RedisClient::DatabaseList)> callback)
{
    bool connected = connection->isConnected();
    if (!connected)
        return;

    RedisClient::DatabaseList availableDatabeses = connection->getKeyspaceInfo();

    if (connection->mode() != RedisClient::Connection::Mode::Cluster) {
        RedisClient::Response scanningResp;
        int dbNumber = (availableDatabeses.size() == 0) ? 0 : availableDatabeses.lastKey() + 1;

        while (true) {
            scanningResp = connection->commandSync("select", QString::number(dbNumber));
            if (!scanningResp.isOkMessage()) {
                break;
            }
            availableDatabeses.insert(dbNumber, 0);
            ++dbNumber;
        }
    }
    return callback(availableDatabeses);
}

void Backend::setTreemodel(TreeModel* treemodel)
{
    m_treemodel = treemodel;
}

TreeModel*
Backend::treeModel()
{
    return m_treemodel;
}

void Backend::loadKeys(const QModelIndex& selectedIndex, const bool invalidateKeysCache)
{
    m_treemodel->lock(selectedIndex);

    int dbNumber;
    QString filter;
    QString pattern = "";
    QModelIndex connectionIndex = treeModel()->connectionParent(selectedIndex);
    QString connectionFilter = treeModel()->data(selectedIndex, ItemFilter).value<QString>();
    QSharedPointer<RedisClient::Connection> connection = getConnectionFromIndex(connectionIndex);
    ServerConfig config = connection->getConfig();

    if (treeModel()->data(selectedIndex, ItemType) == "database") {
        dbNumber = treeModel()->data(selectedIndex, ItemDbNumber).value<int>();
        filter = treeModel()->data(selectedIndex, ItemFilter).value<QString>();
        filter = filter.isEmpty() ? connectionFilter : filter;
        pattern = !filter.isEmpty() ? "*" + filter + "*" : "*";
    } else if (treeModel()->data(selectedIndex, ItemType) == "namespace") {
        QModelIndex dbIndex = treeModel()->databaseParent(selectedIndex);
        dbNumber = treeModel()->data(dbIndex, ItemDbNumber).value<int>();
        filter = treeModel()->data(dbIndex, ItemFilter).value<QString>();
        filter = filter.isEmpty() ? connectionFilter : filter;
        QString namespaceFullPath = treeModel()->data(selectedIndex, ItemNamespaceFullpath).value<QString>();
        pattern = buildPatternForNamespace(connection, namespaceFullPath, filter);
    } else {
        m_treemodel->unlock(selectedIndex);
        return;
    }


    if (connection->mode() == RedisClient::Connection::Mode::Normal)
    {
        auto callback = [this, selectedIndex, config](
                const RedisClient::Connection::NamespaceItems& items, const QString& err) {
            if (!err.isEmpty()) {
                LOG(ERROR) << "ERROR while collecting namespaces and keys: " << err;
                m_treemodel->unlock(selectedIndex);
            } else {
                //qDebug() << items;
                emit dbItemsLoaded(items, selectedIndex, config);
                m_treemodel->unlock(selectedIndex);
            }
        };

        auto processCallback =  [this, selectedIndex, config, pattern, dbNumber](const RedisClient::Connection::RawKeysList& keys, const QString& err) {
            if (!err.isEmpty()) {
                LOG(ERROR) << "ERROR while collecting namespaces and keys: " << err;
                m_treemodel->unlock(selectedIndex);
            } else {
                QMap<int, RedisClient::Connection::RawKeysList> connCache = m_keysCache->value(config.getUuid());
                connCache.insert(dbNumber, keys);
                m_keysCache->insert(config.getUuid(), connCache);
                RedisClient::Connection::NamespaceItems nsItems = convertRawKeysListToNamespaceItems(keys, pattern, config);
                //qDebug() << nsItems;
                emit dbItemsLoaded(nsItems, selectedIndex, config);
                m_treemodel->unlock(selectedIndex);
            }
        };

        if (!config.luaKeysLoading())
        {
            qDebug() << "Client side database scanning";
            if ( !m_keysCache->contains(config.getUuid()) || !m_keysCache->value(config.getUuid()).contains(dbNumber) || invalidateKeysCache)
            {
                qDebug() << "Preloading and Caching keys";
                connection->getDatabaseKeys(processCallback, pattern, dbNumber);
            }
            else
            {
                qDebug() << "Use cached keys";
                AsyncFuture::observe(QtConcurrent::run([this, pattern, config, dbNumber]() {
                    return convertRawKeysListToNamespaceItems(m_keysCache->value(config.getUuid()).value(dbNumber), pattern, config);
                })
                ).subscribe([selectedIndex, config, this](QFuture<RedisClient::Connection::NamespaceItems> worker) {
                    emit dbItemsLoaded(worker.result(), selectedIndex, config);
                    m_treemodel->unlock(selectedIndex);
                });

            }
        }
        else
        {
            qDebug() << "Server side database scanning";
            connection->getNamespaceItems(callback, config.namespaceSeparator(), pattern, dbNumber);
        }

    }
    else if (connection->mode() == RedisClient::Connection::Mode::Cluster)
    {
        auto clusterCallback = [this, selectedIndex, config, connection] (const RedisClient::Connection::NamespaceItems& items, const QString& err) {
            if (!err.isEmpty()) {
                LOG(ERROR) << "ERROR while collecting namespaces and keys: " << err;
                m_treemodel->unlock(selectedIndex);
            } else {
                emit dbItemsLoaded(items, selectedIndex, config);
                m_treemodel->unlock(selectedIndex);
            }

        };
        getClusterNamespaceItems(connection, clusterCallback, pattern);
    }

}


void Backend::getClusterNamespaceItems(QSharedPointer<RedisClient::Connection> connection, RedisClient::Connection::NamespaceItemsCallback callback, const QString &pattern)
{
     if (connection->mode() != RedisClient::Connection::Mode::Cluster) {
        throw Exception("Connection is not in cluster mode");
    }

    QSharedPointer<RedisClient::Connection::NamespaceItems> result(new RedisClient::Connection::NamespaceItems());
    QSharedPointer<RedisClient::Connection::HostList> masterNodes(new RedisClient::Connection::HostList(connection->getMasterNodes()));
    ServerConfig config = connection->getConfig();

    std::function<bool(void)> connectToNextNode = [this, masterNodes, callback, config, connection]() -> bool {
        if (masterNodes->size() > 0) {
            RedisClient::Connection::Host h = masterNodes->first();
            masterNodes->removeFirst();

            RedisClient::SignalWaiter waiter(config.connectionTimeout());
            waiter.addSuccessSignal(connection->getTransporter().data(), &RedisClient::AbstractTransporter::connected);
            waiter.addAbortSignal(connection->getTransporter().data(), &RedisClient::AbstractTransporter::errorOccurred);

            if (config.overrideClusterHost()) {
                connection->reconnectTo(h.first, h.second);
            } else {
                connection->reconnectTo(config.host(), h.second);
            }

            return waiter.wait();
        } else {
            return false;
        }
    };

    m_wrapper= [this, result, callback, pattern, connectToNextNode, masterNodes, config, connection](const RedisClient::Connection::NamespaceItems &res, const QString& err){
        if (!err.isEmpty()) {
            qDebug() << "Error in cluster keys retrival:" << err;
            return callback(RedisClient::Connection::NamespaceItems(), err);
        }

        // List  of Namespaces:  QList<QPair<QString, int>>
        // Merge newly incoming namespace items/info into the so far collected (Namespace title, Items number accumulated)
        mergeNamespaceItems(res.first, result->first);

        // List of Keys: QList<QString>
        result->second.append(res.second);

        if (masterNodes->size() == 0)
            return callback(*result, QString());

        if (connectToNextNode()) {
            connection->getNamespaceItems(m_wrapper, config.namespaceSeparator(), pattern, 0);
        } else {
            return callback(*result, QString("Cannot connect to cluster node %1:%2").arg(config.host()).arg(config.port()));
        }
    };

    if (connectToNextNode()) {
        connection->getNamespaceItems(m_wrapper, config.namespaceSeparator(), pattern, 0);
    } else {
        return callback(*result, QString("Cannot connect to cluster node %1:%2").arg(config.host()).arg(config.port()));
    }
}


void Backend::mergeNamespaceItems(RedisClient::Connection::RootNamespaces source, RedisClient::Connection::RootNamespaces& target)
{
    RedisClient::Connection::RootNamespaces x;

    RedisClient::Connection::RootNamespaces::iterator source_it;
    RedisClient::Connection::RootNamespaces::iterator target_it;

    bool found = false;

    for (source_it = source.begin(); source_it != source.end(); ++source_it)
    {
        found = false;
        for (target_it = target.begin(); target_it != target.end(); ++target_it)
        {
            if (source_it->first == target_it->first)
            {
                target_it->second += source_it->second;
                found = true;
            }
        }
        if (!found)
        {
            target.append(QPair<QByteArray, ulong>(source_it->first, source_it->second));
        }
    }
}


void Backend::reloadKeys(const QModelIndex& selectedIndex)
{
    MainTree::TreeModel* model = treeModel();
    model->removeRows(0, model->rowCount(selectedIndex), selectedIndex);
    loadKeys(selectedIndex, true);
}

QSharedPointer<RedisClient::Connection>
Backend::getConnectionFromIndex(QModelIndex index)
{
    return treeModel()
        ->data(index, ItemConnectionPointer)
        .value<QSharedPointer<RedisClient::Connection> >();
}

void Backend::flushDb(QModelIndex index)
{
    try {
        m_treemodel->lock(index);
        QModelIndex connectionIndex = treeModel()->connectionParent(index);
        QSharedPointer<RedisClient::Connection> connection = getConnectionFromIndex(connectionIndex);
        ServerConfig config = connection->getConfig();
        int dbNumber = treeModel()->data(index, ItemDbNumber).value<int>();

        std::function<void(const QString&)> callback = [this, connection, index, config](const QString& err) -> void {
            if (!err.isEmpty()) {
                LOG(ERROR) << "Error while flushing database: " << err;
            } else {
                reloadKeys(index);
            }
            m_treemodel->unlock(index);
        };
        connection->flushDbKeys(dbNumber, callback);
    } catch (const RedisClient::Connection::Exception& e) {
        LOG(ERROR) << "Flush DB error: " << e.what();
        m_treemodel->unlock(index);
    }
}

QString
Backend::humanSize(long size)
{
    double num = size;
    QStringList list;
    list << "KB"
         << "MB"
         << "GB";

    QStringListIterator i(list);
    QString unit("bytes");

    while (num >= 1024.0 && i.hasNext()) {
        unit = i.next();
        num /= 1024.0;
    }
    return QString().setNum(num, 'f', 2) + " " + unit;
}

long Backend::binaryStringLength(const QVariant& value)
{
    if (!value.canConvert(QVariant::ByteArray)) {
        return -1;
    }
    QByteArray val = value.toByteArray();
    return val.size();
}

void Backend::deleteKeysCollectionByKeys(
    QSharedPointer<RedisClient::Connection> connection, int dbNumber,
    QList<QString> keys)
{
    QList<QByteArray> rawCmd{ "DEL" };
    for (QString k : keys) {
        rawCmd.append(k.toUtf8());
    }

    connection->command(
        rawCmd, this,
        [this, keys](RedisClient::Response response, QString err) {
            QString message = err;
            if (response.isErrorMessage()) {
                message = "Could not delete keys";
            }
            emit keysDeleted(message.isEmpty(), message, keys);

        },
        dbNumber);
}

void Backend::deleteKeysCollectionByIndexes(
    QSharedPointer<RedisClient::Connection> connection, int dbNumber,
    QModelIndexList indexes)
{
    QList<QByteArray> rawCmd{ "DEL" };
    for (QModelIndex index : indexes) {
        rawCmd.append(m_treemodel->data(index, ItemFullPath).toByteArray());
    }

    try {
        connection->command(
            rawCmd, this,
            [this, indexes](RedisClient::Response response, QString err) {
                QString message = err;
                if (response.isErrorMessage()) {
                    message = "Could not delete keys";
                }
                emit keysDeleted(message.isEmpty(), message, indexes);

            },
            dbNumber);
    } catch (const RedisClient::Connection::Exception& e) {
        emit keysDeleted(false, QString(e.what()), QModelIndexList());
    }
}

bool Backend::isBinaryString(const QVariant& value)
{
    if (!value.canConvert(QVariant::ByteArray)) {
        return false;
    }
    QByteArray val = value.toByteArray();
    return isBinary(val);
}

void Backend::addNewKey(QSharedPointer<RedisClient::Connection> connection,
    int dbNumber, QString type, QString keyName, QString v1,
    QString v2, QJSValue jsCallback)
{
    bool exists = keyExists(connection, dbNumber, keyName);
    QString message;

    if (!exists) {
        auto key = BaseKey::factory(connection, dbNumber, keyName.toUtf8(), type);
        message = key->addKey(v1, v2);
    } else {
        message = "Key already exists";
    }

    if (!message.isEmpty())
        LOG(ERROR) << message;

    if (jsCallback.isCallable()) {
        jsCallback.call({ message.isEmpty(), message });
    }
}

bool Backend::keyExists(QSharedPointer<RedisClient::Connection> connection,
    int dbNumber, QString keyName)
{
    RedisClient::Response result = connection->commandSync({ "EXISTS", keyName.toUtf8() }, dbNumber);
    return result.getValue().toInt() > 0;
}

QString Backend::renameKey(QSharedPointer<RedisClient::Connection> connection,
    int dbNumber, QString oldName, QString newName)
{
    QString message;

    if (keyExists(connection, dbNumber, newName)) {
        message = "Key with this name already exists";
    } else if (oldName != newName) {
        RedisClient::Response result = connection->commandSync(
            { "RENAMENX", oldName.toUtf8(), newName.toUtf8() }, dbNumber);
        if (result.isErrorMessage()) {
            message = "Something went wrong while renaming the key";
        }
    }
    if (!message.isEmpty()) {
        LOG(ERROR) << message;
    }

    return message;
}

void Backend::deleteNamespaceKeys(QSharedPointer<RedisClient::Connection> connection,
    int dbNumber, QString keyNamespace, QString filter,
    QModelIndex treeModelIndex)
{

    m_treemodel->lock(treeModelIndex);

    QString pattern("");
    pattern = buildPatternForNamespace(connection, keyNamespace, filter);

    if (pattern == "") {
        LOG(ERROR) << "Empty pattern when deleting Namespace. Not possible and dangerous! Aborted!";
        m_treemodel->unlock(treeModelIndex);
    }

    connection->getDatabaseKeys(
        [this, connection, dbNumber, treeModelIndex](
            const RedisClient::Connection::RawKeysList& keys, const QString& err) {
            if (!err.isEmpty()) {
                m_treemodel->unlock(treeModelIndex);
                emit namespaceKeysDeletionFinished(false, err, treeModelIndex);
                return;
            }

            QList<QByteArray> rawCmd{ "DEL" };
            for (QString k : keys) {
                rawCmd.append(k.toUtf8());
            }
            connection->command(
                rawCmd, this,
                [this, treeModelIndex](RedisClient::Response response, QString err) {
                    QString message = err;
                    if (response.isErrorMessage()) {
                        message = "Error response during operation";
                    }
                    m_treemodel->unlock(treeModelIndex);
                    emit namespaceKeysDeletionFinished(message.isEmpty(), message,
                        treeModelIndex);
                },
                dbNumber);

        },
        pattern, dbNumber);
}

QString
Backend::setTTL(QSharedPointer<RedisClient::Connection> connection,
    int dbNumber, QString keyFullPath, long long ttl)
{
    RedisClient::Response result;
    QString message;

    try {
        if (ttl >= 0)
            result = connection->commandSync(
                { "EXPIRE", keyFullPath.toUtf8(), QString::number(ttl).toLatin1() },
                dbNumber);
        else
            result = connection->commandSync({ "PERSIST", keyFullPath.toUtf8() }, dbNumber);

        if (result.isErrorMessage()) {
            message = "Could not set TTL";
        }

    } catch (const RedisClient::Connection::Exception& e) {
        message = "Connection error: " + QString(e.what());
    }

    return message;
}


void Backend::registerLogger(const RedisClient::Connection& connection)
{

    QSettings settings;
    bool verboseConnectionLog = settings.value("app/verboseConnectionLog", false).toBool();

    if (verboseConnectionLog) {
        QObject::connect(&connection, &RedisClient::Connection::log, this, [this](const QString& info){
            QString msg = QString("Connection: %1").arg(info);
            LOG(INFO) << msg.toStdString();
        });
    }

    QObject::connect(&connection, &RedisClient::Connection::error, this, [this](const QString& error){
        QString msg = QString("Connection: %1").arg(error);
        LOG(ERROR) << msg.toStdString();
    });
}


QString Backend::getPathFromUrl(const QUrl &url)
{
    return url.isLocalFile() ? url.toLocalFile() : url.path();
}

QString Backend::cloneKey(QSharedPointer<RedisClient::Connection> connection,
    int dbNumber, QString oldName, QString newName)
{
    QString message;

    try {
        if (keyExists(connection, dbNumber, newName)) {
            message = "Key with this name already exists";
        } else if (oldName != newName) {

            if (!keyExists(connection, dbNumber, oldName)) {
                message = "Key does not exist anymore";
            }
            else {
                QFile script(":/resources/lua/copy_key.lua");
                if (!script.open(QIODevice::ReadOnly)) {
                    message = "Cannot open LUA resource";
                }
                else
                {
                    QByteArray LUA_SCRIPT = script.readAll();
                    // "2" means 2 KEYS in command line (oldName  newName)
                    RedisClient::Response result = connection->commandSync({"eval", LUA_SCRIPT, "2", oldName.toUtf8(), newName.toUtf8(), "NX"}, dbNumber);
                    if (result.isErrorMessage())
                    {
                        message = "Could not copy a key";
                    }
                }
            }
        }
    } catch (const RedisClient::Connection::Exception& e) {
        message = "Redis connection error";
        LOG(ERROR) << e.what();
    }


    if (!message.isEmpty()) {
        LOG(ERROR) << message;
    }
    return message;
}


QString Backend::getType(QSharedPointer<RedisClient::Connection> connection, int dbNumber, QString keyFullPath) {

    RedisClient::Response result = connection->commandSync({"TYPE", keyFullPath.toUtf8()}, dbNumber);
    if (result.isErrorMessage()) {
        return "";
    }
    return result.getValue().toString();

}


QString Backend::phpSerializedPretty(const QString &input) {
    return PhpSerializedPretty::pretty(input);
}

QString Backend::toPrintable(QByteArray string)
{
    return printableString(string);
}


QByteArray Backend::printableToBinary(QString string)
{
    return printableStringToBinary(string);
}

void Backend::copyToClipboard(const QString &text)
{
    QClipboard* cb = QApplication::clipboard();

    if (!cb)
        return;

    cb->clear();
    cb->setText(text);

}

void Backend::resetGlobalSettings() {
    QSettings settings;
    settings.clear();
    Application::setDefaultGlobalSettings();
}

using namespace QtConcurrent;

QString Backend::dumpKeysToFileByKeyIndexes(QSharedPointer<RedisClient::Connection> connection, int dbNumber, QModelIndexList indexes, QString targetFile, const QString dumpType)
{

    if (indexes.length() <= 0) {
        return "Empty list of keys";
    }

    QString message;

    // Use a separate connection for this "fututre run"
    ServerConfig config = connection->getConfig();
    QSharedPointer<RedisClient::Connection> tmp_connection(new RedisClient::Connection(config));
    ServerConfig conf = config;
    conf.setOwner(tmp_connection.toWeakRef());
    tmp_connection->setConnectionConfig(conf);
    tmp_connection->connect(true);

    QList<QByteArray> keyNames;
    for (QModelIndex index: indexes)
    {
        keyNames.push_back(index.data(ItemFullPath).toByteArray());
    }

    LOG(INFO) << "Keys information collected, dump started!";

    QFuture<void> future = run([this, tmp_connection, dbNumber, keyNames, targetFile, dumpType]() {
        try {
            dumpKeysToJsonFileByKeys(tmp_connection, dbNumber, keyNames, targetFile);
        } catch (const RedisClient::Connection::Exception& e) {
            LOG(ERROR) << "Connection error:" + QString(e.what());
        } catch (...)
        {
            LOG(ERROR) << "Unable to dump keys (unknown error)";
        }
    });

    if (!m_keysDumpWatcher.isRunning())
    {
        m_keysDumpWatcher.setFuture(future);
    }
    else {
        message = "Another dump is already running. Please try later!";
    }

    return message;
}

QString Backend::dumpKeysToFileByDatabase(QSharedPointer<RedisClient::Connection> connection, int dbNumber, QString filter, QString targetFile)
{
    if (m_DumpRunning)
    {
        return "Another dump is already running. Please try later!";
    }

    QByteArray pattern = !filter.isEmpty() ? "*" + filter.toUtf8() + "*" : "*";
    dumpKeysToFileByPattern(connection, dbNumber, pattern, targetFile);

    return "";

}

QString Backend::dumpKeysToFileByNamespace(QSharedPointer<RedisClient::Connection> connection, int dbNumber, QString keyNamespace, QString filter, QString targetFile)
{

    if (m_DumpRunning)
    {
        return "Another dump is already running. Please try later!";
    }

    if (keyNamespace.isEmpty())
    {
        return "Please select a namespace (prefix)";
    }

    ServerConfig config = connection->getConfig();
    QString separator = config.namespaceSeparator();
    QByteArray pattern = keyNamespace.toUtf8() + separator.toUtf8() + (!filter.isEmpty() ? "*" + filter.toUtf8() + "*" : "*");

    dumpKeysToFileByPattern(connection, dbNumber, pattern, targetFile);

    return "";

}

/**
 * Dump keys to JSON file by pattern
 * @brief Backend::dumpKeysToFileByPattern
 * @param connection
 * @param dbNumber
 * @param pattern
 * @param targetFile
 */
void Backend::dumpKeysToFileByPattern(QSharedPointer<RedisClient::Connection> connection, int dbNumber, QByteArray pattern, QString targetFile)
{

    try {
        m_DumpRunning = true;

        ServerConfig config = connection->getConfig();
        QSharedPointer<RedisClient::Connection> tmp_connection(new RedisClient::Connection(config));
        ServerConfig conf = config;
        conf.setOwner(tmp_connection.toWeakRef());
        tmp_connection->setConnectionConfig(conf);
        tmp_connection->connect(true);

        connection->getDatabaseKeys([this, tmp_connection, dbNumber, targetFile](const RedisClient::Connection::RawKeysList& keys, const QString& err) {
            if (!err.isEmpty()) {
                m_DumpRunning = false;
                LOG(ERROR) << err;
                return;
            }
            dumpKeysToJsonFileByKeys(tmp_connection, dbNumber, keys, targetFile);
            LOG(INFO) << (m_keysDumpCancelled ? "Dump cancelled: possible partial data has been collected" : "Dump finished");
            emit keysDumpFinished("");
            m_DumpRunning = false;
        },
        pattern, dbNumber);
    }
    catch (const RedisClient::Connection::Exception& e)
    {
        m_DumpRunning = false;
        LOG(ERROR) << "Dump failed: " << e.what();
    }
    catch (...)
    {
        m_DumpRunning = false;
        LOG(ERROR) << "Unknown and unexpected exception occured while dumping key namespace!";
    }

}

/**
 * Namespace/Database key dump process is running?
 * @brief Backend::isNsOrDbDumpIsRunning
 * @return
 */
bool Backend::isNsOrDbDumpIsRunning() {
    return m_DumpRunning;
}

/**
 * Keys selection dump process is running?
 * @brief Backend::isKyesSelectionDumpIsRunning
 * @return
 */
bool Backend::isKeysSelectionDumpIsRunning() {
    return m_keysDumpWatcher.isRunning();
}

/**
 * Keys import process is running?
 * @brief Backend::isKyesImportIsRunning
 * @return
 */
bool Backend::isKeysImportIsRunning() {
    return m_keysImportWatcher.isRunning();
}

void Backend::dumpKeysToJsonFileByKeys(QSharedPointer<RedisClient::Connection> connection, int dbNumber, QList<QByteArray> keyNames, QString targetFile)
{
    if (keyNames.length() <= 0) {
        return;
    }

    QFile file(targetFile);

    LOG(INFO) << "Dump file: " << targetFile;

    if (file.open(QIODevice::WriteOnly)) {

        QJsonObject allKeysObject = QJsonObject();
        QJsonObject keyData;
        QTextStream stream(&file);
        RedisClient::Response ttlResponse;
        int ttl;
        int current = 0;
        int total = keyNames.count();

        m_keysDumpCancelled = false;

        for (QByteArray keyName : keyNames)
        {
            current++;
            emit keysDumpProgress(current, total);
            if (m_keysDumpCancelled) {
                break;
            }

            try
            {
                auto key = BaseKey::factory(connection, dbNumber, keyName, getType(connection, dbNumber, keyName));
                keyData = key->getKeyAsJsonObject();
                ttlResponse = connection->commandSync({ "TTL", keyName }, dbNumber);
                ttl = ttlResponse.getValue().toInt();
                if (ttl != -1) {
                    keyData.insert("ttl", ttlResponse.getValue().toString());
                }
                allKeysObject.insert(keyName, keyData);
            }
            catch (const RedisClient::Connection::Exception& e)
            {
                LOG(ERROR) << e.what();
                break;
            }
            catch (...)
            {
                LOG(ERROR) << "Unknown exception";
                break;
            }
        }

        QJsonDocument doc(allKeysObject);
        QByteArray bytes = doc.toJson();
        stream << bytes;
        stream.flush();
        file.flush();
        file.close();
    }
}



QJsonDocument Backend::readJsonFile(QString jsonFile)
{

    QString val;
    QFile file;
    file.setFileName(jsonFile);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    val = file.readAll();
    file.close();

    QJsonDocument d = QJsonDocument::fromJson(val.toUtf8());

    return d;

}

QString Backend::importKeysFromJson(QSharedPointer<RedisClient::Connection> connection, int dbNumber, QString jsonFile, QModelIndex index)
{

    QString message = "";

    // Load JSON file into JSON Document
    QJsonDocument doc = readJsonFile(jsonFile);

    // Use a separate connection for this "fututre run"
    ServerConfig config = connection->getConfig();
    QSharedPointer<RedisClient::Connection> tmp_connection(new RedisClient::Connection(config));
    ServerConfig conf = config;
    conf.setOwner(tmp_connection.toWeakRef());
    tmp_connection->setConnectionConfig(conf);
    tmp_connection->connect(true);

    QFuture<void> future = run([this, tmp_connection, dbNumber, doc]() {

        try {
            QJsonObject jsonObject = doc.object();
            QJsonObject::iterator it;
            QByteArray keyName;
            QJsonObject keyData;
            QString keyType;
            int current=0;
            int total = jsonObject.count();
            m_keysImportCancelled = false;
            for (it = jsonObject.begin(); it != jsonObject.end(); it++)
            {
                keyName = it.key().toUtf8();
                keyData = it.value().toObject();
                keyType = keyData["type"].toString();
                auto key = BaseKey::factory(tmp_connection, dbNumber, keyName, keyType);
                key->importKeyFromJson(keyData);
                current++;
                emit keysImportProgress(current, total);
                if (m_keysImportCancelled) {
                    break;
                }
            }

        } catch (const RedisClient::Connection::Exception& e) {
            LOG(ERROR) << "Connection error:" + QString(e.what());
        } catch (...)
        {
            LOG(ERROR) << "Unexpected exception thrown";
        }


    });

    if (!m_keysImportWatcher.isRunning())
    {
        m_importerDbIndex = index;
        m_keysImportWatcher.setFuture(future);
    }
    else {
        message = "Another import is already running. Please try later!";
    }

    return message;

}

void Backend::cancelKeysDump()
{
    m_keysDumpCancelled = true;
}

void Backend::cancelKeysImport()
{
    m_keysImportCancelled = true;
}

void Backend::onKeysDumpWatcherFinished()
{
    LOG(INFO) << (m_keysDumpCancelled ? "Dump cancelled: possible partial data has been collected" : "Dump finished");
    emit keysDumpFinished(m_keysDumpCancelled ? "cancelled" : "finished");
}

void Backend::onKeysImportWatcherFinished()
{
    LOG(INFO) << (m_keysImportCancelled ? "Import cancelled: possible partial data has been imported" : "Import finished");
    emit keysImportFinished(m_keysImportCancelled ? "cancelled" : "finished", m_importerDbIndex);
}

void Backend::onConnectionDisconnected(QModelIndex connectionIndex)
{
    QSharedPointer<RedisClient::Connection> connection = getConnectionFromIndex(connectionIndex);
    ServerConfig config = connection->getConfig();
    m_keysCache->remove(config.getUuid());
}

void Backend::onConnectionRemoved(QSharedPointer<RedisClient::Connection> connection)
{
    ServerConfig config = connection->getConfig();
    m_keysCache->remove(config.getUuid());
}

QString Backend::getVersion() {
    return REDINAV_VERSION;
}

QString Backend::buildPatternForNamespace(QSharedPointer<RedisClient::Connection> connection, QString prefix, QString filter)
{
    QString pattern = "";
    if (!filter.isEmpty()) {
        if (prefix.indexOf(filter) != -1) { // "filter" has FOUND in namespace fullpath
            pattern = prefix + ":*";
        } else {
            pattern = prefix + ":*" + filter + "*";
        }
    } else {
        pattern = prefix + ":*";
    }

    ServerConfig config = connection->getConfig();
    QString separator = config.namespaceSeparator();
    pattern.replace(":", separator);

    return pattern;

}

QString Backend::getFileContent(const QString& filePath)
{
    if (filePath.isEmpty())
    {
        return "";
    }
    QFile aFile(filePath);
    if (!aFile.open(QIODevice::ReadOnly))
    {
        LOG(INFO) << "Could not open file for read: " << filePath;
        return "";
    }
    QByteArray data = aFile.readAll();
    aFile.close();
    return QString(data);
}

RedisClient::Connection::NamespaceItems Backend::convertRawKeysListToNamespaceItems(RedisClient::Connection::RawKeysList keys, QString pattern, ServerConfig config)
{

    QString nss = config.namespaceSeparator();

    QRegExp rx(QString("%1[^%1]*$").arg(nss));
    QRegularExpression rxNamespace(QString("^([^%1]*)%1[^%1]*").arg(nss));
    int localKeyPartIndex = rx.indexIn(pattern);

    if (localKeyPartIndex == -1)
    {
        localKeyPartIndex = 0;
    } else {
        localKeyPartIndex++;
    }

    QByteArray localKeyPart;
    QString ns;
    QString full_ns;
    QRegularExpressionMatch match;
    QStringList captures;
    QList<QByteArray> rootKeys;
    QMap<QString,ulong> rootNamespacesMap;
    RedisClient::Connection::RootNamespaces rootNamespaces;
    QString patternNamespace("");

    patternNamespace = pattern.mid(0, localKeyPartIndex);

    for (QByteArray key : keys)
    {

        if (patternNamespace != key.mid(0, localKeyPartIndex))
        {
            continue;
        }

        localKeyPart = key.mid(localKeyPartIndex);
        match = rxNamespace.match(localKeyPart);
        if (!match.hasMatch())
        {
            rootKeys.append(key);
        }
        else
        {
            captures = match.capturedTexts();
            ns = captures[1];
            full_ns = key.mid(0, localKeyPartIndex) + ns;
            if (!rootNamespacesMap.contains(full_ns))
            {
                rootNamespacesMap[full_ns] = 1;
            }
            else
            {
                rootNamespacesMap[full_ns]++;
            }
        }
    }

    QMap<QString,ulong>::const_iterator ns_it;
    rootNamespaces.reserve(rootNamespacesMap.count());
    for (ns_it = rootNamespacesMap.begin(); ns_it != rootNamespacesMap.end(); ns_it++)
    {
        rootNamespaces.append(QPair<QByteArray, ulong>(ns_it.key().toUtf8(), ns_it.value()));
    }

    std::sort(rootKeys.begin(), rootKeys.end());

    return RedisClient::Connection::NamespaceItems(rootNamespaces, rootKeys);


}



