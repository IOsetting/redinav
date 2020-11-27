#include "listkey.h"
#include <QDebug>
#include <QJsonArray>
#include <QObject>
#include <algorithm>

/**
 * Constructor
 * @brief ListKey::ListKey
 * @param parent
 */
ListKey::ListKey(QObject* parent, const QSharedPointer<RedisClient::Connection> connection, const int& dbNumber, const QByteArray& keyName)
    : QObject(parent)
    , BaseKey(connection, dbNumber, keyName, "list")
{
}

/**
 * Destructor
 * @brief ListKey::~ListKey
 */
ListKey::~ListKey()
{
}

/**
 * Call Redis and fill object's data (m_rows, m_rowsCount, m_listModelData)
 *
 * @brief ListKey::loadRawKeyData
 */
void ListKey::loadRawKeyData(const int& page, const QString& search)
{

    if (isLocked()) {
        emit currentPageChanged();
        return;
    }

    lock();

    m_currentPage = page;
    emit currentPageChanged();

    loadRowsCount();

    RedisClient::Response result;
    int start;

    if (search.size() > 0) {
        m_isPaginated = false;
        int cursorPage = 1;
        int i;
        bool keepRunning = true;
        int maxResults = m_pageSize;
        QVariantList pageRows = QVariantList();
        QVariant member;
        int chunk = 50000;
        int memberIndex;

        emptyRows();
        emptyPairList();

        while (keepRunning) {
            start = (cursorPage - 1) * chunk;
            result = m_connection->commandSync({ "LRANGE", m_keyFullPath, QByteArray::number(start), QByteArray::number(start + chunk - 1) }, m_dbNumber);
            pageRows = result.value().toList();
            for (i = 0; i < pageRows.count(); i++) {
                // Search and add only members having the search string
                member = pageRows.at(i);
                memberIndex = start + i;
                if (member.toString().toLower().indexOf(search.toLower()) != -1) {
                    m_pairList.push_back(QPair<QByteArray, QByteArray>(QByteArray::number(memberIndex), member.toByteArray()));
                }
                if (m_pairList.count() >= maxResults)
                    break;
            }
            cursorPage++;
            keepRunning = (pageRows.count() > 0) && (m_pairList.count() < maxResults);
        }

        m_rowsCount = m_pairList.count();

        buildListData();
        unlock();

    } else {

        m_isPaginated = true;
        emptyRows();
        emptyPairList();

        try {
            start = (m_currentPage - 1) * m_pageSize;
            m_connection->command({ "LRANGE", m_keyFullPath, QByteArray::number(start), QByteArray::number(start + m_pageSize - 1) }, this,
                [this, start](RedisClient::Response result, QString err) {

                    int memberIndex;

                    bool failed = false;
                    if (result.type() != RedisClient::Response::Array) {
                        LOG(ERROR) << "Unexpected response type: " << result.type() << " (expected: " << RedisClient::Response::Array << ")";
                        failed = true;
                    }
                    if (!err.isEmpty()) {
                        LOG(ERROR) << "Cannot load value: " << err;
                        failed = true;
                    }

                    if (failed) {
                        unlock();
                        emit rowsCountChanged();
                        emit listDataChanged();
                        return;
                    }

                    QList<QVariant> tmpList = result.value().toList();

                    for (int i = 0; i < tmpList.count(); i++) {
                        memberIndex = start + i;
                        m_pairList.push_back(QPair<QByteArray, QByteArray>(QByteArray::number(memberIndex), tmpList.at(i).toByteArray()));
                    }

                    buildListData();
                    unlock();

                },
                m_dbNumber);
        } catch (const RedisClient::Connection::Exception& e) {
            unlock();
            emitAllFinalSignals();
            LOG(ERROR) << "Connection error: " << QString(e.what());
        }
    }
}

/**
 * Get the number of members in the list. No search applied whatsoever
 *
 * @brief ListKey::loadRowsCount
 */
void ListKey::loadRowsCount()
{
    RedisClient::Response result;

    try {
        result = m_connection->commandSync({ "LLEN", m_keyFullPath }, m_dbNumber);
        if (result.type() == RedisClient::Response::Integer) {
            m_rowsCount = result.value().toUInt();
        }
        else {
            LOG(ERROR) << "Invalid response type";
        }
    } catch (const RedisClient::Connection::Exception& e) {
        LOG(ERROR) << "Connection error: " << QString(e.what());
    }

    emit rowsCountChanged();
}

void ListKey::buildListData()
{

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

void ListKey::removeRows(QList<int> rowNumbers, QList<QString> rowKeys, QList<QString> rowValues)
{

    Q_UNUSED(rowKeys);
    Q_UNUSED(rowValues);

    int actualRow = 0;
    QByteArray memberValue;
    QString customSystemValue("---VALUE_REMOVED_BY_REDINAV---");
    QList<int> rowsToRemove = {};

    for (int rowNumber : rowNumbers) {
        memberValue = m_pairList.at(rowNumber).second;
        actualRow = m_pairList.at(rowNumber).first.toInt();
        setListRow(actualRow, customSystemValue.toUtf8());
        rowsToRemove.push_back(rowNumber);
    }

    // Remove all system values from list (in Redis)
    deleteRowsByValue(0, customSystemValue.toUtf8());

    std::sort(rowsToRemove.begin(), rowsToRemove.end());
    while (rowsToRemove.count() > 0) {
        m_pairList.removeAt(rowsToRemove.last());
        rowsToRemove.pop_back();
        m_rowsCount--;
    }

    // Key is gone ?
    QString tmp = QString::fromUtf8(m_keyFullPath);
    if (!keyExists(m_connection, m_dbNumber, tmp)) {
        emit keyRemoved();
    }

    buildListData();
}

void ListKey::deleteRowsByValue(int count, const QByteArray& value)
{
    try {
        m_connection->commandSync({ "LREM", m_keyFullPath, QString::number(count).toLatin1(), value }, m_dbNumber);
    } catch (const RedisClient::Connection::Exception& e) {
        LOG(ERROR) << QObject::tr("Connection error: ") << QString(e.what());
    }
}

void ListKey::setListRow(int pos, const QByteArray& value)
{
    try {
        m_connection->commandSync({ "LSET", m_keyFullPath, QString::number(pos).toLatin1(), value }, m_dbNumber);
    } catch (const RedisClient::Connection::Exception& e) {
        LOG(ERROR) << QObject::tr("Connection error: ") << QString(e.what());
    }
}

bool ListKey::isActualPositionChanged(int row, QByteArray currentValue)
{
    using namespace RedisClient;

    Response result;

    try {
        result = m_connection->commandSync({ "LRANGE", m_keyFullPath, QString::number(row).toLatin1(), QString::number(row).toLatin1() }, m_dbNumber);
    } catch (const RedisClient::Connection::Exception& e) {
        LOG(ERROR) << QObject::tr("Connection error: ") << QString(e.what());
        return true;
    }

    QVariantList currentState = result.value().toList();

    return currentState.size() != 1 || currentState[0].toByteArray() != QString(currentValue);
}

QString ListKey::addKey(const QString& v1, const QString& v2)
{

    Q_UNUSED(v1);

    RedisClient::Response result;
    QString message;
    try {
        result = m_connection->commandSync({ "LPUSH", m_keyFullPath, v2.toUtf8() }, m_dbNumber);
        if (result.isErrorMessage()) {
            message = "Could not add key";
        }
    } catch (const RedisClient::Connection::Exception& e) {
        message = "Connection error:" + QString(e.what());
    }

    return "";
}

QString ListKey::updateKey(const QString& v1, const QString& v2, const QString& oldv1, const QString& oldv2)
{

    Q_UNUSED(oldv1);
    Q_UNUSED(oldv2);

    // v1: LIST index
    // v2: member value

    RedisClient::Response result;
    QString message;
    try {
        result = m_connection->commandSync({ "LSET", m_keyFullPath, v1.toUtf8(), v2.toUtf8() }, m_dbNumber);
        if (result.isErrorMessage()) {
            message = "Could not update key";
        }
    } catch (const RedisClient::Connection::Exception& e) {
        message = "Connection error:" + QString(e.what());
    }

    return "";
}

QString ListKey::addRow(const QString& v1, const QString& v2)
{

    Q_UNUSED(v1);

    RedisClient::Response result;
    QString message;
    try {
        result = m_connection->commandSync({ "RPUSH", m_keyFullPath, v2.toUtf8() }, m_dbNumber);
        if (result.isErrorMessage()) {
            message = "Could not add row to a key";
        } else {
            int memberIndex = m_rowsCount;
            m_pairList.push_back(QPair<QByteArray, QByteArray>(QByteArray::number(memberIndex), v2.toUtf8()));
            m_rowsCount++;
            buildListData();
        }
    } catch (const RedisClient::Connection::Exception& e) {
        message = "Connection error:" + QString(e.what());
    }

    return message;
}


QJsonObject ListKey::getKeyAsJsonObject()
{
    QJsonObject keyData = QJsonObject();

    RedisClient::Response response = m_connection->commandSync({ "LRANGE", m_keyFullPath, "0", "-1" }, m_dbNumber);
    QJsonArray data = response.value().toJsonArray();

    keyData.insert("type", "list");
    keyData.insert("value", data);

    return keyData;

}


void ListKey::importKeyFromJson(QJsonObject keyData) {

    if (!keyData.contains("value") || !keyData["value"].isArray())
        return;

    LOG(INFO) << "Import LIST key in DB " << m_dbNumber << ": " << m_keyFullPath;


    QJsonArray value = keyData["value"].toArray();

    if (value.count() <= 0)
        return;

    RedisClient::Response response = m_connection->commandSync({ "DEL", m_keyFullPath }, m_dbNumber);

    QList<QByteArray> rawCmd = {"RPUSH", m_keyFullPath};

    foreach (QJsonValue member, value) {
        rawCmd.push_back(member.toString().toUtf8());
    }

    response = m_connection->commandSync(rawCmd, m_dbNumber);

    if (keyData.contains("ttl"))
    {
        response = m_connection->commandSync({ "EXPIRE", m_keyFullPath, keyData["ttl"].toString().toLatin1() },m_dbNumber);
    }


}

void ListKey::emitAllFinalSignals() {
    emit listDataChanged();
    emit isPaginatedChanged();
    emit rowsCountChanged();
}
