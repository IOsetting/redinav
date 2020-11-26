#include "modules/app.h"
#include <QCoreApplication>
#include <QDebug>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QUrl>
#include <QSslSocket>
#include <QStyleFactory>
#include <QPalette>
#include <QFont>
#include <QSettings>

int main(int argc, char* argv[])
{

#if defined(Q_OS_WIN) || defined(Q_OS_LINUX)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    qApp->setApplicationName(APPLICATION_NAME);
    qApp->setApplicationVersion(QString(REDINAV_VERSION));
    qApp->setOrganizationDomain(ORG_DOMAIN);
    qApp->setOrganizationName(ORG_NAME);

    Application::setDefaultGlobalSettings();

    QSettings globalSettings;
    QString theme = globalSettings.value("app/appTheme", "default").toString();
    QPalette palette;



    if (theme == "dark")
    {
        // styles: macintosh, Fusion, Windows
        qApp->setStyle(QStyleFactory::create("Fusion"));

        // Active
        palette.setColor(QPalette::AlternateBase,       QColor(0x4B, 0x4B, 0x4B));
        palette.setColor(QPalette::Base,                QColor(0x46, 0x46, 0x46));
        palette.setColor(QPalette::BrightText,          Qt::red);
        palette.setColor(QPalette::Button,              QColor(0x53, 0x53, 0x53));
        palette.setColor(QPalette::ButtonText,          Qt::white);
        palette.setColor(QPalette::Dark,                QColor(0x41, 0x41, 0x41));
        palette.setColor(QPalette::Highlight,           QColor(0x2A, 0x82, 0xDA));
        palette.setColor(QPalette::HighlightedText,     Qt::white);
        palette.setColor(QPalette::Light,               QColor(0x46, 0x46, 0x46));
        palette.setColor(QPalette::Link,                QColor(0x93, 0xC9, 0xD6));
        palette.setColor(QPalette::LinkVisited,         QColor(0x93, 0xC9, 0xD6));
        palette.setColor(QPalette::Mid,                 QColor(0x31, 0x31, 0x31));
        palette.setColor(QPalette::Midlight,            QColor(0x77, 0x77, 0x77));
        palette.setColor(QPalette::Shadow,              QColor(0x10, 0x10, 0x10));
        palette.setColor(QPalette::Text,                Qt::white);
        palette.setColor(QPalette::ToolTipBase,         QColor(0x50, 0x50, 0x50));
        palette.setColor(QPalette::ToolTipText,         Qt::white);
        palette.setColor(QPalette::Window,              QColor(0x46, 0x46, 0x46));
        palette.setColor(QPalette::WindowText,          Qt::white);

        // Disabled
        palette.setColor(QPalette::Disabled,    QPalette::ButtonText,       QColor(0x9D, 0x9D, 0x9D));
        palette.setColor(QPalette::Disabled,    QPalette::Highlight,        QColor(0x6E, 0x6E, 0x6E));
        palette.setColor(QPalette::Disabled,    QPalette::HighlightedText,  QColor(0x9D, 0x9D, 0x9D));
        palette.setColor(QPalette::Disabled,    QPalette::Text,             QColor(0x9D, 0x9D, 0x9D));
        palette.setColor(QPalette::Disabled,    QPalette::WindowText,       QColor(0x9D, 0x9D, 0x9D));

        // Inactive



        // ---

        qApp->setPalette(palette);

    }
    // 'default' and 'lite' are the same (Mojave dark mode issues)
    else
    {
        // styles: macintosh, Fusion, Windows
        qApp->setStyle(QStyleFactory::create("Fusion"));

        // Active
        palette.setColor(QPalette::AlternateBase,       QColor(0xf7, 0xf7, 0xf7));
        palette.setColor(QPalette::Base,                QColor(0xff, 0xff, 0xff));
        palette.setColor(QPalette::BrightText,          Qt::red);
        palette.setColor(QPalette::Button,              QColor(0xef, 0xef, 0xef));
        palette.setColor(QPalette::ButtonText,          Qt::black);
        palette.setColor(QPalette::Dark,                QColor(0x9f, 0x9f, 0x9f));
        palette.setColor(QPalette::Highlight,           QColor(0x30, 0x8c, 0xc6));
        palette.setColor(QPalette::HighlightedText,     Qt::white);
        palette.setColor(QPalette::Light,               QColor(0xff, 0xff, 0xff));
        palette.setColor(QPalette::Link,                Qt::blue);
        palette.setColor(QPalette::LinkVisited,         Qt::magenta);
        palette.setColor(QPalette::Mid,                 QColor(0xb8, 0xb8, 0xb8));
        palette.setColor(QPalette::Midlight,            QColor(0x77, 0x77, 0x77));
        palette.setColor(QPalette::Shadow,              QColor(0x76, 0x76, 0x76));
        palette.setColor(QPalette::Text,                Qt::black);
        palette.setColor(QPalette::ToolTipBase,         QColor(0x50, 0x50, 0x50));
        palette.setColor(QPalette::ToolTipText,         Qt::white);
        palette.setColor(QPalette::Window,              QColor(0xef, 0xef, 0xef));
        palette.setColor(QPalette::WindowText,          Qt::black);

        // Disabled
        palette.setColor(QPalette::Disabled,    QPalette::ButtonText,       QColor(0xbe, 0xbe, 0xbe));
        palette.setColor(QPalette::Disabled,    QPalette::Highlight,        QColor(0x91, 0x91, 0x91));
        palette.setColor(QPalette::Disabled,    QPalette::Text,             QColor(0xbe, 0xbe, 0xbe));
        palette.setColor(QPalette::Disabled,    QPalette::WindowText,       QColor(0xbe, 0xbe, 0xbe));

        // Inactive
        // .. same as Active


        // ---

        qApp->setPalette(palette);

    }


    Application app(argc, argv);

    return app.exec();
}

