#include <QCoreApplication>
#include <QDebug>

#include "core/connectionsessionstate.h"

namespace
{
bool normalLifecycle()
{
    ConnectionSessionState session;
    if (!session.requestStart({false, 1000})
        || session.state() != ConnectionState::Starting
        || !session.isActive())
    {
        qCritical() << "normalLifecycle failed at start";
        return false;
    }

    session.connectionEstablished();
    if (session.state() != ConnectionState::Running)
    {
        qCritical() << "normalLifecycle failed at running";
        return false;
    }

    if (!session.requestStop() || session.state() != ConnectionState::Stopping)
    {
        qCritical() << "normalLifecycle failed at stopping";
        return false;
    }

    if (session.processFinished() != ProcessFinishAction::Complete
        || session.state() != ConnectionState::Disconnected
        || session.isActive())
    {
        qCritical() << "normalLifecycle failed at completion";
        return false;
    }
    return true;
}

bool reconnectsOnlyEligibleFailures()
{
    ConnectionSessionState session;
    session.requestStart({true, 2500});
    session.connectionEstablished();
    session.recordError(ZJU_ERROR::AUTH_EXPIRED);

    if (session.processFinished() != ProcessFinishAction::Reconnect
        || session.state() != ConnectionState::Reconnecting
        || session.reconnectDelayMs() != 2500)
    {
        qCritical() << "reconnectsOnlyEligibleFailures failed at scheduling";
        return false;
    }

    session.beginReconnect();
    if (session.state() != ConnectionState::Starting || session.error() != ZJU_ERROR::NONE)
    {
        qCritical() << "reconnectsOnlyEligibleFailures failed at restart";
        return false;
    }

    session.recordError(ZJU_ERROR::INVALID_DETAIL);
    if (session.processFinished() != ProcessFinishAction::Complete
        || session.state() != ConnectionState::Failed)
    {
        qCritical() << "reconnectsOnlyEligibleFailures reconnected an ineligible error";
        return false;
    }
    return true;
}

bool keepsFirstErrorAndCancelsPendingReconnect()
{
    ConnectionSessionState session;
    session.requestStart({true, 1000});
    session.connectionEstablished();
    session.recordError(ZJU_ERROR::AUTH_EXPIRED);
    session.recordError(ZJU_ERROR::OTHER);
    if (session.error() != ZJU_ERROR::AUTH_EXPIRED)
    {
        qCritical() << "keepsFirstErrorAndCancelsPendingReconnect did not keep first error";
        return false;
    }

    session.processFinished();
    if (session.requestStop()
        || session.state() != ConnectionState::Disconnected
        || session.wantsConnection())
    {
        qCritical() << "keepsFirstErrorAndCancelsPendingReconnect failed to cancel reconnect";
        return false;
    }
    return true;
}

bool establishedConnectionEndsAsInterrupted()
{
    ConnectionSessionState session;
    session.requestStart({false, 1000});
    session.connectionEstablished();
    session.recordError(ZJU_ERROR::OTHER);

    if (session.processFinished() != ProcessFinishAction::Complete
        || session.state() != ConnectionState::Interrupted)
    {
        qCritical() << "establishedConnectionEndsAsInterrupted failed";
        return false;
    }
    return true;
}
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    return normalLifecycle()
        && reconnectsOnlyEligibleFailures()
        && keepsFirstErrorAndCancelsPendingReconnect()
        && establishedConnectionEndsAsInterrupted()
        ? 0
        : 1;
}
