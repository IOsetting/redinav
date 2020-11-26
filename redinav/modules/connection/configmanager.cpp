#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QVariantHash>
#include <QXmlStreamReader>
#include <easylogging++.h>
#include <qredisclient/utils/compat.h>

#include "configmanager.h"
#include "connectionconf.h"

/**
 * @brief ConfigManager::ConfigManager
 * @param basePath
 */
ConfigManager::ConfigManager()
{

    QString configDir;

#if defined(Q_OS_MACOS)
        configDir = QString("%1/%2").arg(QDir::homePath()).arg("Library/Preferences");
#elif defined(Q_OS_WIN)
        configDir = QString("%1/%2").arg(QDir::homePath()).arg("RediNav");
#else
        configDir = QString("%1/%2").arg(QDir::homePath()).arg(".redinav");
#endif

    m_configDir = QDir::toNativeSeparators(configDir);

    // Create the directory if it does not exist
    if (!QDir(m_configDir).exists()) {
        QDir(m_configDir).mkpath(m_configDir);
    }

    // Build final FILE PATH
    m_configFilepath = QString("%1/%2").arg(m_configDir).arg(m_configFilename);

    // Check it and set permissions etc.
    if (!checkPath(m_configFilepath))
    {
        QMessageBox::critical(
            nullptr,
            QObject::tr("RediNav encountered a critical problem"),
            QString(QObject::tr("Could not find or create user configuration directory"))
        );
        throw std::runtime_error("Application configuration directory problem (persmissions?)");
    }

}

QString ConfigManager::getConfigFilepath() const
{
    return m_configFilepath;
}

/**
 * @brief ConfigManager::getConfigDir
 * @return
 */
QString ConfigManager::getConfigDir()
{
    return m_configDir;
}

/**
 * @brief ConfigManager::saveConnectionsToFile
 * @param connections
 */
bool ConfigManager::saveConnectionsToFile(const QJsonArray& connections, const QString targetFile)
{

    QJsonDocument config(connections);

    QString filePath = !targetFile.isEmpty() ? targetFile : getConfigFilepath();

    QFile confFile(filePath);

    if (confFile.open(QIODevice::WriteOnly)) {
        QTextStream outStream(&confFile);
        outStream.setCodec("UTF-8");
        outStream << config.toJson();
        confFile.close();
        return true;
    }

    return false;
}

bool ConfigManager::checkPath(const QString& configFilepath)
{
    QFile testConfig(configFilepath);

    QFileInfo checkPermissions(testConfig);

    if (!testConfig.exists() && testConfig.open(QIODevice::ReadWrite))
        testConfig.close();

    if (checkPermissions.isWritable()) {
        setPermissions(testConfig);
        return true;
    }

    return false;
}

void ConfigManager::setPermissions(QFile& file)
{
#ifdef Q_OS_WIN
    extern Q_CORE_EXPORT int qt_ntfs_permission_lookup;
    qt_ntfs_permission_lookup++;
#endif
    if (file.setPermissions(QFile::ReadUser | QFile::WriteUser)) {
    }

#ifdef Q_OS_WIN
    qt_ntfs_permission_lookup--;
#endif
}
