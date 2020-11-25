#include "serverinfo.h"

#include <QJsonArray>

ServerInfo::ServerInfo(QObject *parent) :
    QObject(parent),
    m_frozen(false),
    m_includeClientsList(false),
    m_busy(false),
    m_sort_column(0),
    m_sort_order(Qt::AscendingOrder),
    m_rawData("")
{
    m_updateTimer.setInterval(5000);
    m_updateTimer.setSingleShot(false);

    QObject::connect(&m_updateTimer, &QTimer::timeout, this, [this]{

        if (m_frozen)
            return;

        LOG(INFO) << "Collecting server information: " + m_connection->getConfig().host();

        // General information (Command: INFO all)
        QList<QByteArray> rawCmd {"INFO", "all"};
        m_connection->command(rawCmd, this, [this](RedisClient::Response r, QString err) {

            if (!err.isEmpty()) {
                emit error(QObject::tr("Cannot update server info. Error: %0").arg(err));
                return;
            }
            m_serverData = RedisClient::ServerInfo::fromString(r.getValue().toString()).parsed.toVariantMap();
            emit serverDataChanged();
        });


        // Clients list (Command: client list)
        if (m_includeClientsList) {
            rawCmd = {"client", "list"};
            m_connection->command(rawCmd, this, [this](RedisClient::Response r, QString err) {
                if (!err.isEmpty()) {
                    emit error(QObject::tr("Cannot update server info. Error: %0").arg(err));
                    return;
                }
                m_rawData = r.getValue().toString();
                m_clientsList = toJsonArray(m_rawData);
                emit clientsListChanged();
            });
        }
    });

}


ServerInfo::~ServerInfo(void)
{
}



QSharedPointer<RedisClient::Connection> ServerInfo::getConnection() const
{
    return m_connection;
}

void ServerInfo::setConnection(QSharedPointer<RedisClient::Connection>& arg)
{
    if (arg != m_connection) {
        m_connection = arg;
        emit connectionChanged();
    }
    m_config = m_connection->getConfig().toJsonObject();
    emit configChanged();
}


void ServerInfo::init()
{
    try {
        setBusy(true);
        if (!m_connection->connect())
        {
            emit error(QObject::tr("Connection error. Check network connection"));
            emit connectionFailed();
            setBusy(false);
            return;
        }

        m_updateTimer.start();
        setBusy(false);

    } catch (RedisClient::Connection::Exception&) {
        emit error(QObject::tr("Invalid Connection. Check connection settings."));
        emit connectionFailed();
        setBusy(false);
        return;
    }

    emit initialized();
}


QVariantMap ServerInfo::serverData()
{
    return m_serverData;
}

bool ServerInfo::frozen()
{
    return m_frozen;
}

void ServerInfo::setFrozen(bool arg)
{
    if (m_frozen != arg) {
        m_frozen = arg;
        emit frozenChanged();
    }
}

bool ServerInfo::includeClientsList()
{
    return m_includeClientsList;
}

void ServerInfo::setIncludeClientsList(bool arg)
{
    if (m_includeClientsList != arg) {
        m_includeClientsList = arg;
        emit includeClientsListChanged();
    }
}


QJsonArray ServerInfo::clientsList()
{
    return m_clientsList;
}

bool ServerInfo::busy()
{
    return m_busy;
}

void ServerInfo::setBusy(bool arg)
{
    if (m_busy != arg) {
        m_busy = arg;
        emit busyChanged();
    }
}

QJsonObject ServerInfo::config()
{
    return m_config;
}


QList<QList<QPair<QString, QString>>> ServerInfo::toListOfListOfPairs(const QString &data)
{
    QList<QList<QPair<QString, QString>>> result;

    QStringList lines = data.split("\n");
    QList<QPair<QString, QString>> properties;
    QStringList fieldPairs = {};
    QStringList parts;

    for (QString line : lines)
    {
        if (line.trimmed() == "")
            continue;

        properties = {};
        fieldPairs = line.split(" ");

        if (fieldPairs.count() <= 0)
            continue;
        int column = 0;
        for (QString fieldPair : fieldPairs)
        {
            parts = fieldPair.split("=");
            if (parts.count() < 2) continue;
            properties.push_back({parts[0], parts[1]});
            if (parts[0] == "addr")
                m_sort_column = column;
            column++;
        }
        result.push_back(properties);
    }

    int column = m_sort_column;
    int order = m_sort_order;


    std::sort(result.begin(), result.end(), [column, order](const QList<QPair<QString, QString>>& a, const QList<QPair<QString, QString>>& b) {

        switch (order) {
        case Qt::AscendingOrder:
                if (a[column].first == "age")
                    return a[column].second.toInt() < b[column].second.toInt();
                else
                    return a[column].second.toLower() < b[column].second.toLower();
        case Qt::DescendingOrder:
                return a[column].second.toLower() > b[column].second.toLower();
        }
        return false;
    });


    return result;
}


QJsonArray ServerInfo::toJsonArray(QString data) {

    QList<QList<QPair<QString, QString>>> listOfListOfPairs = toListOfListOfPairs(data);
    QJsonArray result = QJsonArray();

    for (QList<QPair<QString, QString>> row : listOfListOfPairs) {
        QJsonObject obj;
        for (QPair<QString, QString> item : row)
        {
            obj.insert(QString(item.first), QString(item.second));
        }
        result.append(obj);
    }
    return result;
}



