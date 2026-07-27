#ifndef CONNECTIONSESSIONSTATE_H
#define CONNECTIONSESSIONSTATE_H

#include "connectionerror.h"

enum class ConnectionState
{
    Disconnected,
    Starting,
    Running,
    Stopping,
    Reconnecting,
    Failed,
};

struct ReconnectPolicy
{
    bool enabled = false;
    int delayMs = 1000;
};

enum class ProcessFinishAction
{
    Complete,
    Reconnect,
};

class ConnectionSessionState
{
public:
    ConnectionState state() const;
    ZJU_ERROR error() const;
    bool isActive() const;
    bool wantsConnection() const;
    int reconnectDelayMs() const;

    bool requestStart(const ReconnectPolicy &policy);
    void processStarted();
    void recordError(ZJU_ERROR error);
    bool requestStop();
    ProcessFinishAction processFinished();
    void beginReconnect();

private:
    static bool isReconnectable(ZJU_ERROR error);

    ConnectionState currentState = ConnectionState::Disconnected;
    ZJU_ERROR currentError = ZJU_ERROR::NONE;
    ReconnectPolicy reconnectPolicy;
    bool desiredConnected = false;
};

#endif // CONNECTIONSESSIONSTATE_H
