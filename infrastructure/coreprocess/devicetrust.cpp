#include "devicetrust.h"

#include <QProcess>

#include <stdexcept>

#include "infrastructure/coreprocess/coreexecutable.h"
#include "infrastructure/storage/applicationpaths.h"

void DeviceTrust::set(
    QObject *parent,
    const QString &protocol,
    const QString &server,
    int port,
    const QString &profileId,
    bool trusted
)
{
    if (protocol != "atrust")
    {
        throw std::runtime_error("授信设备功能仅支持 aTrust");
    }

    QStringList arguments;
    if (!protocol.isEmpty())
    {
        arguments << "-protocol" << protocol;
    }
    if (!server.isEmpty())
    {
        arguments << "-server" << server;
    }
    if (port != 0)
    {
        arguments << "-port" << QString::number(port);
    }
    arguments << "-client-data-file" << ApplicationPaths::clientDataFile(profileId);
    arguments << (trusted ? "-trust-device" : "-untrust-device");

    QProcess process(parent);
    process.start(CoreExecutable::path(), arguments);
    if (!process.waitForStarted())
    {
        throw std::runtime_error("核心无法启动");
    }
    if (!process.waitForFinished())
    {
        throw std::runtime_error("核心运行超时");
    }

    const QByteArray errorOutput = process.readAllStandardError();
    const QByteArray expected =
        trusted ? QByteArray("Device trusted successfully")
                : QByteArray("Device untrusted successfully");
    if (!errorOutput.contains(expected))
    {
        throw std::runtime_error(errorOutput);
    }
}
