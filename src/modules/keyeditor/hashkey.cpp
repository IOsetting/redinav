#include "hashkey.h"
#include "modules/connection/connectionconf.h"
#include <QDebug>
#include <QObject>

HashKey::HashKey(QObject* parent, const QSharedPointer<RedisClient::Connection> connection, const int& dbNumber, const QByteArray& keyName)
    : QObject(parent)
    , BaseKey(connection, dbNumber, keyName, "hash")
{
}

HashKey::~HashKey()
{
}

void HashKey::loadRawKeyData(const int& page, const QString& search)
{

    Q_UNUSED(page);

    if (isLocked()) {
        return;
    }

    lock();

    RedisClient::Response result;

    QString searchPattern = (search.size() > 0) ? "*" + search + "*" : "*";

    QList<QByteArray> cmdParts = { "HSCAN", m_keyFullPath, "0", "MATCH", searchPattern.toUtf8(), "COUNT", "50000" };
    RedisClient::ScanCommand cmd(cmdParts, m_dbNumber);

    try {
        emptyRows();
        emptyPairList();
        m_connection->retrieveCollection(cmd, [this](QVariant result, QString) {
            if (result.type() == QVariant::Type::List) {
                foreach (QVariant row, result.toList()) {
                    m_rows.push_back(row.toByteArray());
                }
            }
            m_pairList = convertToListOfPairs(m_rows);
            m_rowsCount = m_pairList.count();
            buildListData(0, Qt::AscendingOrder);
            unlock();
        });
    } catch (const RedisClient::Connection::Exception& e) {
        LOG(ERROR) << QObject::tr("Cannot load raw key data").arg(QString(e.what()));
        emitAllFinalSignals();
        unlock();
    }
}

void HashKey::buildListData(int column, Qt::SortOrder order)
{

    std::sort(m_pairList.begin(), m_pairList.end(), [column, order](const QPair<QByteArray, QByteArray>& a, const QPair<QByteArray, QByteArray>& b) {
        switch (order) {
        case Qt::AscendingOrder:
            if (column == 0)
                return a.first.toLower() < b.first.toLower();
            else
                return a.second.toLower() < b.second.toLower();
        case Qt::DescendingOrder:
            if (column == 0)
                return a.first.toLower() > b.first.toLower();
            else
                return a.second.toLower() > b.second.toLower();
        }
        return false;
    });

    while (m_listModelData.count() > 0) {
        m_listModelData.pop_back();
    }

    for (int i = 0; i < m_pairList.count(); i++) {
        QJsonObject item;
        item.insert("key", QString(m_pairList[i].first));
        item.insert("value", QString(m_pairList[i].second));
        m_listModelData.append(item);
    }

    emitAllFinalSignals();
}

void HashKey::removeRows(QList<int> rowNumbers, QList<QString> rowKeys, QList<QString> rowValues)
{

    Q_UNUSED(rowNumbers);

    try {
        int pairIndex;
        QList<QByteArray> rawCmd = { "HDEL", m_keyFullPath };
        for (int i = 0; i < rowKeys.count(); i++) {
            rawCmd.push_back(rowKeys.at(i).toUtf8());
            pairIndex = m_pairList.indexOf(QPair<QByteArray, QByteArray>(rowKeys.at(i).toUtf8(), rowValues.at(i).toUtf8()));
            m_pairList.removeAt(pairIndex);
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

QString HashKey::addRow(const QString& v1, const QString& v2)
{

    if (fieldExists(v1.toUtf8())) {
        return "Field already exists";
    }

    RedisClient::Response result;
    QString message;
    try {
        result = m_connection->commandSync({ "HSET", m_keyFullPath, v1.toUtf8(), v2.toUtf8() }, m_dbNumber);
        if (result.isErrorMessage()) {
            message = "Could not add row to a key";
        } else {
            m_rowsCount++;
            m_pairList.push_back(QPair<QByteArray, QByteArray>(v1.toUtf8(), v2.toUtf8()));
            buildListData(0, Qt::AscendingOrder);
        }
    } catch (const RedisClient::Connection::Exception& e) {
        message = "Connection error:" + QString(e.what());
    }

    return message;
}

QString HashKey::addKey(const QString& v1, const QString& v2)
{
    RedisClient::Response result;
    QString message;
    try {
        result = m_connection->commandSync({ "HSET", m_keyFullPath, v1.toUtf8(), v2.toUtf8() }, m_dbNumber);
        if (result.isErrorMessage()) {
            message = "Could not add key";
        }
    } catch (const RedisClient::Connection::Exception& e) {
        message = "Connection error:" + QString(e.what());
    }

    return message;
}

QString HashKey::updateKey(const QString& v1, const QString& v2, const QString& oldv1, const QString& oldv2)
{

    Q_UNUSED(oldv2);

    RedisClient::Response result;
    QString message;

    try {
        if ((v1 != oldv1) && fieldExists(v1.toUtf8())) {
            message = "Field already exists!";
        } else {
            result = m_connection->commandSync({ "HDEL", m_keyFullPath, oldv1.toUtf8() }, m_dbNumber);
            result = m_connection->commandSync({ "HSET", m_keyFullPath, v1.toUtf8(), v2.toUtf8() }, m_dbNumber);
            if (result.isErrorMessage()) {
                message = "Could not update key";
                result = m_connection->commandSync({ "HSET", m_keyFullPath, oldv1.toUtf8(), oldv2.toUtf8() }, m_dbNumber);
            }
        }
    } catch (const RedisClient::Connection::Exception& e) {
        message = "Connection error:" + QString(e.what());
    }

    return message;
}

bool HashKey::fieldExists(const QByteArray& fieldName)
{
    RedisClient::Response result;
    try {
        result = m_connection->commandSync({ "HEXISTS", m_keyFullPath, fieldName }, m_dbNumber);
        if (result.isErrorMessage()) {
            return false;
        }
        return result.getValue().toInt() > 0;

    } catch (const RedisClient::Connection::Exception& e) {
        return false;
    }
}


QJsonObject HashKey::getKeyAsJsonObject()
{

    RedisClient::Response response = m_connection->commandSync({ "HGETALL", m_keyFullPath }, m_dbNumber);

    if (response.isErrorMessage())
        return QJsonObject();

    QVariantList data = response.getValue().toList();

    QList<QPair<QByteArray, QByteArray>> pairList = convertToListOfPairs(data);

    QJsonObject value = QJsonObject();
    QJsonObject keyData = QJsonObject();

    for (QPair<QByteArray, QByteArray> pair : pairList)
    {
        value.insert(pair.first, QJsonValue(QString(pair.second)));
    }

    keyData.insert("type", "hash");
    keyData.insert("value", value);

    return keyData;
}

void HashKey::importKeyFromJson(QJsonObject keyData) {

    if (!keyData.contains("value") || !keyData["value"].isObject())
        return;

    QJsonObject value = keyData["value"].toObject();
    QJsonObject::iterator it;
    RedisClient::Response response = m_connection->commandSync({ "DEL", m_keyFullPath }, m_dbNumber);
    QByteArray hashFieldValue;
    int fieldCount=0;
    for (it = value.begin(); it != value.end(); it++)
    {
        if (it.value().isString())
        {
            fieldCount++;
            hashFieldValue = it.value().toString().toUtf8();
            response = m_connection->commandSync({ "HSET", m_keyFullPath, it.key().toUtf8(), hashFieldValue }, m_dbNumber);
        }
    }

    if (keyData.contains("ttl") && fieldCount > 0)
    {
        response = m_connection->commandSync({ "EXPIRE", m_keyFullPath, keyData["ttl"].toString().toLatin1() },m_dbNumber);
    }


}

void HashKey::emitAllFinalSignals() {
    emit listDataChanged();
    emit isPaginatedChanged();
    emit rowsCountChanged();
}
