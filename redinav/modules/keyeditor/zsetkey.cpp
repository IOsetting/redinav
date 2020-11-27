#include "zsetkey.h"
#include <QDebug>
#include <QDoubleValidator>
#include <QJsonArray>
#include <QObject>
#include <algorithm>
#include <backend.h>
#include <helpers.h>

/**
 * Constructor
 * @brief ZsetKey::ZsetKey
 * @param parent
 */
ZsetKey::ZsetKey(QObject* parent, const QSharedPointer<RedisClient::Connection> connection, const int& dbNumber, const QByteArray& keyName)
    : QObject(parent)
    , BaseKey(connection, dbNumber, keyName, "zset")
{
}

/**
 * Destructor
 * @brief ZsetKey::~ZsetKey
 */
ZsetKey::~ZsetKey()
{
}

/**
 * Call Redis and fill object's data (m_rows, m_rowsCount, m_listModelData)
 *
 * @brief ZsetKey::loadRawKeyData
 */
void ZsetKey::loadRawKeyData(const int& page, const QString& search)
{

    if (isLocked()) {
        emit currentPageChanged();
        return;
    }

    m_isPaginated = true;

    lock();

    m_currentPage = page;

    // To update the UI to current page
    emit currentPageChanged();

    loadRowsCount();

    RedisClient::Response result;

    if (search.size() > 0) {
        m_isPaginated = false;
        QList<QByteArray> cmdParts = { "ZSCAN", m_keyFullPath, "0", "MATCH", "*" + search.toUtf8() + "*", "COUNT", "50000" };
        RedisClient::ScanCommand cmd(cmdParts, m_dbNumber);
        try {
            emptyRows();
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
            unlock();
            emitAllFinalSignals();
            LOG(ERROR) << e.what();
        }
    } else {
        try {
            int start = (m_currentPage - 1) * m_pageSize;
            result = m_connection->commandSync({ "ZRANGE", m_keyFullPath, QByteArray::number(start), QByteArray::number(start + m_pageSize - 1), "WITHSCORES" }, m_dbNumber);
            m_rows = result.value().toList();
            m_pairList = convertToListOfPairs(m_rows);
            buildListData(0, Qt::AscendingOrder);
            unlock();
        } catch (const RedisClient::Connection::Exception& e) {
            unlock();
            emitAllFinalSignals();
            LOG(ERROR) << "Connection error: " << e.what();
        }
    }
}

/**
 * Get the number of members in the list. No search applied whatsoever
 *
 * @brief ZsetKey::loadRowsCount
 */
void ZsetKey::loadRowsCount()
{
    RedisClient::Response result;

    try {
        result = m_connection->commandSync({ "ZCARD", m_keyFullPath }, m_dbNumber);
        if (result.type() == RedisClient::Response::Integer) {
            m_rowsCount = result.value().toUInt();
        }
        else {
            throw RedisClient::Connection::Exception("Invalid response type");
        }

    } catch (const RedisClient::Connection::Exception& e) {
        LOG(ERROR) << QString(e.what());
    }

    emit rowsCountChanged();
}

void ZsetKey::buildListData(int column, Qt::SortOrder order)
{

    std::sort(m_pairList.begin(), m_pairList.end(), [this, column, order](const QPair<QByteArray, QByteArray>& a, const QPair<QByteArray, QByteArray>& b) {
        switch (order) {
        case Qt::AscendingOrder:
            if (column == 0)
                return a.first.toLower() < b.first.toLower();
            else
                return a.second.toDouble() < b.second.toDouble();
        case Qt::DescendingOrder:
            if (column == 0)
                return a.first.toLower() > b.first.toLower();
            else
                return a.second.toDouble() > b.second.toDouble();
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

void ZsetKey::removeRow(int i)
{
    QByteArray value = m_pairList.at(i).first;

    try {
        m_connection->commandSync({ "ZREM", m_keyFullPath, value }, m_dbNumber);
    } catch (const RedisClient::Connection::Exception& e) {
        LOG(ERROR) << QObject::tr("Connection error: ") << QString(e.what());
        return;
    }

    m_rowsCount--;
    m_pairList.removeAt(i);

    // Key is gone
    if (m_pairList.count() <= 0) {
        emit keyRemoved();
    }

    buildListData();
}

void ZsetKey::removeRows(QList<int> rowNumbers, QList<QString> rowKeys, QList<QString> rowValues)
{

    Q_UNUSED(rowNumbers);

    try {
        int pairIndex;
        QList<QByteArray> rawCmd = { "ZREM", m_keyFullPath };
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

QString ZsetKey::updateKey(const QString& v1, const QString& v2, const QString& oldv1, const QString& oldv2)
{

    // v1 = member
    // v2 = score

    Q_UNUSED(oldv2);

    RedisClient::Response result;
    QString message;
    QString score = v2;
    if (score.trimmed() == "") {
        score = "0";
    }

    try {

        if ((v1 != oldv1) && memberExists(v1.toUtf8())) {
            message = "Member already exists!";
        } else if (!Helpers::isDouble(score)) {
            message = "Invalid Score";
        } else {
            result = m_connection->commandSync({ "ZREM", m_keyFullPath, oldv1.toUtf8() }, m_dbNumber);
            result = m_connection->commandSync({ "ZADD", m_keyFullPath, score.toUtf8(), v1.toUtf8() }, m_dbNumber);
            if (result.isErrorMessage()) {
                message = "Could not update key";
                // Sort of rollback
                result = m_connection->commandSync({ "ZADD", m_keyFullPath, oldv2.toUtf8(), oldv1.toUtf8() }, m_dbNumber);
            }
        }
    } catch (const RedisClient::Connection::Exception& e) {
        message = "Connection error:" + QString(e.what());
    }

    return message;
}

QString ZsetKey::addRow(const QString& v1, const QString& v2)
{

    // v1 = member
    // v2 = score

    if (memberExists(v1.toUtf8())) {
        return "Member already exists";
    }

    RedisClient::Response result;
    QString message;
    QString score = v2;
    if (score.trimmed() == "") {
        score = "0";
    }

    try {
        if (!Helpers::isDouble(score)) {
            message = "Invalid score";
        } else {
            result = m_connection->commandSync({ "ZADD", m_keyFullPath, "NX", score.toUtf8(), v1.toUtf8() }, m_dbNumber);
            if (result.isErrorMessage()) {
                message = "Could not add row to a key";
            } else {
                m_rowsCount++;
                m_pairList.push_back(QPair<QByteArray, QByteArray>(v1.toUtf8(), score.toUtf8()));
                buildListData(0, Qt::AscendingOrder);
            }
        }
    } catch (const RedisClient::Connection::Exception& e) {
        message = "Connection error:" + QString(e.what());
    }

    return message;
}

QString ZsetKey::addKey(const QString& v1, const QString& v2)
{

    // v1 = member
    // v2 = score

    RedisClient::Response result;
    QString message;
    QString score = v2;
    if (score.trimmed() == "") {
        score = "0";
    }
    try {
        if (!Helpers::isDouble(score)) {
            message = "Invalid score";
        } else {
            result = m_connection->commandSync({ "ZADD", m_keyFullPath, score.toUtf8(), v1.toUtf8() }, m_dbNumber);
            if (result.isErrorMessage()) {
                message = "Could not add key";
            }
        }
    } catch (const RedisClient::Connection::Exception& e) {
        message = "Connection error:" + QString(e.what());
    }

    return message;
}

bool ZsetKey::memberExists(const QByteArray& memberValue)
{
    RedisClient::Response result;
    try {
        result = m_connection->commandSync({ "ZRANK", m_keyFullPath, memberValue }, m_dbNumber);
        if (result.isErrorMessage()) {
            return false;
        }
        return result.value().isValid();
    } catch (const RedisClient::Connection::Exception& e) {
        return false;
    }
}


QJsonObject ZsetKey::getKeyAsJsonObject()
{
    RedisClient::Response response = m_connection->commandSync({ "ZRANGE", m_keyFullPath, "0", "-1", "WITHSCORES" }, m_dbNumber);
    QVariantList data = response.value().toList();
    QList<QPair<QByteArray, QByteArray>> pairList = convertToListOfPairs(data);

    QJsonArray value = QJsonArray();
    QJsonArray member = QJsonArray();
    QJsonObject keyData = QJsonObject();
    for (QPair<QByteArray, QByteArray> pair : pairList)
    {
        member.append(QJsonValue(QString::number(pair.second.toDouble(), 'f', 6)));
        member.append(QJsonValue(QString(pair.first)));

        value.append(member);

        member.pop_back();
        member.pop_back();
    }

    keyData.insert("type", "zset");
    keyData.insert("value", value);

    return keyData;
}

void ZsetKey::importKeyFromJson(QJsonObject keyData) {

    if (!keyData.contains("value") || !keyData["value"].isArray())
        return;

    LOG(INFO) << "Import ZSET key in DB " << m_dbNumber << ": " << m_keyFullPath;

    QJsonArray value = keyData["value"].toArray();

    if (value.count() <= 0)
        return;

    RedisClient::Response response = m_connection->commandSync({ "DEL", m_keyFullPath }, m_dbNumber);

    QList<QByteArray> rawCmd = {"ZADD", m_keyFullPath};

    QJsonArray memberPair;
    int counter = 0;
    foreach (QJsonValue member, value) {
        memberPair = member.toArray();
        if (memberPair.count() == 2) {
            rawCmd.push_back(memberPair[0].toString().toUtf8());
            rawCmd.push_back(memberPair[1].toString().toUtf8());
            counter++;
        }
    }

    if (counter > 0) {
        response = m_connection->commandSync(rawCmd, m_dbNumber);
        if (keyData.contains("ttl"))
        {
            response = m_connection->commandSync({ "EXPIRE", m_keyFullPath, keyData["ttl"].toString().toLatin1() },m_dbNumber);
        }
    }


}

void ZsetKey::emitAllFinalSignals() {
    emit listDataChanged();
    emit isPaginatedChanged();
    emit rowsCountChanged();
}
