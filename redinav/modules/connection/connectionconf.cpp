#include "connectionconf.h"
#include <easylogging++.h>

#include <QDir>
#include <QTextStream>
#include <quuid.h>

ServerConfig::ServerConfig(const QString& host, const QString& auth, const uint port, const QString& name)
    : RedisClient::ConnectionConfig(host, auth, port, name)
{
}

ServerConfig::ServerConfig(const RedisClient::ConnectionConfig& other)
{
    m_parameters = other.getInternalParameters();
    m_owner = other.getOwner();
    loadSecurityContentItemsFromFile();
}

ServerConfig::ServerConfig(const QVariantHash &options)
    : RedisClient::ConnectionConfig(options)
{
    setOriginalHost(options.value("host").toString());
    setOriginalPort(options.value("port").toInt());
}

ServerConfig &ServerConfig::operator =(const ServerConfig &other)
{
    if (this != &other) {
        m_parameters = other.m_parameters;
        m_owner = other.m_owner;
        m_sshPrivateKeyContent = other.m_sshPrivateKeyContent;
        m_sslLocalCertContent = other.m_sslLocalCertContent;
        m_sslPrivateKeyContent = other.m_sslPrivateKeyContent;
        m_sslCaCertContent = other.m_sslCaCertContent;
    }
    return *this;
}


QString ServerConfig::keysPattern() const
{
    return param<QString>("keys_pattern", QString(DEFAULT_KEYS_GLOB_PATTERN));
}

void ServerConfig::setKeysPattern(QString keyGlobPattern)
{
    setParam<QString>("keys_pattern", keyGlobPattern);
}

QString ServerConfig::namespaceSeparator() const
{
    return param<QString>("namespace_separator", QString(DEFAULT_NAMESPACE_SEPARATOR));
}

void ServerConfig::setNamespaceSeparator(QString ns)
{
    return setParam<QString>("namespace_separator", ns);
}

bool ServerConfig::luaKeysLoading() const
{
    return param<bool>("lua_keys_loading", DEFAULT_LUA_KEYS_LOADING);
}

void ServerConfig::setLuaKeysLoading(bool value)
{
    return setParam<bool>("lua_keys_loading", value);
}

void ServerConfig::saveSecurityContentItemsToFiles(QSharedPointer<ConfigManager> configManager)
{

    if (!m_sshPrivateKeyContent.isEmpty())
        setSshPrivateKeyPath(saveContentToHashedFilePath(m_sshPrivateKeyContent, configManager, "ssh_pkey", "key"));
    else
        setSshPrivateKeyPath("");

    if (!m_sslLocalCertContent.isEmpty())
        setSslLocalCertPath(saveContentToHashedFilePath(m_sslLocalCertContent, configManager, "ssl_client_cert", "crt"));
    else
        setSslLocalCertPath("");

    if (!m_sslPrivateKeyContent.isEmpty())
        setSslPrivateKeyPath(saveContentToHashedFilePath(m_sslPrivateKeyContent, configManager, "ssl_client_pkey", "key"));
    else
        setSslPrivateKeyPath("");

    if (!m_sslCaCertContent.isEmpty())
        setSslCaCertPath(    saveContentToHashedFilePath(m_sslCaCertContent, configManager, "ssl_ca_cert", "pem"));
    else
        setSslCaCertPath("");


}

bool ServerConfig::readOnly() const
{
    return param<bool>("read_only", DEFAULT_READ_ONLY);
}

void ServerConfig::setReadOnly(bool value)
{
    return setParam<bool>("read_only", value);
}


QString ServerConfig::loadSshPrivateKeyContent()
{
    return getFileContent(getSshPrivateKeyPath());
}

QString ServerConfig::loadSslLocalCertContent()
{
    return getFileContent(sslLocalCertPath());
}

QString ServerConfig::loadSslPrivateKeyContent()
{
    return getFileContent(sslPrivateKeyPath());
}

QString ServerConfig::loadSslCaCertContent()
{
    return getFileContent(sslCaCertPath());
}

bool ServerConfig::getSshEnabled() const
{
    return param<bool>("ssh", false);
}

void ServerConfig::setSshEnabled(bool value)
{
    return setParam<bool>("ssh", value);
}


bool ServerConfig::getSslEnabled() const
{
    return useSsl();
}

void ServerConfig::setSslEnabled(bool value)
{
    return setSsl(value);
}


QString ServerConfig::getFileContent(const QString& filePath)
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

uint ServerConfig::getOriginalPort() const
{
    return param<uint>("original_port", 6379);
}

void ServerConfig::setOriginalPort(const uint &originalPort)
{
    return setParam<uint>("original_port", originalPort);
}

QString ServerConfig::getDefaultFilter() const
{
    return param<QString>("default_filter", "");
}

void ServerConfig::setDefaultFilter(const QString &filter)
{
    return setParam<QString>("default_filter", filter);
}

QString ServerConfig::getOriginalHost() const
{
    return param<QString>("original_host", "127.0.0.1");
}

void ServerConfig::setOriginalHost(const QString &originalHost)
{
    return setParam<QString>("original_host", originalHost);
}

QString ServerConfig::getUuid() const
{
    return param<QString>("uuid", "");
}

void ServerConfig::setUuid(const QString &Uuid)
{
    return setParam<QString>("uuid", Uuid);
}

void ServerConfig::setSshPrivateKeyContent(QString value)
{
    m_sshPrivateKeyContent = value;
}


void ServerConfig::ServerConfig::setSslLocalCertContent(QString value)
{
    m_sslLocalCertContent = value;
}

void ServerConfig::ServerConfig::setSslPrivateKeyContent(QString value)
{
    m_sslPrivateKeyContent = value;
}

void ServerConfig::ServerConfig::setSslCaCertContent(QString value)
{
    m_sslCaCertContent = value;
}

QString ServerConfig::sshPrivateKeyContent() const
{
    return m_sshPrivateKeyContent;
}

QString ServerConfig::ServerConfig::sslLocalCertContent() const
{
    return m_sslLocalCertContent;
}

QString ServerConfig::ServerConfig::sslPrivateKeyContent() const
{
    return m_sslPrivateKeyContent;
}

QString ServerConfig::ServerConfig::sslCaCertContent() const
{
    return m_sslCaCertContent;
}


/**
 *
 * Given a content, generate file name (hashed name) and write the content itself into that file
 *
 * @brief ServerConfig::saveContentToHashedFilePath
 * @param value
 * @param configMan
 * @param prefix
 * @param ext
 * @return
 */
QString ServerConfig::saveContentToHashedFilePath(const QString value, QSharedPointer<ConfigManager> configMan, const QString prefix, const QString ext)
{
    if (value.isEmpty())
    {
        return "";
    }
    QFile keyFile;
    QString filePath = generateFilePathFromContent(value.toUtf8(), configMan, prefix, ext);
    keyFile.setFileName(filePath);
    if (!keyFile.open(QIODevice::ReadWrite | QIODevice::Text))
    {
        LOG(INFO) << "Could not open file for write: " << filePath;
        return "";
    }
    QTextStream out(&keyFile);
    out << value.toUtf8();
    keyFile.close();
    return filePath;
}

void ServerConfig::loadSecurityContentItemsFromFile()
{
    setSshPrivateKeyContent(loadSshPrivateKeyContent());
    setSslLocalCertContent(loadSslLocalCertContent());
    setSslPrivateKeyContent(loadSslPrivateKeyContent());
    setSslCaCertContent(loadSslCaCertContent());
}

/**
 * @brief Generate file path, pointing to file in "configuration folder", where the name of the file is sha1 of the content (i.e. unique)
 * @param content
 * @param configMan
 * @return
 */
QString ServerConfig::generateFilePathFromContent(const QByteArray content, QSharedPointer<ConfigManager> configMan, const QString prefix, const QString ext)
{
    QString fileName = QString(QCryptographicHash::hash((content),QCryptographicHash::Md5).toHex());
    QString filePath = QDir::toNativeSeparators(QString("%1/%2_%3.%4").arg(configMan->getConfigDir()).arg(prefix).arg(fileName).arg(ext));
    return filePath;
}

bool ServerConfig::isEncrypted()
{
    return getSshEnabled() || getSslEnabled();
}

