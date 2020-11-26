#pragma once
#include <QObject>
#include <easylogging++.h>
#include <qredisclient/connection.h>

class KeyEditor : public QObject {

    Q_PROPERTY(QSharedPointer<RedisClient::Connection> connection READ getConnection WRITE setConnection NOTIFY connectionChanged)
    Q_PROPERTY(QByteArray keyFullPath READ getKey WRITE setKey NOTIFY keyChanged)
    Q_PROPERTY(int dbNumber READ getDbNumber WRITE setDbNumber NOTIFY dbNumberChanged)
    Q_PROPERTY(QString keyType READ getKeyType() NOTIFY keyTypeChanged)
    Q_PROPERTY(int ttl READ getTtl WRITE setTtl NOTIFY ttlChanged)

    Q_OBJECT
    ADD_EXCEPTION

public:
    explicit KeyEditor(QObject* parent = nullptr);
    ~KeyEditor();

    QString getKeyType();
    Q_INVOKABLE void loadKeyType();
    Q_INVOKABLE void loadKeyTtl();
    Q_INVOKABLE QJsonObject getConnectionConfig();

    QSharedPointer<RedisClient::Connection> getConnection() const;
    void setConnection(QSharedPointer<RedisClient::Connection>& arg);

    QByteArray getKey() const;
    void setKey(QByteArray& arg);

    int getDbNumber() const;
    void setDbNumber(int& arg);

    int getTtl()
    {
        return m_ttl;
    }

    void setTtl(int arg)
    {
        if (m_ttl != arg) {
            m_ttl = arg;
            emit ttlChanged();
        }
    }


signals:
    void keyEditorReady();
    void connectionChanged();
    void keyChanged();
    void dbNumberChanged();
    void ttlChanged();
    void keyTypeChanged();

public slots:

private:
    QByteArray m_keyFullPath;
    QSharedPointer<RedisClient::Connection> m_connection;
    int m_dbNumber;
    QString m_keyType;
    int m_ttl;
};
