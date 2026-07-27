#include <memory>

#include <QCoreApplication>
#include <QDebug>

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
    }

    void clear() override
    {
        ++clearCalls;
    }
};

bool delegatesPlatformOperationsAndTracksOwnedState()
{
    auto backend = std::make_unique<FakeSystemProxyBackend>();
    FakeSystemProxyBackend *fake = backend.get();
    fake->conflict = true;
    SystemProxySession session(std::move(backend));
    const SystemProxyConfig config{1081, 1080, "localhost"};

    if (!session.hasConflict(config)
        || fake->conflictChecks != 1
        || session.isEnabled())
    {
        qCritical() << "delegatesPlatformOperationsAndTracksOwnedState failed at conflict check";
        return false;
    }

    session.enable(config);
    if (!session.isEnabled()
        || fake->applyCalls != 1
        || fake->lastConfig.httpPort != 1081
        || fake->lastConfig.socksPort != 1080
        || fake->lastConfig.bypass != "localhost")
    {
        qCritical() << "delegatesPlatformOperationsAndTracksOwnedState failed at enable";
        return false;
    }

    session.disable();
    if (session.isEnabled() || fake->clearCalls != 1)
    {
        qCritical() << "delegatesPlatformOperationsAndTracksOwnedState failed at disable";
        return false;
    }

    session.disable();
    if (fake->clearCalls != 2)
    {
        qCritical() << "disable must also support clearing externally-owned proxy state";
        return false;
    }
    return true;
}
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    return delegatesPlatformOperationsAndTracksOwnedState() ? 0 : 1;
}
