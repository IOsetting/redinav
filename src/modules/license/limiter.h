#pragma once
#include <QObject>
#include <qredisclient/connection.h>
#include "licensemanager.h"


class Limiter : public QObject
{
    Q_OBJECT
    ADD_EXCEPTION

public:

    explicit Limiter(QObject *parent = nullptr);

    enum Operation {
        None = 0,
        ImportConnections,
        ExportConnections,
        DumpDatabase,
        DumpKeys,
        DumpNamespaces,
        ImportKeys,
        AddNewKey,
        DeleteKey,
        DeleteManyKeys,
        FlushDatabase,
        CloneKey,
        RenameKey,
        ServerInfo
    };
    Q_ENUM(Operation)

    Q_INVOKABLE bool isActive() const;
    Q_INVOKABLE void setIsActive(bool isActive);
    Q_INVOKABLE bool canAccess(const Operation op = None);

    Q_PROPERTY(bool isActive READ isActive WRITE setIsActive NOTIFY isActiveChanged)

    QString ActiveReasonText() const;
    void setActiveReasonText(const QString &ActiveReasonText);


private:
    bool m_isActive;
    QString m_ActiveReasonText;

    QList<Operation> m_forbiddenOps = {
        ImportConnections,
        ExportConnections,
        DumpDatabase,
        DumpKeys,
        DumpNamespaces,
        ImportKeys,
        FlushDatabase,
        ServerInfo,
        CloneKey,
        AddNewKey,
        DeleteKey
    };


signals:
    void isActiveChanged();

public slots:
    void slotActivate();
    void slotDeactivate();
    void slotLicenseCheckStarted();
    void slotLicenseCheckFinished(LicenseManager::CheckOperation type, const bool success, const QString message = "");
};

