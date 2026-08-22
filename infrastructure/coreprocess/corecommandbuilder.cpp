#include "corecommandbuilder.h"

namespace
{
QString quoteArgumentForLog(const QString &argument)
{
    bool requiresQuotes = argument.isEmpty();
    for (const QChar character : argument)
    {
        if (character.isSpace() || character == '"')
        {
            requiresQuotes = true;
            break;
        }
    }

    if (!requiresQuotes)
    {
        return argument;
    }

    QString quoted;
    quoted.reserve(argument.size() + 2);
    quoted += '"';
    for (const QChar character : argument)
    {
        switch (character.unicode())
        {
        case '\\':
            quoted += "\\\\";
            break;
        case '"':
            quoted += "\\\"";
            break;
        case '\n':
            quoted += "\\n";
            break;
        case '\r':
            quoted += "\\r";
            break;
        case '\t':
            quoted += "\\t";
            break;
        default:
            quoted += character;
            break;
        }
    }
    quoted += '"';
    return quoted;
}

void appendOption(QStringList &arguments, const QString &name, const QString &value)
{
    if (!value.isEmpty())
    {
        arguments << name << value;
    }
}
}

QString CoreCommand::loggableCommandLine() const
{
    QStringList quotedArguments;
    quotedArguments.reserve(loggableArguments.size());
    for (const QString &argument : loggableArguments)
    {
        quotedArguments.append(quoteArgumentForLog(argument));
    }
    return quotedArguments.join(' ');
}

CoreCommand CoreCommandBuilder::build(const ConnectionProfile &profile, const CoreRuntimePaths &runtimePaths)
{
    QStringList arguments;
    QStringList credentials;

    appendOption(arguments, "-protocol", profile.endpoint.protocol);

    appendOption(arguments, "-graph-code-file", runtimePaths.graphCodeFile);
    if (profile.endpoint.protocol == "atrust")
    {
        if (!profile.endpoint.authType.isEmpty())
        {
            arguments << "-auth-type" << "auth/" + profile.endpoint.authType;
        }
        appendOption(arguments, "-client-data-file", runtimePaths.clientDataFile);
        appendOption(arguments, "-login-domain", profile.endpoint.loginDomain);
        appendOption(arguments, "-phone", profile.endpoint.phone);
        if (profile.behavior.updateBestNodesInterval != 300)
        {
            arguments << "-update-best-nodes-interval" << QString::number(profile.behavior.updateBestNodesInterval);
        }
    }
    else if (profile.endpoint.protocol == "easyconnect")
    {
        if (profile.behavior.disableMultiLine)
        {
            arguments << "-disable-multi-line";
        }
        if (profile.behavior.disableZjuConfig)
        {
            arguments << "-disable-zju-config";
        }
        if (profile.behavior.skipDomainResource)
        {
            arguments << "-skip-domain-resource";
        }
        appendOption(arguments, "-custom-proxy-domain", profile.proxy.customDomains);
        appendOption(credentials, "-cert-file", profile.credentials.certFile);
        appendOption(credentials, "-cert-password", profile.credentials.certPassword);
    }

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
    if (profile.behavior.debugDump)
    {
        arguments << "-debug-dump";
    }

    appendOption(arguments, "-socks-bind", profile.proxy.socksBind);
    appendOption(arguments, "-http-bind", profile.proxy.httpBind);
    appendOption(arguments, "-shadowsocks-url", profile.proxy.shadowsocksUrl);
    appendOption(arguments, "-dial-direct-proxy", profile.proxy.dialDirectProxy);
    appendOption(arguments, "-tcp-port-forwarding", profile.tunnel.tcpPortForwarding);
    appendOption(arguments, "-udp-port-forwarding", profile.tunnel.udpPortForwarding);
    appendOption(arguments, "-custom-dns", profile.dns.custom);
    if (!profile.extraArguments.isEmpty())
    {
        arguments.append(profile.extraArguments.split(" "));
    }

    CoreCommand command;
    command.loggableArguments = arguments;

    appendOption(credentials, "-username", profile.credentials.username);
    appendOption(credentials, "-password", profile.credentials.password);
    appendOption(credentials, "-totp-secret", profile.credentials.totpSecret);

    command.arguments = credentials + arguments;
    return command;
}
