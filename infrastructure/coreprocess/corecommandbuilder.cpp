#include "corecommandbuilder.h"

namespace
{
void appendOption(QStringList &arguments, const QString &name, const QString &value)
{
    if (!value.isEmpty())
    {
        arguments << name << value;
    }
}
}

CoreCommand CoreCommandBuilder::build(const ConnectionProfile &profile, const CoreRuntimePaths &runtimePaths)
{
    QStringList arguments;

    appendOption(arguments, "-protocol", profile.endpoint.protocol);
    if (!profile.endpoint.authType.isEmpty())
    {
        arguments << "-auth-type" << "auth/" + profile.endpoint.authType;
    }

    appendOption(arguments, "-graph-code-file", runtimePaths.graphCodeFile);
    if (profile.endpoint.protocol == "atrust")
    {
        appendOption(arguments, "-client-data-file", runtimePaths.clientDataFile);
    }

    appendOption(arguments, "-phone", profile.endpoint.phone);
    appendOption(arguments, "-login-domain", profile.endpoint.loginDomain);
    appendOption(arguments, "-server", profile.endpoint.server);
    if (profile.endpoint.port != 0)
    {
        arguments << "-port" << QString::number(profile.endpoint.port);
    }

    if (!profile.dns.primary.isEmpty() || profile.dns.automatic)
    {
        arguments << "-zju-dns-server" << (profile.dns.automatic ? "auto" : profile.dns.primary);
    }
    if (profile.dns.ttl != 3600)
    {
        arguments << "-dns-ttl" << QString::number(profile.dns.ttl);
    }
    appendOption(arguments, "-secondary-dns-server", profile.dns.secondary);

    if (profile.behavior.disableMultiLine)
    {
        arguments << "-disable-multi-line";
    }
    if (profile.behavior.disableKeepAlive)
    {
        arguments << "-disable-keep-alive";
    }
    appendOption(arguments, "-keep-alive-url", profile.behavior.keepAliveUrl);
    appendOption(arguments, "-bind-interface", profile.behavior.bindInterface);
    if (profile.behavior.autoDetectInterface)
    {
        arguments << "-auto-detect-interface";
    }
    if (profile.behavior.disableZjuConfig)
    {
        arguments << "-disable-zju-config";
    }
    if (profile.dns.disableZjuDns)
    {
        arguments << "-disable-zju-dns";
    }
    if (profile.behavior.disableServerConfig)
    {
        arguments << "-disable-server-config";
    }
    if (profile.proxy.proxyAll)
    {
        arguments << "-proxy-all";
    }
    if (profile.behavior.skipDomainResource)
    {
        arguments << "-skip-domain-resource";
    }

    if (profile.tunnel.tunMode)
    {
        arguments << "-tun-mode";
        if (profile.tunnel.dnsHijack)
        {
            arguments << "-dns-hijack";
            if (profile.tunnel.fakeIp)
            {
                arguments << "-fake-ip";
            }
        }
        if (profile.tunnel.addRoute)
        {
            arguments << "-add-route";
        }
    }
    if (profile.tunnel.tcpTunnelMode)
    {
        arguments << "-tcp-tunnel-mode";
    }
    if (profile.behavior.debugDump)
    {
        arguments << "-debug-dump";
    }

    appendOption(arguments, "-socks-bind", profile.proxy.socksBind);
    appendOption(arguments, "-http-bind", profile.proxy.httpBind);
    appendOption(arguments, "-shadowsocks-url", profile.proxy.shadowsocksUrl);
    appendOption(arguments, "-dial-direct-proxy", profile.proxy.dialDirectProxy);
    if (profile.behavior.updateBestNodesInterval != 300)
    {
        arguments << "-update-best-nodes-interval"
                  << QString::number(profile.behavior.updateBestNodesInterval);
    }
    appendOption(arguments, "-tcp-port-forwarding", profile.tunnel.tcpPortForwarding);
    appendOption(arguments, "-udp-port-forwarding", profile.tunnel.udpPortForwarding);
    appendOption(arguments, "-custom-dns", profile.dns.custom);
    appendOption(arguments, "-custom-proxy-domain", profile.proxy.customDomains);
    if (!profile.extraArguments.isEmpty())
    {
        arguments.append(profile.extraArguments.split(" "));
    }

    CoreCommand command;
    command.loggableArguments = arguments;

    QStringList credentials;
    appendOption(credentials, "-username", profile.credentials.username);
    appendOption(credentials, "-password", profile.credentials.password);
    appendOption(credentials, "-totp-secret", profile.credentials.totpSecret);

    appendOption(arguments, "-cert-file", profile.credentials.certFile);
    appendOption(arguments, "-cert-password", profile.credentials.certPassword);
    command.arguments = credentials + arguments;
    return command;
}
