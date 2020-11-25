#include "setkey.h"
#include <QDebug>
#include <QObject>

SetKey::SetKey(QObject* parent, const QSharedPointer<RedisClient::Connection> connection, const int& dbNumber, const QByteArray& keyName)
    : QObject(parent)
    , BaseKey(connection, dbNumber, keyName, "set")
{
}

SetKey::~SetKey()
{
}

void SetKey::loadRawKeyData(const int& page, const QString& search)
{

    Q_UNUSED(page);

    if (isLocked()) {
        return;
    }

    lock();

    //loadRowsCount();

    QString searchPattern = (search.size() > 0) ? "*" + search + "*" : "*";

    QList<QByteArray> cmdParts = { "SSCAN", m_keyFullPath, "0", "MATCH", searchPattern.toUtf8(), "COUNT", "10000" };
    RedisClient::ScanCommand cmd(cmdParts, m_dbNumber);

    try {
        emptyRows();
        m_connection->retrieveCollection(cmd, [this](QVariant result, QString) {
            if (result.type() == QVariant::Type::List) {
                foreach (QVariant row, result.toList()) {
                    m_rows.push_back(row.toByteArray());
                }
            }
            m_rowsCount = m_rows.count();
            buildListData(0, Qt::AscendingOrder);
            unlock();
        });
    } catch (const RedisClient::Connection::Exception& e) {
        unlock();
        emitAllFinalSignals();
        LOG(ERROR) << QObject::tr("Connection error: ") << QString(e.what());
    }
}

/**
 * Get the number of members in the list. No search applied whatsoever
 * @brief SetKey::loadRowsCount
 */
void SetKey::loadRowsCount()
{
    RedisClient::Response result;

    try {
        result = m_connection->commandSync({ "SCARD", m_keyFullPath }, m_dbNumber);
        if (result.getType() == RedisClient::Response::Integer) {
            m_rowsCount = result.getValue().toUInt();
        }
        else {
            LOG(ERROR) << "Invalid response type";
        }
    } catch (const RedisClient::Connection::Exception& e) {
        LOG(ERROR) << QObject::tr("Connection error: ") << QString(e.what());
    }


    emit rowsCountChanged();
}

void SetKey::buildListData(int column, Qt::SortOrder order)
{

    std::sort(m_rows.begin(), m_rows.end(), [this, column, order](const QVariant& a, const QVariant& b) {
        switch (order) {
        case Qt::AscendingOrder:
            return a.toString().toLower() < b.toString().toLower();
        case Qt::DescendingOrder:
            return a.toString().toLower() > b.toString().toLower();
        }
        return false;
    });

    while (m_listModelData.count() > 0) {
        m_listModelData.pop_back();
    }

    for (int i = 0; i < m_rows.count(); i++) {
        QJsonObject item;
        item.insert("value", QJsonValue::fromVariant(m_rows.at(i)).toString());
        m_listModelData.append(item);
    }

    emitAllFinalSignals();

}

void SetKey::removeRows(QList<int> rowNumbers, QList<QString> rowKeys, QList<QString> rowValues)
{
    Q_UNUSED(rowNumbers);
    Q_UNUSED(rowKeys);

    try {
        QList<QByteArray> rawCmd = { "SREM", m_keyFullPath };
        for (int i = 0; i < rowValues.count(); i++) {
            rawCmd.push_back(rowValues.at(i).toUtf8());
            m_rows.removeAt(m_rows.indexOf(rowValues.at(i).toUtf8()));
            m_rowsCount--;
        }
        m_connection->commandSync(rawCmd, m_dbNumber);

        // Key is gone ?
        QString tmp = QString::fromUtf8(m_keyFullPath);
        if (!keyExists(m_connection, m_dbNumber, tmp)) {
            emit keyRemoved();
        }
        buildListData();
    } catch (const RedisClient::Connection::Exception& e) {
        LOG(ERROR) << QObject::tr("Connection error: ") << QString(e.what());
    }
}

QString SetKey::updateKey(const QString& v1, const QString& v2, const QString& oldv1, const QString& oldv2)
{

    Q_UNUSED(v1);
    Q_UNUSED(oldv1);

    RedisClient::Response result;
    QString message;
    try {

        if ((v2 != oldv2) && memberExists(v2.toUtf8())) {
            message = "Member already exists!";
        } else {
            result = m_connection->commandSync({ "SREM", m_keyFullPath, oldv2.toUtf8() }, m_dbNumber);
            result = m_connection->commandSync({ "SADD", m_keyFullPath, v2.toUtf8() }, m_dbNumber);
            if (result.isErrorMessage()) {
                message = "Could not update key";
                result = m_connection->commandSync({ "SADD", m_keyFullPath, oldv2.toUtf8() }, m_dbNumber);
            }
        }
    } catch (const RedisClient::Connection::Exception& e) {
        message = "Connection error:" + QString(e.what());
    }

    return message;
}

QString SetKey::addRow(const QString& v1, const QString& v2)
{

    Q_UNUSED(v1);

    if (memberExists(v2.toUtf8())) {
        return "Member already exists";
    }

    RedisClient::Response result;
    QString message;
    try {
        result = m_connection->commandSync({ "SADD", m_keyFullPath, v2.toUtf8() }, m_dbNumber);
        if (result.isErrorMessage()) {
            message = "Could not add row to a key";
        } else {
            // SET : values are unique
            if (m_rows.indexOf(v2.toUtf8()) == -1) {
                m_rows.push_back(v2.toUtf8());
                m_rowsCount = m_rows.count();
                buildListData();
            }
        }
    } catch (const RedisClient::Connection::Exception& e) {
        message = "Connection error:" + QString(e.what());
    }

    return message;
}

QString SetKey::addKey(const QString& v1, const QString& v2)
{

    Q_UNUSED(v1);

    RedisClient::Response result;
    QString message;
    try {
        result = m_connection->commandSync({ "SADD", m_keyFullPath, v2.toUtf8() }, m_dbNumber);
        if (result.isErrorMessage()) {
            message = "Could not add key";
        }
    } catch (const RedisClient::Connection::Exception& e) {
        message = "Connection error:" + QString(e.what());
    }

    return message;
}

bool SetKey::memberExists(const QByteArray& memberValue)
{
    RedisClient::Response result;
    try {
        result = m_connection->commandSync({ "SISMEMBER", m_keyFullPath, memberValue }, m_dbNumber);
        if (result.isErrorMessage()) {
            return false;
        }

        return result.getValue().toInt() > 0;
    } catch (const RedisClient::Connection::Exception& e) {
        return false;
    }
}

QJsonObject SetKey::getKeyAsJsonObject()
{
    QJsonObject keyData = QJsonObject();

    RedisClient::Response response = m_connection->commandSync({ "SMEMBERS", m_keyFullPath }, m_dbNumber);
    QJsonArray data = response.getValue().toJsonArray();

    keyData.insert("type", "set");
    keyData.insert("value", data);

    return keyData;
}


void SetKey::importKeyFromJson(QJsonObject keyData) {

    if (!keyData.contains("value") || !keyData["value"].isArray())
        return;

    LOG(INFO) << "Import SET key in DB " << m_dbNumber << ": " << m_keyFullPath;

    QJsonArray value = keyData["value"].toArray();

    if (value.count() <= 0)
        return;

    RedisClient::Response response = m_connection->commandSync({ "DEL", m_keyFullPath }, m_dbNumber);

    QList<QByteArray> rawCmd = {"SADD", m_keyFullPath};

    foreach (QJsonValue member, value) {
        rawCmd.push_back(member.toString().toUtf8());
    }

    response = m_connection->commandSync(rawCmd, m_dbNumber);

    if (keyData.contains("ttl"))
    {
        response = m_connection->commandSync({ "EXPIRE", m_keyFullPath, keyData["ttl"].toString().toLatin1() },m_dbNumber);
    }


}


void SetKey::emitAllFinalSignals() {
    emit listDataChanged();
    emit isPaginatedChanged();
    emit rowsCountChanged();
}
