#include "app.h"
#include <QCommandLineParser>
#include <QDebug>
#include <QDir>
#include <QDir>
#include <QFontDatabase>
#include <QIcon>
#include <QQmlContext>
#include <QQuickWindow>
#include <QSGRendererInterface>
#include <QNetworkProxyFactory>
#include <QSettings>

using namespace MainTree;

INITIALIZE_EASYLOGGINGPP

Application::Application(int& argc, char** argv)
    : QApplication(argc, argv)
    , m_engine(this)
    , m_renderingBackend("auto")
{

    setWindowIcon(QIcon(":resources/icons/128x128/redinav.png"));

    // Initializ m_configManager
    initConfigManager();

    // Functionality limiter
    initLimiter();

    // Init easylogging (AFTER config manager!!!)
    initLog();

    // License manager
    initLicenseManager();

    // Initialize font, take Application setting (platform based)
    initAppFonts();

    /*
    initProxySettings();
    */

    // QRedisClient
    initRedisClient();

    // Init "backend" object (it will load connections)
    initBackend();

    // Initialize Main Tree Model used in TreeView, load connections into it
    initMainTreeModel();

    // Connect various signals to slots
    connectSignals();

    // Finally, initialize QML stuff
    initQml();


    //installTranslator();


    m_licenseManager->checkLicenseStatus();


}

void Application::connectSignals()
{

    // Connect Connections Manager to Main Tree Model "connection updated" signal
    QObject::connect(
                m_backend,
                SIGNAL(connectionUpdated(QSharedPointer<RedisClient::Connection>)),
                m_maintreemodel,
                SLOT(onConnectionUpdated(QSharedPointer<RedisClient::Connection>)));

    QObject::connect(
                m_backend,
                SIGNAL(connectionRemoved(QSharedPointer<RedisClient::Connection>)),
                m_maintreemodel,
                SLOT(onConnectionRemoved(QSharedPointer<RedisClient::Connection>)));

    QObject::connect(
                m_backend,
                SIGNAL(databasesLoaded(QModelIndex, RedisClient::DatabaseList)),
                m_maintreemodel,
                SLOT(onDatabasesLoaded(QModelIndex, RedisClient::DatabaseList)));

    QObject::connect(
                m_backend,
                SIGNAL(connectionDisconnected(QModelIndex)),
                m_maintreemodel,
                SLOT(onConnectionDisconnected(QModelIndex)));

    QObject::connect(
                m_backend,
                SIGNAL(dbItemsLoaded(RedisClient::Connection::NamespaceItems, QModelIndex, ServerConfig)),
                m_maintreemodel,
                SLOT(onDbItemsLoaded(RedisClient::Connection::NamespaceItems, QModelIndex, ServerConfig)));

    QObject::connect(
                m_licenseManager,
                SIGNAL(licenseCheckStarted()),
                m_limiter,
                SLOT(slotLicenseCheckStarted()));

    QObject::connect(
                m_licenseManager,
                SIGNAL(licenseCheckFinished(LicenseManager::CheckOperation,bool,QString)),
                m_limiter,
                SLOT(slotLicenseCheckFinished(LicenseManager::CheckOperation,bool,QString)));



}


void Application::initConfigManager()
{
    m_configManager = QSharedPointer<ConfigManager>(new ConfigManager());
}

void Application::initAppFonts()
{
    QSettings settings;
#ifdef Q_OS_MACOS
    QString defaultFontName("Helvetica Neue");
    int defaultFontSize = 12;
#else
    QString defaultFontName("Open Sans");
    int defaultFontSize = 10;
#endif
    QString appFont = settings.value("app/appFont", defaultFontName).toString();
    int appFontSize = settings.value("app/appFontSize", defaultFontSize).toInt();
#ifdef Q_OS_LINUX
    if (appFont == "Open Sans") {
        int result = QFontDatabase::addApplicationFont("://resources/fonts/OpenSans.ttc");

        if (result == -1) {
            appFont = "Ubuntu";
        }
    }
#endif
    QFont defaultFont(appFont, appFontSize);
    QApplication::setFont(defaultFont);
}

void Application::initBackend()
{
    // Load connection descriptors from configuration file
    m_backend = new Backend(m_configManager, m_limiter);
}

void Application::initMainTreeModel()
{
    m_maintreemodel = new MainTree::TreeModel(m_backend->getConnections());
    m_backend->setTreemodel(m_maintreemodel);
}

void Application::initQml()
{

    registerQmlTypes();
    registerQmlRootObjects();

    try {
        m_engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));
    } catch (...) {
        qDebug() << "Failed to load app window. Retrying with software renderer...";
        QQuickWindow::setSceneGraphBackend(QSGRendererInterface::Software);
        m_engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));
    }

    // QLoggingCategory::setFilterRules("qt.scenegraph.general=true");

}

void Application::initProxySettings()
{
    QSettings settings;
    QNetworkProxyFactory::setUseSystemConfiguration(settings.value("app/useSystemProxy", false).toBool());
}

void Application::initLicenseManager()
{
    m_licenseManager = new LicenseManager((QObject *) this, m_configManager);
}

void Application::initLimiter()
{
    m_limiter = new Limiter((QObject *) this);
}

void Application::registerQmlTypes()
{
    qRegisterMetaType<ServerConfig>();
    qmlRegisterType<ServerInfo>("org.redinav.qml", 1, 0, "ServerInfo");
    qmlRegisterType<KeyEditor>("org.redinav.qml", 1, 0, "KeyEditor");
    qmlRegisterType<StringKey>("org.redinav.qml", 1, 0, "StringKey");
    qmlRegisterType<HashKey>("org.redinav.qml", 1, 0, "HashKey");
    qmlRegisterType<ListKey>("org.redinav.qml", 1, 0, "ListKey");
    qmlRegisterType<SetKey>("org.redinav.qml", 1, 0, "SetKey");
    qmlRegisterType<ZsetKey>("org.redinav.qml", 1, 0, "ZsetKey");
    qmlRegisterType<LicenseManager>("org.redinav.qml", 1, 0, "LicenseManager");
    qmlRegisterType<Limiter>("org.redinav.qml", 1, 0, "Limiter");
    qmlRegisterType<Terminal>("org.redinav.qml", 1, 0, "Terminal");
}

void Application::registerQmlRootObjects()
{
    m_engine.rootContext()->setContextProperty("mainTreeModel", m_maintreemodel);
    m_engine.rootContext()->setContextProperty("backend", m_backend);
    m_engine.rootContext()->setContextProperty("appLogger", m_logger);
    m_engine.rootContext()->setContextProperty("licenseManager", m_licenseManager);
    m_engine.rootContext()->setContextProperty("limiter", m_limiter);

}

QSharedPointer<ConfigManager> Application::configmanager() const
{
    return m_configManager;
}

void Application::initLog()
{

    QSettings settings;
    bool logToFile = settings.value("app/logToFile", "false").toBool();
    bool infoLog = settings.value("app/infoLog", "false").toBool();
    bool verboseLog = settings.value("app/verboseLog", "false").toBool();
#ifdef Q_OS_MACOS
    QString logFilePath = QDir::toNativeSeparators(QString("%1/Library/Logs/RediNav.log").arg(QDir::homePath()));
#else
    QString logFilePath = QDir::toNativeSeparators(QString("%1/log/%2").arg(m_configManager->getConfigDir()).arg("redinav.log"));
#endif

    el::Configurations defaultConf;
    defaultConf.setToDefault();

    defaultConf.setGlobally(el::ConfigurationType::ToStandardOutput, "true");

    // Log to file?
    defaultConf.setGlobally(el::ConfigurationType::ToFile, logToFile ? "true" : "false");
    defaultConf.setGlobally(el::ConfigurationType::Filename, logFilePath.toLatin1().data());
    defaultConf.setGlobally(el::ConfigurationType::MaxLogFileSize, "10000000");


    // Message formatting. Only for logging to stdout and file. The Logging Area (app UI) text is formatted by LogHandler!!!
    // Examples:
    //defaultConf.setGlobally(el::ConfigurationType::Format, "%datetime %level [%logger] %msg");   <---- this is the default one
    //defaultConf.setGlobally(el::ConfigurationType::Format, "[%datetime{%Y-%M-%d %H:%m:%s}] [%level] %msg");

    // Enable/disable various log levels
    defaultConf.set(el::Level::Info, el::ConfigurationType::Enabled, infoLog || verboseLog ? "true" : "false");
    defaultConf.set(el::Level::Debug, el::ConfigurationType::Enabled, verboseLog ? "true" : "false");
    defaultConf.set(el::Level::Trace, el::ConfigurationType::Enabled, verboseLog ? "true" : "false");

    // Configure logger
    el::Loggers::reconfigureLogger("default", defaultConf);

    el::Loggers::removeFlag(el::LoggingFlag::NewLineForContainer);
    el::Helpers::installLogDispatchCallback<LogHandler>("LogHandler");
    m_logger = el::Helpers::logDispatchCallback<LogHandler>("LogHandler");

    if (!m_logger) {
        LOG(ERROR) << "Failed to initialize logger";
    } else {
        LOG(INFO) << "Logger initialized";
    }



}

void Application::installTranslator()
{
    QSettings settings;
    QString preferredLocale = settings.value("app/locale", "system").toString();

    QString locale;

    if (preferredLocale == "system") {
        settings.setValue("app/locale", "system");
        locale = QLocale::system().uiLanguages().first().replace( "-", "_" );

        if (locale.isEmpty() || locale == "C")
            locale = "en_US";


    } else {
        locale = preferredLocale;
    }

    QTranslator* translator = new QTranslator((QObject *)this);
    if (translator->load( QString( ":/translations/redinav_" ) + locale ))
    {
        QCoreApplication::installTranslator( translator );
    } else {
        delete translator;
    }
}

void Application::setDefaultGlobalSettings() {

    const QMap<QString, QVariant> defaultSettings = {
        { "app/appTheme",                   "default" },
        { "app/appFont",                    QSysInfo::productType() == "osx"? "Helvetica Neue" : "Open Sans" },
        { "app/appFontSize",                QSysInfo::productType() == "osx"? "12" : "10" },
        { "app/defaultConnectionTimeout",   10 },
        { "app/disableFlushDb",             true },
        { "app/disableNamespaceDelete",     true },
        { "app/infoLog",                    true },
        { "app/verboseLog",                 false },
        { "app/verboseConnectionLog",       false },
        { "app/logToFile",                  false },
        { "app/doubleClickTreeItemActivation", false }
    };


    QSettings settings;
    QMap<QString, QVariant>::const_iterator it;

    for (it = defaultSettings.begin(); it != defaultSettings.end(); ++it)
    {
        if (!settings.contains(it.key()))
        {
            LOG(INFO) << "Apply default setting:" << it.key() << "=" << it.value().toString();
            settings.setValue(it.key(), it.value());
        }
    }
    settings.sync();

}
