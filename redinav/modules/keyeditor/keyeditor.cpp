#include "keyeditor.h"
#include <QDebug>
#include <qredisclient/redisclient.h>

KeyEditor::KeyEditor(QObject* parent)
    : QObject(parent)
{
}

KeyEditor::~KeyEditor()
{
}

QString KeyEditor::getKeyType()
{
    return m_keyType;
}

QSharedPointer<RedisClient::Connection> KeyEditor::getConnection() const
{
    return m_connection;
}

void KeyEditor::setConnection(QSharedPointer<RedisClient::Connection>& arg)
{
    if (arg != m_connection) {
        m_connection = arg;
        emit connectionChanged();
    }
}

QByteArray KeyEditor::getKey() const
{
    return m_keyFullPath;
}

void KeyEditor::setKey(QByteArray& arg)
{
    if (arg != m_keyFullPath) {
        m_keyFullPath = arg;
        emit keyChanged();
    }
}

int KeyEditor::getDbNumber() const
{
    return m_dbNumber;
}

void KeyEditor::setDbNumber(int& arg)
{
    if (arg != m_dbNumber) {
        m_dbNumber = arg;
        emit dbNumberChanged();
    }
}

void KeyEditor::loadKeyType()
{
    RedisClient::Command typeCmd({ "type", m_keyFullPath }, this, [this](RedisClient::Response resp, QString) {
        m_keyType = resp.value().toString();
        emit keyEditorReady(); }, m_dbNumber);

    try {
        m_connection->runCommand(typeCmd);

    } catch (const RedisClient::Connection::Exception& e) {
        LOG(ERROR) << QObject::tr("Cannot retrieve type of the key: %1").arg(QString(e.what()));
    }
}


void KeyEditor::loadKeyTtl()
{

    RedisClient::Command ttlCmd({ "TTL", m_keyFullPath }, this, [this](RedisClient::Response resp, QString) {
        setTtl(resp.value().toInt());
    }, m_dbNumber);

    try {
        m_connection->runCommand(ttlCmd);
    } catch (const RedisClient::Connection::Exception& e) {
        LOG(ERROR) << QObject::tr("Cannot retrieve key TTL: %1").arg(QString(e.what()));
    }
}


QJsonObject KeyEditor::getConnectionConfig()
{
    return m_connection->getConfig().toJsonObject();
}
