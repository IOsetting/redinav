#include "limiter.h"
#include "easylogging++.h"

Limiter::Limiter(QObject *parent) : QObject(parent),
    m_isActive(false)
{

}

bool Limiter::isActive() const
{
#if defined(SKIP_LIMITER)
    return false;
#else
    return m_isActive;
#endif
}

void Limiter::setIsActive(bool isActive)
{
    if (m_isActive != isActive)
    {
        m_isActive = isActive;
        emit isActiveChanged();
    }
}

QString Limiter::ActiveReasonText() const
{
    return m_ActiveReasonText;
}

void Limiter::setActiveReasonText(const QString &ActiveReasonText)
{
    m_ActiveReasonText = ActiveReasonText;
}

bool Limiter::canAccess(const Limiter::Operation op)
{

    Q_UNUSED(op);
    if (!isActive() || op == Operation::None) {
        return true;
    }

    if (m_forbiddenOps.contains(op))
    {
        return false;
    }
    return true;
}

void Limiter::slotActivate()
{
    setIsActive(true);
}

void Limiter::slotDeactivate()
{
    setIsActive(false);
}

void Limiter::slotLicenseCheckStarted()
{
    setActiveReasonText("Valid and activated license required");
    setIsActive(true);
}

void Limiter::slotLicenseCheckFinished(LicenseManager::CheckOperation type, const bool success, const QString message)
{
    Q_UNUSED(type);
    setIsActive(!success);
    m_ActiveReasonText = message;
}

