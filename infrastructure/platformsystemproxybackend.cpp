#include "platformsystemproxybackend.h"

#include "utils/utils.h"

bool PlatformSystemProxyBackend::hasConflict(const SystemProxyConfig &config)
{
    return Utils::isSystemProxySet(config.httpPort, config.socksPort);
}

void PlatformSystemProxyBackend::apply(const SystemProxyConfig &config)
{
    Utils::setSystemProxy(config.httpPort, config.socksPort, config.bypass);
}

void PlatformSystemProxyBackend::clear()
{
    Utils::clearSystemProxy();
}
