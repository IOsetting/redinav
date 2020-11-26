#pragma once
#include <QSysInfo>
#include <easylogging++.h>
#include <QNetworkReply>
#include <QObject>

#include <connection/configmanager.h>

#ifndef REDINAV_URL
    #define REDINAV_URL "https://hmm.ahoydummy.net/"
#endif


#define VALIDATION_FILE_NAME "license.bin"
#define UPDATES_CHECK_INTERVAL 7*24*3600 // weekly
#define DEFAULT_REMOTE_VALIDATION_PERIOD 24*3600  // daily

class LicenseManager : public QObject
{
    Q_OBJECT
    ADD_EXCEPTION

public:

    enum CheckOperation
    {
        Activation=1,
        Validation,
        Deactivation
    };
    Q_ENUM(CheckOperation)

    explicit LicenseManager(QObject *parent = nullptr, QSharedPointer<ConfigManager> configManager = nullptr);

    Q_INVOKABLE bool requireLicense();
    Q_INVOKABLE void activate();
    Q_INVOKABLE void deActivate();
    Q_INVOKABLE void validate();
    Q_INVOKABLE void validate(const bool forceRemoteValidation, const bool forceUpdatesCheck);
    Q_INVOKABLE QString licenseKey() const;
    Q_INVOKABLE void setLicenseKey(const QString &licenseKey);
    Q_INVOKABLE void checkLicenseStatus();
    Q_INVOKABLE bool isValid() const;
    Q_INVOKABLE bool requireActivation();

    Q_PROPERTY(bool valid READ isValid() NOTIFY isValidChanged)


    Q_INVOKABLE  uint expire() const;
    void setExpire(const uint &expire);

    Q_INVOKABLE QString keyExpireDate() const;
    void setKeyExpireDate(const QString &keyExpireDate);

    Q_INVOKABLE bool keyHasExpired() const;
    void setKeyHasExpired(bool keyHasExpired);

    Q_INVOKABLE QByteArray keyUrl() const;
    void setKeyUrl(const QByteArray &keyUrl);

    Q_INVOKABLE int daysLeft() const;
    void setDaysLeft(const uint &daysLeft);

    Q_INVOKABLE QString storeUrl() const;
    void setStoreUrl(const QString &storeUrl);

    Q_INVOKABLE QString supportEmail() const;
    void setSupportEmail(const QString &supportEmail);

private:
    void extractMachineId();

    void saveLicenseValidation(QByteArray validationPayload);
    void loadLicenseBin();
    bool validateLicenseBin(QByteArray payload);
    bool checkResponse(QByteArray payload);
    bool checkValidationResponse(QByteArray payload);
    bool checkActivationResponse(QByteArray payload);
    bool checkDectivationResponse(QByteArray payload);
    bool licenseBinExists();
    void removeLicenseBin();
    void setIsValid(bool isValid);
    bool requireRemoteLicenseValidation();
    bool isUpdateAvailable(QByteArray payload, bool force, QString &newVersion);

    QString m_storeCode;
    QString m_sku;
    QByteArray m_machineId;
    quint64 m_machineIdHashed;
    QByteArray m_apiBaseUrl;
    QString m_activationId;
    QSharedPointer<ConfigManager> m_configManager;
    QString m_licenseBinFilepath;
    QString m_licenseKey;
    QString m_domain;
    bool m_isValid;
    QString m_storeUrl;
    QString m_supportEmail;

    uint m_expire;  // seconds since epoch
    QString m_keyExpireDate;
    QByteArray m_keyUrl;
    bool m_keyHasExpired;
    QString m_keyStatus;
    bool m_keyAllowOffline;
    QString m_keyOfflineInterval;
    int m_keyOfflineValue;
    uint m_lastValidated;
    int m_daysLeft;
    uint m_lastUpdateCheck;

#ifdef Q_OS_MACOS
    QString m_licenseBinFilename = QString(ENV_MODE) == QString("production") ? "redinav-license.bin" : QString("redinav-license-%1.bin").arg(ENV_MODE);
#else
    QString m_licenseBinFilename = QString(ENV_MODE) == QString("production") ? "license.bin" : QString("license-%1.bin").arg(ENV_MODE);
#endif


    QNetworkAccessManager *m_netManager;
    QNetworkRequest m_request;


signals:
    void requestLicenseKeyActivation();
    void licenseCheckStarted();
    void licenseCheckFinished(const LicenseManager::CheckOperation type, const bool success, const QString message="");
    void isValidChanged();
    void updatesChecked(const bool isAvail=false, const QString newVersion="", const QString downloadUrl="");

public slots:
    void slotNetManagerFinished(QNetworkReply*reply);

    void slotActivateFinished();
    void slotDeactivateFinished();
    void slotValidateFinished(const bool forceUpdateCheck=false);


};



