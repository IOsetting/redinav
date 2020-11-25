#include "stringkey.h"
#include <QDebug>
#include <QObject>

StringKey::StringKey(QObject* parent, const QSharedPointer<RedisClient::Connection> connection, const int& dbNumber, const QByteArray& keyName)
    : QObject(parent)
    , BaseKey(connection, dbNumber, keyName, "string")
{
}

StringKey::~StringKey()
{
}

void StringKey::loadRawKeyData()
{
    try {
        m_connection->command({ "GET", m_keyFullPath }, this,
            [this](RedisClient::Response resp, QString e) {
                if (resp.getType() != RedisClient::Response::Bulk || !e.isEmpty()) {
                    throw Exception(QObject::tr("Cannot load value"));
                }
                m_value = resp.getValue().toByteArray();
                emit valueChanged();
            },
            m_dbNumber);
    } catch (const RedisClient::Connection::Exception& e) {
        LOG(ERROR) << QObject::tr("Connection error: ") << QString(e.what());
    }
}

QString StringKey::addKey(const QString& v1, const QString& v2)
{

    Q_UNUSED(v1);

    RedisClient::Response result;
    QString message;
    try {
        result = m_connection->commandSync({ "SET", m_keyFullPath, v2.toUtf8() }, m_dbNumber);
        if (result.isErrorMessage()) {
            message = "Could not add key";
        }
    } catch (const RedisClient::Connection::Exception& e) {
        message = "Connection error:" + QString(e.what());
    }

    return message;
}

QString StringKey::updateKey(const QString& v1, const QString& v2, const QString& oldv1, const QString& oldv2)
{

    Q_UNUSED(v1);
    Q_UNUSED(oldv1);
    Q_UNUSED(oldv2);

    RedisClient::Response result;
    QString message;
    try {
        result = m_connection->commandSync({ "SET", m_keyFullPath, v2.toUtf8() }, m_dbNumber);
        if (result.isErrorMessage()) {
            message = "Could not add/update key";
        }
    } catch (const RedisClient::Connection::Exception& e) {
        message = "Connection error:" + QString(e.what());
    }

    return message;
}


QJsonObject StringKey::getKeyAsJsonObject()
{
    QJsonObject keyData = QJsonObject();

    RedisClient::Response response = m_connection->commandSync({ "GET", m_keyFullPath }, m_dbNumber);
    QString data = response.getValue().toString();

    keyData.insert("type", "string");
    keyData.insert("value", data);

    return keyData;

}

void StringKey::importKeyFromJson(QJsonObject keyData) {

    if (!keyData.contains("value") || !keyData["value"].isString())
        return;

    LOG(INFO) << "Import STRING key in DB " << m_dbNumber << ": " << m_keyFullPath;

    QJsonValue value = keyData["value"];
    QList<QByteArray> rawCommand = QList<QByteArray>({"SET", m_keyFullPath, value.toString().toUtf8()});
    if (keyData.contains("ttl"))
    {
        rawCommand.push_back("EX");
        rawCommand.push_back(keyData["ttl"].toString().toLatin1());
    }
    RedisClient::Response response = m_connection->commandSync(rawCommand, m_dbNumber);

}
