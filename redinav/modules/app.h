#pragma once
#include "keyeditor/hashkey.h"
#include "keyeditor/keyeditor.h"
#include "keyeditor/listkey.h"
#include "keyeditor/setkey.h"
#include "keyeditor/stringkey.h"
#include "keyeditor/zsetkey.h"
#include "connection/serverinfo.h"
#include "connection/configmanager.h"
#include "version.h"
#include "backend.h"
#include "logger.h"
#include "license/licensemanager.h"
#include "license/limiter.h"
#include "terminal/terminal.h"
#include <maintree/treemodel.h>
#include <qredisclient/redisclient.h>
#include <QApplication>
#include <QQmlApplicationEngine>
#include <easylogging++.h>


class Application : public QApplication {
    Q_OBJECT

public:
    Application(int& argc, char** argv);
    QSharedPointer<ConfigManager> configmanager() const;
    static void setDefaultGlobalSettings();

private slots:

private:
    void initMainTreeModel();
    void initQml();
    void initLog();
    void initBackend();

    void registerQmlTypes();
    void registerQmlRootObjects();

    QQmlApplicationEngine m_engine;
    LogHandler* m_logger;
    Backend* m_backend;
    QSharedPointer<ConfigManager> m_configManager;
    TreeModel* m_maintreemodel;
    QString m_renderingBackend;
    LicenseManager *m_licenseManager;
    Limiter *m_limiter;

    void initAppFonts();
    void connectSignals();
    void initConfigManager();
    void installTranslator();
    void initProxySettings();
    void initLicenseManager();
    void initLimiter();
};
