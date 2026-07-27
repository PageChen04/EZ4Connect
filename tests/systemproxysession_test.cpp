#include <functional>
#include <memory>

#include <QCoreApplication>
#include <QDebug>
#include <QElapsedTimer>
#include <QSemaphore>
#include <QThread>

#include "application/systemproxysession.h"

namespace
{
class FakeSystemProxyBackend : public SystemProxyBackend
{
public:
    bool conflict = false;
    int conflictChecks = 0;
    int applyCalls = 0;
    int clearCalls = 0;
    SystemProxyConfig lastConfig;
    QSemaphore operationStarted;
    QSemaphore allowOperationToFinish;

    bool hasConflict(const SystemProxyConfig &config) override
    {
        ++conflictChecks;
        lastConfig = config;
        return conflict;
    }

    void apply(const SystemProxyConfig &config) override
    {
        ++applyCalls;
        lastConfig = config;
        operationStarted.release();
        allowOperationToFinish.acquire();
    }

    void clear() override
    {
        ++clearCalls;
    }
};

bool waitUntil(const std::function<bool()> &condition)
{
    QElapsedTimer timer;
    timer.start();
    while (!condition() && timer.elapsed() < 1000)
    {
        QCoreApplication::processEvents();
        QThread::msleep(1);
    }
    return condition();
}

bool delegatesPlatformOperationsAsynchronouslyAndTracksOwnedState()
{
    auto backend = std::make_unique<FakeSystemProxyBackend>();
    FakeSystemProxyBackend *fake = backend.get();
    fake->conflict = true;
    SystemProxySession session(std::move(backend));
    const SystemProxyConfig config{1081, 1080, "localhost"};

    bool conflictResult = false;
    QObject::connect(&session, &SystemProxySession::conflictCheckFinished,
                     [&](bool conflict) { conflictResult = conflict; });
    if (!session.checkConflict(config)
        || !waitUntil([&]() { return !session.isBusy(); })
        || !conflictResult
        || fake->conflictChecks != 1
        || session.isEnabled())
    {
        qCritical() << "delegatesPlatformOperationsAsynchronouslyAndTracksOwnedState failed at conflict check";
        return false;
    }

    if (!session.enable(config)
        || !fake->operationStarted.tryAcquire(1, 1000)
        || !session.isBusy()
        || session.isEnabled()
        || session.disable())
    {
        qCritical() << "delegatesPlatformOperationsAsynchronouslyAndTracksOwnedState failed while enabling";
        return false;
    }

    fake->allowOperationToFinish.release();
    if (!waitUntil([&]() { return !session.isBusy(); })
        || !session.isEnabled()
        || fake->applyCalls != 1
        || fake->lastConfig.httpPort != 1081
        || fake->lastConfig.socksPort != 1080
        || fake->lastConfig.bypass != "localhost")
    {
        qCritical() << "delegatesPlatformOperationsAsynchronouslyAndTracksOwnedState failed after enabling";
        return false;
    }

    if (!session.disable()
        || !waitUntil([&]() { return !session.isBusy(); })
        || session.isEnabled()
        || fake->clearCalls != 1)
    {
        qCritical() << "delegatesPlatformOperationsAsynchronouslyAndTracksOwnedState failed at disable";
        return false;
    }

    if (!session.disable()
        || !waitUntil([&]() { return !session.isBusy(); })
        || fake->clearCalls != 2)
    {
        qCritical() << "disable must also support clearing externally-owned proxy state";
        return false;
    }

    if (!session.enable(config)
        || !fake->operationStarted.tryAcquire(1, 1000))
    {
        qCritical() << "clearBeforeShutdown failed to start enable";
        return false;
    }
    fake->allowOperationToFinish.release();
    session.clearBeforeShutdown();
    if (session.isBusy()
        || session.isEnabled()
        || fake->applyCalls != 2
        || fake->clearCalls != 3)
    {
        qCritical() << "clearBeforeShutdown must clear an in-flight enable";
        return false;
    }
    return true;
}
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    return delegatesPlatformOperationsAsynchronouslyAndTracksOwnedState() ? 0 : 1;
}
