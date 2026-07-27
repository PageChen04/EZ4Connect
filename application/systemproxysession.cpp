#include "systemproxysession.h"

#include "infrastructure/platformsystemproxybackend.h"

SystemProxySession::SystemProxySession(QObject *parent)
    : SystemProxySession(std::make_unique<PlatformSystemProxyBackend>(), parent)
{
}

SystemProxySession::SystemProxySession(
    std::unique_ptr<SystemProxyBackend> backend,
    QObject *parent
)
    : QObject(parent),
      backend(std::move(backend))
{
}

bool SystemProxySession::hasConflict(const SystemProxyConfig &config)
{
    return backend->hasConflict(config);
}

void SystemProxySession::enable(const SystemProxyConfig &config)
{
    backend->apply(config);
    if (!enabled)
    {
        enabled = true;
        emit enabledChanged(true);
    }
}

void SystemProxySession::disable()
{
    backend->clear();
    if (enabled)
    {
        enabled = false;
        emit enabledChanged(false);
    }
}

bool SystemProxySession::isEnabled() const
{
    return enabled;
}
