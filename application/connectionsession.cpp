#include "connectionsession.h"

ConnectionSession::ConnectionSession(CoreProcess *coreProcess, QObject *parent)
    : QObject(parent),
      coreProcess(coreProcess)
{
    coreProcess->setParent(this);
    reconnectTimer.setSingleShot(true);

    connect(coreProcess, &CoreProcess::outputRead, this, &ConnectionSession::outputRead);
    connect(coreProcess, &CoreProcess::graphCaptcha, this, &ConnectionSession::graphCaptcha);
    connect(coreProcess, &CoreProcess::smsCode, this, &ConnectionSession::smsCode);
    connect(coreProcess, &CoreProcess::totpCode, this, &ConnectionSession::totpCode);
    connect(coreProcess, &CoreProcess::ssoAuth, this, &ConnectionSession::ssoAuth);
    connect(coreProcess, &CoreProcess::askSudoPass,
            this, &ConnectionSession::handleSudoPasswordRequest);

    connect(coreProcess, &CoreProcess::error, this, [this](ZJU_ERROR error)
    {
        sessionState.recordError(error);
    });
    connect(coreProcess, &CoreProcess::started, this, [this]()
    {
        sessionState.processStarted();
        emit stateChanged(sessionState.state());
    });
    connect(coreProcess, &CoreProcess::finished,
            this, &ConnectionSession::handleCoreFinished);
    connect(&reconnectTimer, &QTimer::timeout, this, [this]()
    {
        if (sessionState.state() != ConnectionState::Reconnecting)
        {
            return;
        }
        sessionState.beginReconnect();
        emit stateChanged(sessionState.state());
        startCore();
    });
}

bool ConnectionSession::start(const ConnectionProfile &profile, const ReconnectPolicy &policy)
{
    if (!sessionState.requestStart(policy))
    {
        return false;
    }

    currentProfile = profile;
    emit stateChanged(sessionState.state());
    startCore();
    return true;
}

void ConnectionSession::stop()
{
    const ConnectionState previousState = sessionState.state();
    const bool processNeedsStop = sessionState.requestStop();
    if (previousState == ConnectionState::Reconnecting)
    {
        reconnectTimer.stop();
        emit stateChanged(sessionState.state());
        emit finished(sessionState.error());
        return;
    }

    if (processNeedsStop)
    {
        emit stateChanged(sessionState.state());
        coreProcess->stop();
    }
}

void ConnectionSession::submitInput(const QByteArray &data)
{
    coreProcess->writeInput(data);
}

void ConnectionSession::cancelInteractiveInput()
{
    submitInput("\r\n");
    stop();
}

void ConnectionSession::submitSudoPassword(const QString &password, bool remember)
{
    if (password.isEmpty())
    {
        stop();
        return;
    }

    if (remember)
    {
        savedSudoPassword = password;
    }
    sudoPasswordSubmitted = true;
    submitInput(password.toUtf8() + "\n");
}

ConnectionState ConnectionSession::state() const
{
    return sessionState.state();
}

bool ConnectionSession::isActive() const
{
    return sessionState.isActive();
}

void ConnectionSession::startCore()
{
    sudoPasswordSubmitted = false;
    coreProcess->start(currentProfile);
}

void ConnectionSession::handleCoreFinished()
{
    if (sessionState.processFinished() == ProcessFinishAction::Reconnect)
    {
        emit stateChanged(sessionState.state());
        emit reconnectScheduled(sessionState.reconnectDelayMs());
        reconnectTimer.start(sessionState.reconnectDelayMs());
        return;
    }

    emit stateChanged(sessionState.state());
    emit finished(sessionState.error());
}

void ConnectionSession::handleSudoPasswordRequest()
{
    if (savedSudoPassword.isEmpty())
    {
        emit askSudoPass();
        return;
    }

    if (sudoPasswordSubmitted)
    {
        savedSudoPassword.clear();
        emit savedSudoPasswordRejected();
        emit askSudoPass();
        return;
    }

    sudoPasswordSubmitted = true;
    submitInput(savedSudoPassword.toUtf8() + "\n");
}
