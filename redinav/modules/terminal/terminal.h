#pragma once
#include <QObject>
#include <qredisclient/connection.h>
#include <qredisclient/connectionconfig.h>
#include <backend.h>
#include <easylogging++.h>

class Terminal : public QObject
{
    Q_OBJECT
    ADD_EXCEPTION


    Q_PROPERTY(QSharedPointer<RedisClient::Connection> connection READ connection WRITE setConnection NOTIFY connectionChanged)
    Q_PROPERTY(int db READ workingDb WRITE setWorkingDb NOTIFY dbChanged)
    Q_PROPERTY(QStringList history READ history NOTIFY historyChanged)
    Q_PROPERTY(int dbCount READ databasesCount NOTIFY databaseCountChanged)
    Q_PROPERTY(QJsonObject config READ config NOTIFY configChanged)
    Q_PROPERTY(QSharedPointer<Backend> backend READ getBackend WRITE setBackend NOTIFY backendChanged)
    Q_PROPERTY(QJsonArray masterNodes READ getMasterNodesJson NOTIFY masterNodesChanged)
    Q_PROPERTY(bool busy READ getBusy WRITE setBusy NOTIFY busyChanged)
    Q_PROPERTY(int connectionMode READ getConnectionMode NOTIFY connectionModeChanged)


public:
    explicit Terminal(QObject *parent = nullptr);
    ~Terminal();

    Q_INVOKABLE void connect();

    QSharedPointer<RedisClient::Connection> connection() const;
    void setConnection(const QSharedPointer<RedisClient::Connection> &connection);

    Q_INVOKABLE bool execCmd(const QString &input, const bool skipHistory = false);
    Q_INVOKABLE void execCmdText(const QString &input, const bool skipHistory = true);

    int workingDb() const;
    void setWorkingDb(int workingDb);

    QStringList history() const;

    Q_INVOKABLE void execCommandsFile(const QString filePath);

    Q_INVOKABLE void execLua(const QString content, QString params="");
    Q_INVOKABLE void execLuaFile(QString filePath, QString params="");

    Q_INVOKABLE QByteArray loadFile(const QString filePath);


    int databasesCount() const;
    void setDatabasesCount(int databasesCount);

    QSharedPointer<Backend> getBackend() const;
    void setBackend(const QSharedPointer<Backend> &backend);

    RedisClient::Connection::HostList getMasterNodes() const;
    void setMasterNodes(const RedisClient::Connection::HostList &masterNodes);

    QJsonArray getMasterNodesJson() const;

    Q_INVOKABLE void reConnectTo(const int index = 0);

    bool getBusy() const;
    void setBusy(bool busy);


private:
    QSharedPointer<RedisClient::Connection> m_connection;
    int m_workingDb;
    int m_databasesCount;

    QStringList m_history;


    void addToHistory(QString command);
    void clearHistory();
    int getServerDatabasesCount();
    QJsonObject config();

    QSharedPointer<Backend> m_backend;

    RedisClient::Connection::HostList m_masterNodes;
    bool m_busy;

    QString m_originalHost;
    uint m_originalPort;

    int getConnectionMode();

signals:
    void connectionChanged();
    void dbChanged();
    void responseTaken(QString resp);
    void historyChanged();
    void databaseCountChanged();
    void configChanged();
    void backendChanged();
    void masterNodesChanged();
    void connectionFailed();
    void error(const QString& message);
    void busyChanged();
    void connectionModeChanged();

public slots:
    void slotConnectionConnected();
};

