#ifndef SYSTEMPROXYBACKEND_H
#define SYSTEMPROXYBACKEND_H

#include <QString>

struct SystemProxyConfig
{
    int httpPort = 0;
    int socksPort = 0;
    QString bypass;
};

class SystemProxyBackend
{
public:
    virtual ~SystemProxyBackend() = default;

    virtual bool hasConflict(const SystemProxyConfig &config) = 0;
    virtual void apply(const SystemProxyConfig &config) = 0;
    virtual void clear() = 0;
};

#endif // SYSTEMPROXYBACKEND_H
