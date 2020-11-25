#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkInterface>
#include <QDateTime>
#include <QString>
#include <QUrlQuery>
#include <QRegularExpression>
#include <QVersionNumber>
#include "licensemanager.h"
#include "simplecrypt.h"
#include "version.h"

LicenseManager::LicenseManager(QObject *parent, QSharedPointer<ConfigManager> configManager) : QObject(parent),
    m_storeCode(""),
    m_sku(""),
    m_machineId(""),
    m_machineIdHashed(0),
    m_apiBaseUrl(""),
    m_activationId(""),
    m_configManager(configManager),
    m_domain(""),
    m_isValid(false),

    // Activation/Validation payload data attributes
    m_keyExpireDate(""),
    m_keyUrl(""),
    m_keyHasExpired(false),
    m_keyStatus("active"),
    m_keyAllowOffline(false),
    m_keyOfflineInterval("days"),
    m_keyOfflineValue(1),
    m_lastUpdateCheck(0)

{
    extractMachineId();

    m_licenseBinFilepath = QString("%1/%2").arg(m_configManager->getConfigDir()).arg(m_licenseBinFilename);

#ifdef LICENSE_STORE_CODE
    m_storeCode = LICENSE_STORE_CODE;
#else
    m_storeCode = "dummy";
#endif

#ifdef LICENSE_PRODUCT_SKU
    m_sku = LICENSE_PRODUCT_SKU;
#else
    m_sku = "dummy";
#endif

#ifdef LICENSE_API_BASE_URL
    m_apiBaseUrl = LICENSE_API_BASE_URL;
#else
    m_apiBaseUrl = "dummy";
#endif

#ifdef SUPPORT_EMAIL
    m_supportEmail = SUPPORT_EMAIL;
#else
    m_supportEmail = "dummy@dummy.com";
#endif

    m_storeUrl = REDINAV_URL;


    // Manager is created once and stay until the end of the App live
    m_netManager = new QNetworkAccessManager(this);
    connect(m_netManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(slotNetManagerFinished(QNetworkReply*)));

    m_request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    m_request.setUrl(QUrl(m_apiBaseUrl));


}


bool LicenseManager::requireLicense()
{
#if defined(SKIP_LICENSE_CHECK)
    return false;
#else
    return true;
#endif
}


void LicenseManager::activate()
{

    QUrlQuery postData;
    postData.addQueryItem("action", "license_key_activate");
    postData.addQueryItem("store_code", m_storeCode);
    postData.addQueryItem("sku", m_sku);
    postData.addQueryItem("license_key", m_licenseKey);
    postData.addQueryItem("domain", "");
    postData.addQueryItem("activation_id", "");

    connect(m_netManager->post(m_request, postData.toString(QUrl::FullyEncoded).toUtf8()), SIGNAL(finished()), this, SLOT(slotActivateFinished()));

}

void LicenseManager::deActivate()
{
    try
    {
        loadLicenseBin();
    }
    catch (Exception e)
    {
        emit licenseCheckFinished(CheckOperation::Deactivation, false, e.what());
        return;
    }

    QUrlQuery postData;
    postData.addQueryItem("action", "license_key_deactivate");
    postData.addQueryItem("store_code", m_storeCode);
    postData.addQueryItem("sku", m_sku);
    postData.addQueryItem("license_key", m_licenseKey);
    postData.addQueryItem("domain", "");
    postData.addQueryItem("activation_id", m_activationId);

    connect(m_netManager->post(m_request, postData.toString(QUrl::FullyEncoded).toUtf8()), SIGNAL(finished()), this, SLOT(slotDeactivateFinished()));

}


void LicenseManager::validate(const bool forceRemoteValidation, const bool forceUpdatesCheck)
{
    try
    {
        loadLicenseBin();
    }
    catch (Exception e)
    {
//        emit licenseCheckFinished(CheckOperation::Validation, false,
//            "License problem - switching to restricted mode! Check log for additional information. "
//            "You can continue using the program, but some limitations apply, mostly write-operations. "
//            "To unlock all features, please activate a valid license.");
        LOG(ERROR) << e.what();
        return;
    }

    if (!requireRemoteLicenseValidation() && !forceRemoteValidation) {
        setIsValid(true);
        LOG(INFO) << "License validation success (local)";
        emit licenseCheckFinished(CheckOperation::Validation, true);
        return;
    }
    else
    {
        LOG(INFO) << "Validate license remotely";
    }

    QUrlQuery postData;
    postData.addQueryItem("action", "license_key_validate");
    postData.addQueryItem("store_code", m_storeCode);
    postData.addQueryItem("sku", m_sku);
    postData.addQueryItem("license_key", m_licenseKey);
    postData.addQueryItem("domain", "");
    postData.addQueryItem("activation_id", m_activationId);

    connect(
        m_netManager->post(m_request, postData.toString(QUrl::FullyEncoded).toUtf8()), &QNetworkReply::finished,
        this, [this, forceUpdatesCheck](){
            slotValidateFinished(forceUpdatesCheck);
        }
    );
}


void LicenseManager::validate()
{
    validate(false,false);
}


void LicenseManager::extractMachineId()
{
    m_machineId = QSysInfo::machineUniqueId();

    if (m_machineId.isEmpty())
    {
        foreach(QNetworkInterface netInterface, QNetworkInterface::allInterfaces())
        {
            if (!(netInterface.flags() & QNetworkInterface::IsLoopBack))
            {
                m_machineId = netInterface.hardwareAddress().toUtf8();
                if (!m_machineId.isEmpty())
                {
                    break;
                }
            }
        }
    }

    if (m_machineId.isEmpty())
    {
        m_machineId = "machine-id-not-available";
    }

    m_machineId = m_machineId + "varna"; // just a little more ...

    std::hash<std::string> hasher;
    m_machineIdHashed = hasher(m_machineId.toStdString());

}

void LicenseManager::saveLicenseValidation(QByteArray validationPayload)
{
    QJsonDocument jsonDoc = QJsonDocument::fromJson(validationPayload);
    if (jsonDoc.isNull())
    {
        throw Exception("Invalid validation response");
    }

    QJsonObject jsonObj = jsonDoc.object();
    jsonObj.insert("last_validated", QJsonValue::fromVariant(QDateTime::currentDateTimeUtc().toTime_t()));
    jsonObj.insert("last_update_check", QJsonValue::fromVariant(m_lastUpdateCheck));

    QJsonDocument finalJsonDoc(jsonObj);

    SimpleCrypt crypto(m_machineIdHashed);
    QString encrypted = crypto.encryptToString(finalJsonDoc.toJson());
    QFile file(m_licenseBinFilepath);
    if (file.open(QIODevice::WriteOnly))
    {
        QTextStream stream(&file);
        stream << encrypted.toUtf8();
        stream.flush();
        file.flush();
        file.close();
    }
}


bool LicenseManager::licenseBinExists()
{
    QFile file(m_licenseBinFilepath);
    return file.exists();
}

void LicenseManager::removeLicenseBin()
{
    QFile file(m_licenseBinFilepath);
    file.remove();
}

bool LicenseManager::isValid() const
{
    return m_isValid;
}

void LicenseManager::setIsValid(bool isValid)
{
    if (isValid != m_isValid) {
        m_isValid = isValid;
        emit isValidChanged();
    }
}

bool LicenseManager::requireRemoteLicenseValidation()
{

    // If it seems like an expired one (judging from locally saved license data), validate remotely
    if (daysLeft() < 0)
    {
        return true;
    }

    if (m_keyAllowOffline && (m_keyOfflineInterval == "unlimited"))
    {
        return false;
    }

    // Default period for remote validation (after last validation)
    int allowedOfflineSeconds = (int) DEFAULT_REMOTE_VALIDATION_PERIOD;


    if (m_keyAllowOffline) {

        if (m_keyOfflineInterval == "minutes")
        {
            allowedOfflineSeconds = m_keyOfflineValue * (60);
        }
        else if (m_keyOfflineInterval == "days")
        {
            allowedOfflineSeconds = m_keyOfflineValue * (24*3600);
        }
        else if (m_keyOfflineInterval == "months")
        {
            allowedOfflineSeconds = m_keyOfflineValue * (24*3600*30);
        }
        else if (m_keyOfflineInterval == "years")
        {
            allowedOfflineSeconds = m_keyOfflineValue * (24*3600*365);
        }
    }

    return (QDateTime::currentDateTimeUtc().toTime_t() - m_lastValidated) > allowedOfflineSeconds;


}

bool LicenseManager::isUpdateAvailable(QByteArray payload, bool force, QString &newVersion)
{
    uint currentTime = QDateTime::currentDateTimeUtc().toTime_t();
    if (((currentTime - m_lastUpdateCheck) < (uint) UPDATES_CHECK_INTERVAL) && !force)
    {
        return false;
    }

    m_lastUpdateCheck = currentTime;

    QJsonDocument jsonDoc = QJsonDocument::fromJson(payload);
    if (jsonDoc.isNull() || !jsonDoc.object().contains("data") || !jsonDoc.object().value("data").toObject().contains("downloadable"))
    {
        return false;
    }
    QJsonObject downloadableObj = jsonDoc.object().value("data").toObject().value("downloadable").toObject();
    QString url = downloadableObj.value("url").toString();

    if (url.isEmpty() || url.isNull())
    {
        return false;
    }

    QRegularExpression re("(\\d+)\\.(\\d+)\\.(\\d+)");

    // Remote version
    QRegularExpressionMatch match = re.match(url);
    if (!match.hasMatch())
    {
        return false;
    }
    QStringList capts = match.capturedTexts();
    if (capts.count() != 4)
    {
        return false;
    }
    QVersionNumber remoteVersion(QString(capts[1]).toInt(), QString(capts[2]).toInt(), QString(capts[3]).toInt());

    // THIS version
    match = re.match(REDINAV_VERSION);
    QStringList capts1 = match.capturedTexts();
    if (capts1.count() != 4)
    {
        return false;
    }
    QVersionNumber currentVersion(QString(capts1[1]).toInt(), QString(capts1[2]).toInt(), QString(capts1[3]).toInt());

    if (QVersionNumber::compare(remoteVersion, currentVersion) > 0)
    {
        LOG(INFO) << "New version is available: " << capts[0];
        newVersion = capts[0];
        return true;
    }

    return false;

}

QString LicenseManager::supportEmail() const
{
    return m_supportEmail;
}

void LicenseManager::setSupportEmail(const QString &supportEmail)
{
    m_supportEmail = supportEmail;
}

QString LicenseManager::storeUrl() const
{
    return m_storeUrl;
}

void LicenseManager::setStoreUrl(const QString &storeUrl)
{
    m_storeUrl = storeUrl;
}

int LicenseManager::daysLeft() const
{
    return m_daysLeft;
}

void LicenseManager::setDaysLeft(const uint &daysLeft)
{
    m_daysLeft = daysLeft;
}

QByteArray LicenseManager::keyUrl() const
{
    return m_keyUrl;
}

void LicenseManager::setKeyUrl(const QByteArray &keyUrl)
{
    m_keyUrl = keyUrl;
}

bool LicenseManager::keyHasExpired() const
{
    return m_keyHasExpired;
}

void LicenseManager::setKeyHasExpired(bool keyHasExpired)
{
    m_keyHasExpired = keyHasExpired;
}

QString LicenseManager::keyExpireDate() const
{
    return m_keyExpireDate;
}

void LicenseManager::setKeyExpireDate(const QString &keyExpireDate)
{
    m_keyExpireDate = keyExpireDate;
}

uint LicenseManager::expire() const
{
    return m_expire;
}

void LicenseManager::setExpire(const uint &expire)
{
    m_expire = expire;
}

bool LicenseManager::requireActivation()
{
    return requireLicense() && !isValid();
}

void LicenseManager::loadLicenseBin()
{
    QFile file(m_licenseBinFilepath);
    if (file.open(QIODevice::ReadOnly))
    {
        QTextStream in(&file);
        QString encrypted = in.readAll();
        in.flush();
        file.flush();
        file.close();
        SimpleCrypt crypto(m_machineIdHashed);
        validateLicenseBin(crypto.decryptToByteArray(encrypted));
    }
    else
    {
        throw Exception("Could not open license file");
    }
}

bool LicenseManager::validateLicenseBin(QByteArray payload)
{
    QJsonDocument jsonDoc = QJsonDocument::fromJson(payload);
    if (jsonDoc.isNull())
    {
        throw Exception("Invalid license");
    }
    QJsonObject jsonObj = jsonDoc.object();

    if (jsonObj.contains("data"))
    {
        QJsonObject dataObj = jsonObj.value("data").toObject();
        //qDebug() << dataObj;
        // Taken from server, during Activation or Validation
        m_activationId          = QString::number(dataObj.value("activation_id").toInt());
        m_expire                = dataObj.value("expire").toInt();
        m_keyExpireDate         = dataObj.value("expire_date").toString();
        m_licenseKey            = dataObj.value("the_key").toString();
        m_keyUrl                = dataObj.value("url").toString().toUtf8();
        m_keyHasExpired         = dataObj.value("has_expired").toBool();
        m_keyStatus             = dataObj.value("status").toString();
        m_keyAllowOffline       = dataObj.value("allow_offline").toBool();
        m_keyOfflineInterval    = dataObj.value("offline_interval").toString();
        m_keyOfflineValue       = dataObj.value("offline_value").toInt();

        // NOT IN "data" !!
        m_lastValidated         = jsonObj.value("last_validated").toInt();
        m_lastUpdateCheck       = jsonObj.value("last_update_check").toInt();

        // Calculated
        setDaysLeft(qRound((double) ((m_expire - QDateTime::currentDateTimeUtc().toTime_t()) / (24*3600))));

    }
    else
    {
        throw Exception("Invalid license");
    }

    if (
            m_activationId.isEmpty() ||
            m_licenseKey.isEmpty()
       )
    {
        throw Exception("Invalid license");
    }
    return true;
}

bool LicenseManager::checkResponse(QByteArray payload)
{
    QJsonDocument jsonDoc = QJsonDocument::fromJson(payload);

    if (jsonDoc.isNull())
    {
        LOG(ERROR) << "Invalid response";
        return false;
    }

    QJsonObject jsonObj = jsonDoc.object();
    if (jsonObj.value("error").toBool() == true)
    {
        QJsonObject errorsObj = jsonObj.value("errors").toObject();
        QJsonObject::iterator it;
        QVariantList errorStrings;
        for (it = errorsObj.begin(); it != errorsObj.end(); it++)
        {
            errorStrings = it.value().toArray().toVariantList();
            for (QVariantList::const_iterator item = errorStrings.begin(); item != errorStrings.end(); ++item)
            {
                LOG(ERROR) << item->toString();
            }
        }
        return false;
    }

    QJsonObject dataObj = jsonDoc.object().value("data").toObject();
    if (dataObj.isEmpty())
    {
        LOG(ERROR) << "Invalid response";
        return false;
    }

    return true;


}

bool LicenseManager::checkValidationResponse(QByteArray payload)
{
    return checkResponse(payload);
}

bool LicenseManager::checkActivationResponse(QByteArray payload)
{
    return checkResponse(payload);
}


bool LicenseManager::checkDectivationResponse(QByteArray payload)
{
    return checkResponse(payload);
}

QString LicenseManager::licenseKey() const
{
    return m_licenseKey;
}

void LicenseManager::setLicenseKey(const QString &licenseKey)
{
    m_licenseKey = licenseKey;
}

void LicenseManager::checkLicenseStatus()
{
    if (requireLicense())
    {
        emit licenseCheckStarted();
        validate();
    }
}

void LicenseManager::slotNetManagerFinished(QNetworkReply *reply)
{
    // Important!
    reply->deleteLater();
}


void LicenseManager::slotActivateFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());
    if (reply->error() == QNetworkReply::NoError)
    {
        QByteArray response = reply->readAll();
        QString errorString = "";
        bool success = checkActivationResponse(response);
        if (success)
        {
            if (licenseBinExists())
            {
                removeLicenseBin();
            }
            saveLicenseValidation(response);
            setIsValid(true);
            loadLicenseBin();
            emit licenseCheckFinished(CheckOperation::Activation, true);
            LOG(INFO) << "License activation success";
        }
        else
        {
            emit licenseCheckFinished(CheckOperation::Activation, false,
                                      QString(
                                      "License activation failed. Check the log for additional information. "
                                      "Chances are you exceeded the number of allowed activations or the license is not a valid one (wrong key, expired). "
                                      "If activations limit is reached, visit <a href=\"%1/my-account/license-keys/\">RediNav website</a>, log in and deactivate some of the activations. "
                                      "Remember, your license key activation is tied to a device. "
                                      "In case you need to activate it on more devices, no worries, contact us at <a href=\"%1/support/\">Support page</a> and we shall add one for free!"
                                      ).arg(m_storeUrl)
                                      );
        }
    }
    else
    {
        QByteArray message =
                "Unable to activate a license. Please check your network connection. \n"
                "Switching to restricted mode. If the problem persists, please contact us at Support page and provide the log content.";
        LOG(ERROR) << "Network error: " << reply->error();
        emit licenseCheckFinished(CheckOperation::Activation, false, message);
    }
}

void LicenseManager::slotDeactivateFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());
    if (reply->error() == QNetworkReply::NoError)
    {
        QByteArray response = reply->readAll();
        bool success = checkDectivationResponse(response);
        if (success)
        {
            LOG(INFO) << "License deactivation success";
            emit licenseCheckFinished(CheckOperation::Deactivation, true);
        }
        else
        {
            emit licenseCheckFinished(CheckOperation::Deactivation, false, "License deactivation failed. Check the log for additional information.");
        }
    }
    else
    {
        emit licenseCheckFinished(CheckOperation::Deactivation, false, "Unable to deactivate your license. Please check your network connection.");
    }
}

void LicenseManager::slotValidateFinished(const bool forceUpdateCheck)
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());
    if (reply->error() == QNetworkReply::NoError)
    {
        QByteArray response = reply->readAll();
        bool success = checkValidationResponse(response);
        if (success)
        {
            QString newVersion = "";
            QString downloadsUrl = QString("%1/my-account/downloads/").arg(m_storeUrl);

            bool updateAvail = isUpdateAvailable(response, forceUpdateCheck, newVersion);

            saveLicenseValidation(response);
            setIsValid(true);
            loadLicenseBin();

            if (updateAvail)
            {
                emit updatesChecked(true, newVersion, downloadsUrl);
            }
            else
            {
                emit updatesChecked(false);
            }
            emit licenseCheckFinished(CheckOperation::Validation, true);
            LOG(INFO) << "License validation passed";
        }
        else
        {
            setIsValid(false);
            emit licenseCheckFinished(CheckOperation::Validation, false,
                                      QString(
                                      "License validation failed. Check log for additional information. "
                                      "If you see a message like 'Invalid activation', you probably dactivated the license for this device. "
                                      "Activate it again and if you are not exceeding the number of activations, everything should be fine! "
                                      "If the rpoblem persists, please contact us at <a href=\"%1/support/\">Support page</a> to resolve the issue. "
                                      ).arg(m_storeUrl)
                                      );
            LOG(ERROR) << "License validation failed";
        }

    }
    else
    {
        QByteArray message =
                "Unable to validate a license. Please check your network connection. "
                "Switching to restricted mode. If the problem persists, please contact us at Support page and provide the log content.";
        LOG(ERROR) << "Network error: " << reply->error();
        emit licenseCheckFinished(CheckOperation::Activation, false, message);
    }
}


