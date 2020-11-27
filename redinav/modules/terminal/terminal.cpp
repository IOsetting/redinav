#include "terminal.h"
#include <QObject>

#include <connection/connectionconf.h>

Terminal::Terminal(QObject *parent) : QObject(parent),
    m_workingDb(0),
    m_history({}),
    m_masterNodes({}),
    m_busy(true)
{
    addToHistory("");
}

Terminal::~Terminal()
{
    if (m_connection->isConnected())
    {
        m_connection->disconnect();
    }
}

void Terminal::connect()
{
    try {
        setBusy(true);
        if (!m_connection->connect())
        {
            LOG(ERROR) << "Connecting failed";
            emit connectionFailed();
            setBusy(false);
            return;
        }

        setDatabasesCount(getServerDatabasesCount());

        if (m_connection->mode() == RedisClient::Connection::Mode::Cluster)
        {
            m_masterNodes = m_connection->getMasterNodes();
            emit masterNodesChanged();
        }

        setBusy(false);
    } catch (RedisClient::Connection::Exception& e) {
        LOG(ERROR) << e.what();
        emit connectionFailed();
        setBusy(false);
        return;
    }
}

QSharedPointer<RedisClient::Connection> Terminal::connection() const
{
    return m_connection;
}

void Terminal::setConnection(const QSharedPointer<RedisClient::Connection> &connection)
{
    // Use a separate connection for terminal!!!
    ServerConfig config = connection->getConfig();
    m_connection = QSharedPointer<RedisClient::Connection>(new RedisClient::Connection(config));
    ServerConfig conf = config;
    conf.setOwner(m_connection.toWeakRef());
    m_connection->setConnectionConfig(conf);

    QObject::connect(m_connection.data(), &RedisClient::Connection::connected, this, &Terminal::slotConnectionConnected);

    m_originalHost = config.getOriginalHost();
    m_originalPort = config.getOriginalPort();

    emit configChanged();
}

bool Terminal::execCmd(const QString &input, const bool skipHistory)
{
    try
    {
        if (!skipHistory)
        {
            addToHistory(input);
        }

        RedisClient::Command command(RedisClient::Command::splitCommandString(input), workingDb());

        if (command.isSubscriptionCommand())
        {
            // inform the worls we are in subscription mode
            command.setCallBack(this, [this, input](RedisClient::Response result, QString err) {
                if (!err.isEmpty()) {
                    emit responseTaken(err);
                    return;
                }
                QVariant respValue = result.value();
                QString out = QString("channel message>\n%1\n").arg(RedisClient::Response::valueToHumanReadString(respValue));
                emit responseTaken(out);
            });
            m_connection->command(command);
            return true;
        }
        else
        {

            RedisClient::Response resp;
            resp = m_connection->commandSync(command);

            QVariant respValue = resp.value();
            QString out = QString("command > %1\n%2\n").arg(QString(input)).arg(RedisClient::Response::valueToHumanReadString(respValue));
            emit responseTaken(out);

            if (!resp.isErrorMessage() && (command.isSelectCommand() || m_connection->mode() == RedisClient::Connection::Mode::Cluster))
            {
                int requestedDb = command.getPartAsString(1).toInt();
                setWorkingDb(requestedDb);
            }
            return true;
        }
    }
    catch (const RedisClient::Connection::Exception& e)
    {
        emit responseTaken(e.what());
        LOG(ERROR) << e.what();
        return false;
    }

}

void Terminal::execCmdText(const QString &input, const bool skipHistory)
{
    QStringList list = input.split(QRegExp("[\r\n]"),QString::SkipEmptyParts);
    QStringListIterator i(list);

    QString cmd("");
    while (i.hasNext()) {
        cmd = i.next();
        if (!cmd.trimmed().isEmpty())
        {
            if (!execCmd(cmd, skipHistory))
            {
                break;
            }
        }
    }

}

int Terminal::workingDb() const
{
    return m_workingDb;
}

void Terminal::setWorkingDb(int workingDb)
{
    if (workingDb != m_workingDb) {
        m_workingDb = workingDb;
        emit dbChanged();
    }
}

QStringList Terminal::history() const
{
    return m_history;
}

void Terminal::execCommandsFile(QString filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }
    int currentWorkingDb = workingDb();
    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        if (!execCmd(line, true))
        {
            break;
        }
    }
    setWorkingDb(currentWorkingDb);
    file.close();
}

void Terminal::execLua(const QString content, QString params)
{
    if (content.isEmpty())
    {
        return;
    }

    try
    {

        QList<QByteArray> command({});

        if (params.trimmed().isEmpty())
        {
            params = "0";
        }

        QList<QByteArray> paramsList = RedisClient::Command::splitCommandString(params.trimmed());

        // In reverse order!
        command = paramsList;
        command.push_front(content.toUtf8());
        command.push_front("eval");

        RedisClient::Response resp = m_connection->commandSync(command, workingDb());
        QVariant respValue = resp.value();
        QString out = QString("command > eval <script> %1\n%2\n").arg(QString(paramsList.join(" "))).arg(RedisClient::Response::valueToHumanReadString(respValue));
        emit responseTaken(out);

        return;

    }
    catch (const RedisClient::Connection::Exception& e)
    {
        LOG(ERROR) << "Exception: " << e.what();
        return;
    }

}



void Terminal::execLuaFile(QString filePath, QString params)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        emit responseTaken("Could not open file");
        return;
    }
    else {
        try {
            QByteArray luaScript = file.readAll();
            return execLua(luaScript, params);
        }
        catch (const RedisClient::Connection::Exception& e)
        {
            LOG(ERROR) << "Exception: " << e.what();
            return;
        }
    }

}

QByteArray Terminal::loadFile(const QString filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        emit responseTaken("Could not open file");
        return "";
    }
    else {
        try {
            return file.readAll();
        }
        catch (const RedisClient::Connection::Exception& e)
        {
            LOG(ERROR) << "Exception: " << e.what();
            return "";
        }
    }
}

int Terminal::databasesCount() const
{
    return m_databasesCount;
}

void Terminal::setDatabasesCount(int databasesCount)
{
    if (m_databasesCount != databasesCount) {
        m_databasesCount = databasesCount;
        emit databaseCountChanged();
    }
}


void Terminal::addToHistory(QString command)
{
    m_history.append(command);
    emit historyChanged();
}

void Terminal::clearHistory()
{
    m_history.clear();
    emit historyChanged();
}

int Terminal::getServerDatabasesCount()
{
    QByteArrayList command = {"config", "get", "databases"};
    RedisClient::Response resp = m_connection->commandSync(command, 0);
    int result = 0;
    if (!resp.isErrorMessage())
    {
        QList<QVariant> list = resp.value().toList();
        result = 1;
        if (!list.isEmpty())
        {
            result = list.at(1).toInt();
        }
    }
    return result;
}

QJsonObject Terminal::config()
{
    if (m_connection)
    {
        return m_connection->getConfig().toJsonObject();
    }
    return QJsonObject();
}

int Terminal::getConnectionMode()
{
    if (m_connection && m_connection->isConnected())
    {
        return (int) m_connection->mode();
    }
    return 0;
}

bool Terminal::getBusy() const
{
    return m_busy;
}

void Terminal::setBusy(bool busy)
{
    if (m_busy != busy) {
        m_busy = busy;
        emit busyChanged();
    }
}

void Terminal::slotConnectionConnected()
{
    emit connectionModeChanged();
}


QJsonArray Terminal::getMasterNodesJson() const
{
    QJsonArray tmp;
    for (auto & node : m_masterNodes)
    {
        QJsonObject o;
        o.insert("host", node.first);
        o.insert("port", node.second);
        tmp.append(o);
    }
    return tmp;
}

void Terminal::reConnectTo(const int index)
{
    if (!m_connection || !m_connection->isConnected())
        return;

    if (index == 0)
    {
        m_connection->reconnectTo(m_originalHost, m_originalPort);
        LOG(INFO) << "Reconnecting to " << m_originalHost << ":" << m_originalPort;
    }
    else if (index <= m_masterNodes.count())
    {
        QString h = m_masterNodes.at(index-1).first;
        uint p = m_masterNodes.at(index-1).second;
        m_connection->reconnectTo(h, p);
        LOG(INFO) << "Reconnecting to " << h << ":" << p;
    }
    emit configChanged();
}



RedisClient::Connection::HostList Terminal::getMasterNodes() const
{
    return m_masterNodes;
}

void Terminal::setMasterNodes(const RedisClient::Connection::HostList &masterNodes)
{
    m_masterNodes = masterNodes;
}

QSharedPointer<Backend> Terminal::getBackend() const
{
    return m_backend;
}

void Terminal::setBackend(const QSharedPointer<Backend> &backend)
{
    if (backend != m_backend) {
        m_backend = backend;
        emit backendChanged();
    }

}



