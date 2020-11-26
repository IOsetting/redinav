#pragma once
#include "basekey.h"
#include <QObject>
#include <easylogging++.h>
#include <qredisclient/connection.h>

class StringKey : public QObject, public BaseKey {

    Q_PROPERTY(QSharedPointer<RedisClient::Connection> connection READ getConnection WRITE setConnection)
    Q_PROPERTY(QByteArray keyFullPath READ getKey WRITE setKey)
    Q_PROPERTY(int dbNumber READ getDbNumber WRITE setDbNumber)

    Q_PROPERTY(QByteArray value READ getValue() NOTIFY valueChanged)

    Q_OBJECT
    ADD_EXCEPTION

public:
    explicit StringKey(QObject* parent = nullptr, const QSharedPointer<RedisClient::Connection> connection = Q_NULLPTR, const int& dbNumber = 0, const QByteArray& keyName = "");
    ~StringKey();

    Q_INVOKABLE void loadRawKeyData();
    Q_INVOKABLE QString updateKey(const QString& v1, const QString& v2, const QString& oldv1, const QString& oldv2) override;

    QString addKey(const QString& v1, const QString& v2) override;
    QJsonObject getKeyAsJsonObject() override;
    void importKeyFromJson(QJsonObject keyData) override;
signals:
    void valueChanged();
    void keyRemoved();
};
