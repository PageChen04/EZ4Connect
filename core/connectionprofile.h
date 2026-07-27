#ifndef CONNECTIONPROFILE_H
#define CONNECTIONPROFILE_H

#include <QString>

struct ConnectionCredentials
{
    QString username;
    QString password;
    QString totpSecret;
    QString certFile;
    QString certPassword;
};

struct ConnectionEndpoint
{
    QString protocol;
    QString authType;
    QString loginDomain;
    QString phone;
    QString server;
    int port = 0;
};

struct DnsOptions
{
    QString primary;
    bool automatic = false;
    QString secondary;
    int ttl = 3600;
    bool disableZjuDns = false;
    QString custom;
};

struct ProxyOptions
{
    QString socksBind;
    QString httpBind;
    QString shadowsocksUrl;
    QString dialDirectProxy;
    bool proxyAll = false;
    QString customDomains;
};

struct TunnelOptions
{
    bool tunMode = false;
    bool addRoute = false;
    bool dnsHijack = false;
    bool fakeIp = false;
    bool tcpTunnelMode = false;
    QString tcpPortForwarding;
    QString udpPortForwarding;
};

struct ConnectionBehavior
{
    int updateBestNodesInterval = 300;
    bool disableMultiLine = false;
    bool disableKeepAlive = false;
    QString keepAliveUrl;
    QString bindInterface;
    bool autoDetectInterface = false;
    bool skipDomainResource = false;
    bool disableServerConfig = false;
    bool disableZjuConfig = false;
    bool debugDump = false;
};

struct ConnectionProfile
{
    QString program;
    QString profileId;
    ConnectionCredentials credentials;
    ConnectionEndpoint endpoint;
    DnsOptions dns;
    ProxyOptions proxy;
    TunnelOptions tunnel;
    ConnectionBehavior behavior;
    QString extraArguments;
};

#endif // CONNECTIONPROFILE_H
