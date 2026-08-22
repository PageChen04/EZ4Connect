#include <QCoreApplication>
#include <QDebug>

#include "infrastructure/coreprocess/corecommandbuilder.h"

namespace
{
bool expectEqual(const QStringList &actual, const QStringList &expected, const char *testName)
{
    if (actual == expected)
    {
        return true;
    }

    qCritical().noquote() << testName << "failed"
                          << "\nexpected:" << expected.join('|')
                          << "\nactual:  " << actual.join('|');
    return false;
}

bool buildsMinimalCommand()
{
    ConnectionProfile profile;
    profile.endpoint.protocol = "easyconnect";

    const CoreCommand command = CoreCommandBuilder::build(profile);
    return expectEqual(command.arguments, {"-protocol", "easyconnect"}, "buildsMinimalCommand")
        && expectEqual(command.loggableArguments, command.arguments, "minimalCommandIsLoggable");
}

bool addsGraphCaptchaFileForEasyConnect()
{
    ConnectionProfile profile;
    profile.endpoint = {
        "easyconnect",
        "cas",
        "domain",
        "86-123",
        "vpn.example.edu",
        443
    };
    profile.behavior.updateBestNodesInterval = 30;

    CoreRuntimePaths runtimePaths;
    runtimePaths.graphCodeFile = "/tmp/easyconnect-graph.jpg";
    runtimePaths.clientDataFile = "/tmp/atrust-client-data.json";
    const CoreCommand command = CoreCommandBuilder::build(profile, runtimePaths);
    return expectEqual(
        command.arguments,
        {
            "-protocol", "easyconnect",
            "-graph-code-file", "/tmp/easyconnect-graph.jpg",
            "-server", "vpn.example.edu",
            "-port", "443"
        },
        "addsGraphCaptchaFileForEasyConnect"
    );
}

bool buildsCompleteCommandInCompatibleOrder()
{
    ConnectionProfile profile;
    profile.endpoint = {"atrust", "cas", "domain", "86-123", "vpn.example.edu", 8443};
    profile.credentials = {"alice", "secret", "TOTP", "/tmp/client.p12", "cert-secret"};
    profile.dns = {"10.0.0.1", false, "10.0.0.2", 60, true, "example.org=1.1.1.1"};
    profile.proxy = {"127.0.0.1:1080", "127.0.0.1:1081", "ss://url", "http://direct",
                     true, "example.org"};
    profile.tunnel = {true, true, true, true, "127.0.0.1:80/10.0.0.1:80",
                      "127.0.0.1:53/10.0.0.1:53"};
    profile.behavior = {30, true, true, "https://keepalive", "en0", true, true, true,
                        true, true};
    profile.extraArguments = "-foo bar";

    const CoreRuntimePaths runtimePaths{"/tmp/graph.jpg", "/tmp/client-data.json"};
    const CoreCommand command = CoreCommandBuilder::build(profile, runtimePaths);
    const QStringList expected{
        "-username", "alice",
        "-password", "secret",
        "-totp-secret", "TOTP",
        "-protocol", "atrust",
        "-graph-code-file", "/tmp/graph.jpg",
        "-auth-type", "auth/cas",
        "-client-data-file", "/tmp/client-data.json",
        "-login-domain", "domain",
        "-phone", "86-123",
        "-update-best-nodes-interval", "30",
        "-server", "vpn.example.edu",
        "-port", "8443",
        "-zju-dns-server", "10.0.0.1",
        "-dns-ttl", "60",
        "-secondary-dns-server", "10.0.0.2",
        "-disable-keep-alive",
        "-keep-alive-url", "https://keepalive",
        "-bind-interface", "en0",
        "-auto-detect-interface",
        "-disable-zju-dns",
        "-disable-server-config",
        "-proxy-all",
        "-tun-mode",
        "-dns-hijack",
        "-fake-ip",
        "-add-route",
        "-debug-dump",
        "-socks-bind", "127.0.0.1:1080",
        "-http-bind", "127.0.0.1:1081",
        "-shadowsocks-url", "ss://url",
        "-dial-direct-proxy", "http://direct",
        "-tcp-port-forwarding", "127.0.0.1:80/10.0.0.1:80",
        "-udp-port-forwarding", "127.0.0.1:53/10.0.0.1:53",
        "-custom-dns", "example.org=1.1.1.1",
        "-foo", "bar"
    };
    return expectEqual(command.arguments, expected, "buildsCompleteCommandInCompatibleOrder");
}

bool keepsEasyConnectOnlyOptionsOutOfATrustCommand()
{
    ConnectionProfile profile;
    profile.endpoint.protocol = "atrust";
    profile.credentials.certFile = "/tmp/client.p12";
    profile.credentials.certPassword = "cert-secret";
    profile.proxy.customDomains = "example.org";
    profile.behavior.disableMultiLine = true;
    profile.behavior.disableZjuConfig = true;
    profile.behavior.skipDomainResource = true;

    const CoreCommand command = CoreCommandBuilder::build(profile);
    return expectEqual(
        command.arguments,
        {"-protocol", "atrust"},
        "keepsEasyConnectOnlyOptionsOutOfATrustCommand"
    );
}

bool addsEasyConnectOnlyOptionsForEasyConnect()
{
    ConnectionProfile profile;
    profile.endpoint.protocol = "easyconnect";
    profile.credentials.username = "alice";
    profile.credentials.certFile = "/tmp/client.p12";
    profile.credentials.certPassword = "cert-secret";
    profile.proxy.customDomains = "example.org";
    profile.behavior.disableMultiLine = true;
    profile.behavior.disableZjuConfig = true;
    profile.behavior.skipDomainResource = true;

    const CoreCommand command = CoreCommandBuilder::build(profile);
    return expectEqual(
        command.arguments,
        {
            "-cert-file", "/tmp/client.p12",
            "-cert-password", "cert-secret",
            "-username", "alice",
            "-protocol", "easyconnect",
            "-disable-multi-line",
            "-disable-zju-config",
            "-skip-domain-resource",
            "-custom-proxy-domain", "example.org"
        },
        "addsEasyConnectOnlyOptionsForEasyConnect"
    );
}

bool keepsATrustOnlyOptionsOutOfEasyConnectCommand()
{
    ConnectionProfile profile;
    profile.endpoint = {"easyconnect", "cas", "domain", "86-123", QString(), 0};
    profile.behavior.updateBestNodesInterval = 30;

    const CoreRuntimePaths runtimePaths{QString(), "/tmp/structured-client-data.json"};
    const CoreCommand command = CoreCommandBuilder::build(profile, runtimePaths);
    return expectEqual(
        command.arguments,
        {"-protocol", "easyconnect"},
        "keepsATrustOnlyOptionsOutOfEasyConnectCommand"
    );
}

bool excludesCredentialsFromLoggableArguments()
{
    ConnectionProfile profile;
    profile.endpoint.protocol = "easyconnect";
    profile.credentials = {"alice", "secret", "TOTP", "/tmp/client.p12", "cert-secret"};

    const CoreCommand command = CoreCommandBuilder::build(profile);
    const QString logLine = command.loggableArguments.join(' ');
    const bool safe = !logLine.contains("alice")
        && !logLine.contains("secret")
        && !logLine.contains("TOTP")
        && !logLine.contains("/tmp/client.p12");
    if (!safe)
    {
        qCritical() << "excludesCredentialsFromLoggableArguments failed:" << logLine;
    }
    return safe;
}

bool quotesLoggableArgumentsWithoutChangingArguments()
{
    CoreCommand command;
    command.loggableArguments = {
        "-graph-code-file",
        "/tmp/captcha images/graph.jpg",
        "",
        "embedded\"quote",
        "line\nbreak"
    };

    const QStringList originalArguments = command.loggableArguments;
    const QString expected =
        "-graph-code-file \"/tmp/captcha images/graph.jpg\" \"\" "
        "\"embedded\\\"quote\" \"line\\nbreak\"";
    const QString actual = command.loggableCommandLine();
    if (actual != expected || command.loggableArguments != originalArguments)
    {
        qCritical().noquote() << "quotesLoggableArgumentsWithoutChangingArguments failed"
                              << "\nexpected:" << expected
                              << "\nactual:  " << actual;
        return false;
    }
    return true;
}
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    const bool passed = buildsMinimalCommand()
        && addsGraphCaptchaFileForEasyConnect()
        && buildsCompleteCommandInCompatibleOrder()
        && keepsEasyConnectOnlyOptionsOutOfATrustCommand()
        && addsEasyConnectOnlyOptionsForEasyConnect()
        && keepsATrustOnlyOptionsOutOfEasyConnectCommand()
        && excludesCredentialsFromLoggableArguments()
        && quotesLoggableArgumentsWithoutChangingArguments();
    return passed ? 0 : 1;
}
