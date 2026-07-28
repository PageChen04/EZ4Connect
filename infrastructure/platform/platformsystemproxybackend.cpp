#include "platformsystemproxybackend.h"

#include "infrastructure/platform/systemproxyfunctions.h"

bool PlatformSystemProxyBackend::hasConflict(const SystemProxyConfig &config)
{
    return PlatformSystemProxy::isSet(config.httpPort, config.socksPort);
}

void PlatformSystemProxyBackend::apply(const SystemProxyConfig &config)
{
    PlatformSystemProxy::set(config.httpPort, config.socksPort, config.bypass);
}

void PlatformSystemProxyBackend::clear()
{
    PlatformSystemProxy::clear();
}
