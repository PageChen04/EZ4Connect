#ifndef PLATFORMSYSTEMPROXYBACKEND_H
#define PLATFORMSYSTEMPROXYBACKEND_H

#include "application/systemproxybackend.h"

class PlatformSystemProxyBackend : public SystemProxyBackend
{
public:
    bool hasConflict(const SystemProxyConfig &config) override;
    bool apply(const SystemProxyConfig &config) override;
    bool clear() override;
};

#endif // PLATFORMSYSTEMPROXYBACKEND_H
