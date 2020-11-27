#pragma once
#include "configmanager.h"

#include <QObject>
#include <qredisclient/connectionconfig.h>

class ServerConfig : public RedisClient::ConnectionConfig {
    Q_GADGET

    /* UUID */
    Q_PROPERTY(QString uuid READ getUuid)

    /* Basic settings */
    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(QString host READ host WRITE setHost)
    Q_PROPERTY(uint port READ port WRITE setPort)
    Q_PROPERTY(QString auth READ auth WRITE setAuth)
    Q_PROPERTY(QString originalHost READ getOriginalHost WRITE setOriginalHost)
    Q_PROPERTY(uint originalPort READ getOriginalPort WRITE setOriginalPort)

    /* Advanced settings */
    Q_PROPERTY(QString keysPattern READ keysPattern WRITE setKeysPattern)
    Q_PROPERTY(QString namespaceSeparator READ namespaceSeparator WRITE setNamespaceSeparator)
    Q_PROPERTY(uint executeTimeout READ executeTimeout WRITE setExecutionTimeout)
    Q_PROPERTY(uint connectionTimeout READ connectionTimeout WRITE setConnectionTimeout)
    Q_PROPERTY(bool luaKeysLoading READ luaKeysLoading WRITE setLuaKeysLoading)
    Q_PROPERTY(bool readOnly READ readOnly WRITE setReadOnly)
    Q_PROPERTY(QString defaultFilter READ getDefaultFilter WRITE setDefaultFilter)

    /* SSL settings */
    Q_PROPERTY(bool sslEnabled READ getSslEnabled WRITE setSslEnabled)
    Q_PROPERTY(QString sslLocalCertContent READ sslLocalCertContent WRITE setSslLocalCertContent)
    Q_PROPERTY(QString sslPrivateKeyContent READ sslPrivateKeyContent WRITE setSslPrivateKeyContent)
    Q_PROPERTY(QString sslCaCertContent READ sslCaCertContent WRITE setSslCaCertContent)

    /* SSH Settings */
    Q_PROPERTY(bool sshEnabled READ getSshEnabled WRITE setSshEnabled)
    Q_PROPERTY(QString  sshPassword READ sshPassword WRITE setSshPassword)
    Q_PROPERTY(QString  sshUser READ sshUser WRITE setSshUser)
    Q_PROPERTY(QString  sshHost READ sshHost WRITE setSshHost)
    Q_PROPERTY(uint     sshPort READ sshPort WRITE setSshPort)
    Q_PROPERTY(QString  sshPrivateKeyContent READ sshPrivateKeyContent WRITE setSshPrivateKeyContent)

    Q_PROPERTY(bool isEncrypted READ isEncrypted)


    QString generateFilePathFromContent(const QByteArray content, QSharedPointer<ConfigManager> configMan, const QString prefix = "redinav_", const QString ext = "txt");
    QString getFileContent(const QString &filePath);

    QString m_sshPrivateKeyContent;
    QString m_sslLocalCertContent;
    QString m_sslPrivateKeyContent;
    QString m_sslCaCertContent;
    QString m_originalHost;
    uint m_originalPort;

    void setSshPrivateKeyContent(QString value);
    void setSslLocalCertContent(QString value);
    void setSslPrivateKeyContent(QString value);
    void setSslCaCertContent(QString value);

    QString sshPrivateKeyContent() const;
    QString sslLocalCertContent() const;
    QString sslPrivateKeyContent() const;
    QString sslCaCertContent() const;

    QString loadSshPrivateKeyContent();
    QString loadSslLocalCertContent();
    QString loadSslPrivateKeyContent();
    QString loadSslCaCertContent();

    bool getSshEnabled() const;
    void setSshEnabled(bool value);
    bool getSslEnabled() const;
    void setSslEnabled(bool value);

    bool isEncrypted();
public:
    static const char DEFAULT_NAMESPACE_SEPARATOR = ':';
    static const char DEFAULT_KEYS_GLOB_PATTERN = '*';
    static const bool DEFAULT_LUA_KEYS_LOADING = false;
    static const bool DEFAULT_READ_ONLY = false;


public:
    ServerConfig(const QString& host = "127.0.0.1", const QString& auth = "", const uint port = DEFAULT_REDIS_PORT, const QString& name = "");
    ServerConfig(const RedisClient::ConnectionConfig&);
    ServerConfig(const QVariantHash& options);
    ServerConfig & operator = (const ServerConfig & other);

    QString keysPattern() const;
    void setKeysPattern(QString keyGlobPattern);

    QString namespaceSeparator() const;
    void setNamespaceSeparator(QString);

    bool luaKeysLoading() const;
    void setLuaKeysLoading(bool);

    bool readOnly() const;
    void setReadOnly(bool);

    void saveSecurityContentItemsToFiles(QSharedPointer<ConfigManager> configManager);

    QString saveContentToHashedFilePath(const QString value, QSharedPointer<ConfigManager> configMan, const QString prefix, const QString ext);

    Q_INVOKABLE void loadSecurityContentItemsFromFile();


    QString getUuid() const;
    void setUuid(const QString &Uuid);
    QString getOriginalHost() const;
    void setOriginalHost(const QString &originalHost);
    uint getOriginalPort() const;
    void setOriginalPort(const uint &originalPort);

    QString getDefaultFilter() const;
    void setDefaultFilter(const QString &filter);

    QWeakPointer<RedisClient::Connection> getOwner() const;
    void setOwner(QWeakPointer<RedisClient::Connection> o);

private:
    QWeakPointer<RedisClient::Connection> m_owner;
};

Q_DECLARE_METATYPE(ServerConfig)
