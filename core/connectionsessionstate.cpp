#include "connectionsessionstate.h"

ConnectionState ConnectionSessionState::state() const
{
    return currentState;
}

ZJU_ERROR ConnectionSessionState::error() const
{
    return currentError;
}

bool ConnectionSessionState::isActive() const
{
    return currentState == ConnectionState::Starting
        || currentState == ConnectionState::Running
        || currentState == ConnectionState::Stopping
        || currentState == ConnectionState::Reconnecting;
}

bool ConnectionSessionState::wantsConnection() const
{
    return desiredConnected;
}

int ConnectionSessionState::reconnectDelayMs() const
{
    return reconnectPolicy.delayMs;
}

bool ConnectionSessionState::requestStart(const ReconnectPolicy &policy)
{
    if (isActive())
    {
        return false;
    }

    reconnectPolicy = policy;
    desiredConnected = true;
    currentError = ZJU_ERROR::NONE;
    currentState = ConnectionState::Starting;
    return true;
}

void ConnectionSessionState::connectionEstablished()
{
    if (currentState == ConnectionState::Starting)
    {
        currentState = ConnectionState::Running;
    }
}

void ConnectionSessionState::recordError(ZJU_ERROR error)
{
    if (currentError == ZJU_ERROR::NONE)
    {
        currentError = error;
    }
}

bool ConnectionSessionState::requestStop()
{
    desiredConnected = false;
    if (currentState == ConnectionState::Reconnecting)
    {
        currentState = ConnectionState::Disconnected;
        return false;
    }
    if (currentState == ConnectionState::Starting || currentState == ConnectionState::Running)
    {
        currentState = ConnectionState::Stopping;
        return true;
    }
    return currentState == ConnectionState::Stopping;
}

ProcessFinishAction ConnectionSessionState::processFinished()
{
    const bool connectionWasEstablished = currentState == ConnectionState::Running;
    if (desiredConnected
        && reconnectPolicy.enabled
        && isReconnectable(currentError))
    {
        currentState = ConnectionState::Reconnecting;
        return ProcessFinishAction::Reconnect;
    }

    desiredConnected = false;
    if (connectionWasEstablished)
    {
        currentState = ConnectionState::Interrupted;
    }
    else
    {
        currentState = currentError == ZJU_ERROR::NONE
            ? ConnectionState::Disconnected
            : ConnectionState::Failed;
    }
    return ProcessFinishAction::Complete;
}

void ConnectionSessionState::beginReconnect()
{
    if (currentState == ConnectionState::Reconnecting && desiredConnected)
    {
        currentError = ZJU_ERROR::NONE;
        currentState = ConnectionState::Starting;
    }
}

bool ConnectionSessionState::isReconnectable(ZJU_ERROR error)
{
    return error == ZJU_ERROR::AUTH_EXPIRED || error == ZJU_ERROR::OTHER;
}
