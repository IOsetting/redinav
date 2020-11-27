#include "basekey.h"
#include "basekey.h"
#include "hashkey.h"
#include "listkey.h"
#include "setkey.h"
#include "stringkey.h"
#include "zsetkey.h"

BaseKey::BaseKey(const QSharedPointer<RedisClient::Connection> connection, const int& dbNumber, const QByteArray& keyName, const QString& type)
    : m_locked(false)
    , m_currentPage(1)
    , m_rowsCount(0)
    , m_search("")
    , m_value("")
{
    m_connection = connection;
    m_dbNumber = dbNumber;
    m_keyFullPath = keyName;
    m_keyType = type;
    m_isPaginated = (m_keyType == "list" || m_keyType == "zset");
    m_pageSize = 1000;
    m_listModelData = QJsonArray();
}

BaseKey::~BaseKey()
{
}

QSharedPointer<RedisClient::Connection> BaseKey::getConnection() const
{
    return m_connection;
}

void BaseKey::setConnection(QSharedPointer<RedisClient::Connection>& arg)
{
    m_connection = arg;
}

QByteArray BaseKey::getKey() const
{
    return m_keyFullPath;
}

void BaseKey::setKey(QByteArray arg)
{
    m_keyFullPath = arg;
}

int BaseKey::getDbNumber() const
{
    return m_dbNumber;
}

void BaseKey::setDbNumber(int arg)
{
    m_dbNumber = arg;
}

int BaseKey::pageSize() const
{
    return m_pageSize;
}

void BaseKey::setPageSize(int& arg)
{
    m_pageSize = arg;
}

int BaseKey::rowsCount()
{
    return m_rowsCount;
}

QJsonArray BaseKey::getListData()
{
    return m_listModelData;
}

QList<QVariant> BaseKey::rows()
{
    return m_rows;
}

void BaseKey::lock()
{
    m_locked = true;
}

void BaseKey::unlock()
{
    m_locked = false;
}

bool BaseKey::isLocked()
{
    return m_locked;
}

int BaseKey::currentPage()
{
    return m_currentPage;
}

void BaseKey::setCurrentPage(int& arg)
{
    m_currentPage = arg;
}

QByteArray BaseKey::getValue()
{
    return m_value;
}

void BaseKey::emptyRows()
{
    while (m_rows.count() > 0) {
        m_rows.pop_back();
    }
}

void BaseKey::emptyPairList()
{
    while (m_pairList.count() > 0) {
        m_pairList.pop_back();
    }
}

bool BaseKey::isPaginated()
{
    return m_isPaginated;
}

QList<QPair<QByteArray, QByteArray> > BaseKey::convertToListOfPairs(const QVariantList& rows)
{
    QList<QPair<QByteArray, QByteArray> > result;

    for (QVariantList::const_iterator item = rows.begin();
         item != rows.end(); ++item) {

        QPair<QByteArray, QByteArray> value;
        value.first = item->toByteArray();
        ++item;

        if (item == rows.end()) {
            LOG(ERROR) << "=======================================================";
            LOG(ERROR) << QObject::tr("Partially loaded Key Data!!");
            LOG(ERROR) << "=======================================================";
            break;
        }

        value.second = item->toByteArray();

        result.push_back(value);
    }

    return result;
}

QSharedPointer<BaseKey> BaseKey::factory(const QSharedPointer<RedisClient::Connection> connection, const int& dbNumber, const QByteArray& keyName, const QString& type)
{
    if (type == "string")
        return QSharedPointer<StringKey>(new StringKey(Q_NULLPTR, connection, dbNumber, keyName));
    else if (type == "hash")
        return QSharedPointer<HashKey>(new HashKey(Q_NULLPTR, connection, dbNumber, keyName));
    else if (type == "list")
        return QSharedPointer<ListKey>(new ListKey(Q_NULLPTR, connection, dbNumber, keyName));
    else if (type == "set")
        return QSharedPointer<SetKey>(new SetKey(Q_NULLPTR, connection, dbNumber, keyName));
    else if (type == "zset")
        return QSharedPointer<ZsetKey>(new ZsetKey(Q_NULLPTR, connection, dbNumber, keyName));
    else
        return nullptr;
}

QString BaseKey::addKey(const QString& v1, const QString& v2)
{
    Q_UNUSED(v1);
    Q_UNUSED(v2);
    return "";
}

QString BaseKey::updateKey(const QString& v1, const QString& v2, const QString& oldv1, const QString& oldv2)
{
    Q_UNUSED(v1);
    Q_UNUSED(v2);
    Q_UNUSED(oldv1);
    Q_UNUSED(oldv2);
    return "";
}

QString BaseKey::addRow(const QString& v1, const QString& v2)
{
    Q_UNUSED(v1);
    Q_UNUSED(v2);
    return "";
}

bool BaseKey::keyExists(QSharedPointer<RedisClient::Connection> connection, int& dbNumber, QString& keyName)
{
    RedisClient::Response result = connection->commandSync({ "EXISTS", keyName.toUtf8() }, dbNumber);
    return result.value().toInt() > 0;
}

QJsonObject BaseKey::getKeyAsJsonObject()
{
    QJsonObject(obj);
    return obj;
}

void BaseKey::importKeyFromJson(QJsonObject keyData)
{
    LOG(WARNING) << "Import key from jsom not implemented yet: " << m_dbNumber << "->" << keyData["type"].toString();
}
