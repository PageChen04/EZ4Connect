#include <QCoreApplication>
#include <QDebug>

#include "application/connectionsession.h"

namespace
{
class FakeCoreProcess : public CoreProcess
{
public:
    int startCalls = 0;
    int stopCalls = 0;
    QByteArray lastInput;

    void start(const ConnectionProfile &) override
    {
        ++startCalls;
        emit started();
    }

    void stop() override
    {
        ++stopCalls;
    }

    void writeInput(const QByteArray &data) override
    {
        lastInput = data;
    }

    void complete()
    {
        emit finished();
    }

    void requestSudoPassword()
    {
        emit askSudoPass();
    }
};

bool delegatesProcessLifecycleThroughPort()
{
    auto *coreProcess = new FakeCoreProcess();
    ConnectionSession session(coreProcess);
    ConnectionProfile profile;
    int finishedCalls = 0;
    QObject::connect(
        &session,
        &ConnectionSession::finished,
        [&](ZJU_ERROR) { ++finishedCalls; }
    );

    if (!session.start(profile, {})
        || coreProcess->startCalls != 1
        || session.state() != ConnectionState::Running
        || !session.isActive()
        || session.start(profile, {}))
    {
        qCritical() << "start delegation failed";
        return false;
    }

    session.submitInput("input");
    session.stop();
    if (coreProcess->lastInput != "input"
        || coreProcess->stopCalls != 1
        || session.state() != ConnectionState::Stopping)
    {
        qCritical() << "input or stop delegation failed";
        return false;
    }

    coreProcess->complete();
    if (finishedCalls != 1
        || session.state() != ConnectionState::Disconnected
        || session.isActive())
    {
        qCritical() << "completion delegation failed";
        return false;
    }
    return true;
}

bool emptySudoPasswordStopsTheSession()
{
    auto *coreProcess = new FakeCoreProcess();
    ConnectionSession session(coreProcess);
    ConnectionProfile profile;

    if (!session.start(profile, {}))
    {
        qCritical() << "failed to start session for sudo cancellation";
        return false;
    }

    coreProcess->requestSudoPassword();
    session.submitSudoPassword({}, false);
    if (coreProcess->stopCalls != 1
        || session.state() != ConnectionState::Stopping)
    {
        qCritical() << "empty sudo password did not stop the session";
        return false;
    }
    return true;
}

bool cancelledInteractiveInputSubmitsNewlineBeforeStopping()
{
    auto *coreProcess = new FakeCoreProcess();
    ConnectionSession session(coreProcess);
    ConnectionProfile profile;

    if (!session.start(profile, {}))
    {
        qCritical() << "failed to start session for interactive input cancellation";
        return false;
    }

    session.cancelInteractiveInput();
    if (coreProcess->lastInput != "\r\n"
        || coreProcess->stopCalls != 1
        || session.state() != ConnectionState::Stopping)
    {
        qCritical() << "interactive input cancellation did not submit a newline and stop";
        return false;
    }
    return true;
}
}

int main(int argc, char *argv[])
{
    QCoreApplication application(argc, argv);
    return delegatesProcessLifecycleThroughPort()
        && emptySudoPasswordStopsTheSession()
        && cancelledInteractiveInputSubmitsNewlineBeforeStopping() ? 0 : 1;
}
