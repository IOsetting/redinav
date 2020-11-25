#pragma once
#include "basekey.h"
#include <QAbstractListModel>
#include <QJsonArray>
#include <QList>
#include <QObject>
#include <easylogging++.h>
#include <qredisclient/connection.h>

class ListKey : public QObject, public BaseKey {
    // Common
    Q_PROPERTY(QSharedPointer<RedisClient::Connection> connection READ getConnection WRITE setConnection)
    Q_PROPERTY(QByteArray keyFullPath READ getKey WRITE setKey)
    Q_PROPERTY(int dbNumber READ getDbNumber WRITE setDbNumber)

    // LIST like specific
    Q_PROPERTY(int pageSize READ pageSize WRITE setPageSize NOTIFY pageSizeChanged)
    Q_PROPERTY(int currentPage READ currentPage WRITE setCurrentPage NOTIFY currentPageChanged)
    Q_PROPERTY(int rowsCount READ rowsCount NOTIFY rowsCountChanged)
    Q_PROPERTY(QJsonArray listData READ getListData NOTIFY listDataChanged)
    Q_PROPERTY(int isPaginated READ isPaginated NOTIFY isPaginatedChanged)

    Q_OBJECT
    ADD_EXCEPTION

public:
    explicit ListKey(QObject* parent = nullptr, const QSharedPointer<RedisClient::Connection> connection = Q_NULLPTR, const int& dbNumber = 0, const QByteArray& keyName = "");
    ~ListKey();

    Q_INVOKABLE void loadRawKeyData(const int& page = 1, const QString& search = "");
    Q_INVOKABLE void buildListData();
    Q_INVOKABLE void removeRows(QList<int> rowNumbers, QList<QString> rowKeys, QList<QString> rowValues);
    Q_INVOKABLE QString addRow(const QString& v1, const QString& v2) override;
    Q_INVOKABLE QString updateKey(const QString& v1, const QString& v2, const QString& oldv1, const QString& oldv2) override;

    QString addKey(const QString& v1, const QString& v2) override;
    QJsonObject getKeyAsJsonObject() override;
    void importKeyFromJson(QJsonObject keyData) override;

signals:
    void listDataChanged();
    void pageSizeChanged();
    void rowsCountChanged();
    void currentPageChanged();
    void isPaginatedChanged();
    void keyRemoved();

public slots:

protected:
private:
    void loadRowsCount();
    void deleteRowsByValue(int count, const QByteArray& value);
    void setListRow(int pos, const QByteArray& value);
    bool isActualPositionChanged(int row, QByteArray currentValue);
    void emitAllFinalSignals();
};
