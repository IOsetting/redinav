#pragma once
#include <qredisclient/connection.h>
#include <qredisclient/connectionconfig.h>
#include <QJsonArray>
#include <QObject>
#include <QVariantMap>
#include <easylogging++.h>

class ServerInfo : public QObject
{

    Q_PROPERTY(QSharedPointer<RedisClient::Connection> connection READ getConnection WRITE setConnection NOTIFY connectionChanged)
    Q_PROPERTY(QVariantMap serverData READ serverData NOTIFY serverDataChanged)
    Q_PROPERTY(bool frozen READ frozen WRITE setFrozen NOTIFY frozenChanged)
    Q_PROPERTY(bool includeClientsList READ includeClientsList WRITE setIncludeClientsList NOTIFY includeClientsListChanged)
    Q_PROPERTY(QJsonObject config READ config NOTIFY configChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)

    Q_PROPERTY(QJsonArray clientsList READ clientsList NOTIFY clientsListChanged)

    Q_OBJECT
    ADD_EXCEPTION
public:

        explicit ServerInfo(QObject *parent = nullptr);
        ~ServerInfo(void);

    QSharedPointer<RedisClient::Connection> getConnection() const;
    void setConnection(QSharedPointer<RedisClient::Connection>& arg);

    QVariantMap serverData();
    bool frozen();
    void setFrozen(bool arg);

    bool includeClientsList();
    void setIncludeClientsList(bool arg);

    QJsonArray clientsListJson();

    QJsonArray clientsList();

    bool busy();
    void setBusy(bool arg);

    QJsonObject config();


    Q_INVOKABLE void init();

signals:
    void connectionChanged();
    void error(const QString& error);
    void initialized();
    void serverDataChanged();
    void frozenChanged();
    void includeClientsListChanged();
    void configChanged();
    void busyChanged();
    void clientsListJsonChanged();
    void clientsListChanged();
    void destroyed(QObject *);
    void serverConnected();
    void connectionFailed();

public slots:


private:
    QTimer m_updateTimer;
    QVariantMap m_serverData;
    QSharedPointer<RedisClient::Connection> m_connection;
    bool m_frozen;
    bool m_includeClientsList;
    QJsonObject m_config;
    bool m_busy;

    QJsonArray m_clientsList;

    int m_sort_column;
    int m_sort_order;

    QString m_rawData;

    QJsonArray toJsonArray(QString data);
    QList<QList<QPair<QString, QString> > > toListOfListOfPairs(const QString &data);
};

