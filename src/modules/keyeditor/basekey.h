#pragma once
#include <QByteArray>
#include <QJsonArray>
#include <QList>
#include <easylogging++.h>
#include <qredisclient/connection.h>

class BaseKey {

    ADD_EXCEPTION

public:
    BaseKey(const QSharedPointer<RedisClient::Connection> connection = Q_NULLPTR, const int& dbNumber = 0, const QByteArray& keyName = "", const QString& type = "");
    ~BaseKey();

    // Connection
    QSharedPointer<RedisClient::Connection> getConnection() const;
    void setConnection(QSharedPointer<RedisClient::Connection>& arg);

    // Key FULL PATH
    QByteArray getKey() const;
    void setKey(QByteArray arg);

    // DB Number
    int getDbNumber() const;
    void setDbNumber(int arg);

    // --- FOR single value keys
    QByteArray getValue();

    // --- FOR LIST like keys
    int pageSize() const;
    void setPageSize(int& arg);
    QJsonArray getListData();
    int rowsCount();
    QList<QVariant> rows();

    void emptyRows();
    void emptyPairList();

    bool isPaginated();

    QList<QPair<QByteArray, QByteArray> > convertToListOfPairs(const QVariantList& rows);

    QString getType() { return m_keyType; }

    virtual QString addKey(const QString& v1, const QString& v2);

    virtual QString updateKey(const QString& v1, const QString& v2, const QString& oldv1, const QString& oldv2);

    virtual QString addRow(const QString& v1, const QString& v2);

    virtual QJsonObject getKeyAsJsonObject();

    virtual void importKeyFromJson(QJsonObject keyData);

    static QSharedPointer<BaseKey> factory(const QSharedPointer<RedisClient::Connection> connection, const int& dbNumber, const QByteArray& keyName, const QString& type);

protected:
    QByteArray m_keyFullPath;
    QSharedPointer<RedisClient::Connection> m_connection;
    int m_dbNumber;
    QString m_keyType;

    bool m_locked = false;
    int m_currentPage;

    /**
     * TOTAL number of rows in the key (members in lists, rows in HASH keys, etc.). NOT filtered amount!!!
     * @brief m_rowsCount
     */
    int m_rowsCount;

    /**
     * Currently applied (or to apply) search text
     * @brief m_search
     */
    QString m_search;

    /**
     * The String value of the key
     * @brief m_value
     */
    QByteArray m_value;

    /**
     * For HASH keys, list of pairs (key -> value)
     * @brief m_pairList
     */
    QList<QPair<QByteArray, QByteArray> > m_pairList;

    // --- For LIST like keys (list, set, zset, hash)
    /**
     * Raw data loaded initially from Redis
     * @brief m_rows
     */
    QVariantList m_rows;

    /**
     * Page size, if key data is going to be paginated
     * @brief m_pageSize
     */
    int m_pageSize;

    /**
     * QML's TableView "model" structure (JSON for now)
     *
     * @brief m_listModelData
     */
    QJsonArray m_listModelData;

    bool m_isPaginated;

    void lock();
    void unlock();
    bool isLocked();

    int currentPage();
    void setCurrentPage(int& arg);
    bool keyExists(QSharedPointer<RedisClient::Connection> connection, int& dbNumber, QString& keyName);
};
