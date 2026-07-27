#ifndef PLATFORMSYSTEMPROXYBACKEND_H
#define PLATFORMSYSTEMPROXYBACKEND_H

#include "application/systemproxybackend.h"

class PlatformSystemProxyBackend : public SystemProxyBackend
{
public:
    bool hasConflict(const SystemProxyConfig &config) override;
    void apply(const SystemProxyConfig &config) override;
    void clear() override;
};

#endif // PLATFORMSYSTEMPROXYBACKEND_H
