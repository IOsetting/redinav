#pragma once
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QString>

#include <qredisclient/connection.h>

class ConfigManager {
public:
    ConfigManager();
    QString getConfigDir();
    bool saveConnectionsToFile(const QJsonArray& connections, const QString targetFile="");
    QString getConfigFilepath() const;

private:
    QString m_configDir;

#ifdef Q_OS_MACOS
        QString m_configFilename = "redinav-connections.json";
#else
        QString m_configFilename = "connections.json";
#endif

    QString m_configFilepath;
    bool checkPath(const QString& configFilepath);
    static void setPermissions(QFile&);
};
