#include "applicationpaths.h"

#include <QDir>
#include <QDateTime>
#include <QFile>
#include <QRegularExpression>
#include <QStandardPaths>

namespace
{
QString safeProfileName(const QString &profileId)
{
    QString name = profileId.isEmpty() ? "default" : profileId;
    name.replace(QRegularExpression("[^A-Za-z0-9_.-]"), "_");
    return name;
}

void createPrivateFile(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        return;
    }
    file.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner);
}
}

QString ApplicationPaths::clientDataFile(const QString &profileId)
{
    QDir dataDirectory(
        QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation)
    );
    if (!dataDirectory.exists())
    {
        dataDirectory.mkpath(".");
    }

    QDir profileDirectory(dataDirectory.filePath("profiles/" + profileId));
    if (!profileDirectory.exists())
    {
        profileDirectory.mkpath(".");
    }

    QFile clientData(profileDirectory.filePath("client-data.json"));
    if (!clientData.exists() && clientData.open(QIODevice::WriteOnly))
    {
        clientData.write("{}");
        clientData.close();
    }
    return clientData.fileName();
}

void ApplicationPaths::clearClientData(const QString &profileId)
{
    QFile::remove(clientDataFile(profileId));
}

QString ApplicationPaths::logDirectory()
{
    QDir logDirectory(
        QDir(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation))
            .filePath("logs")
    );
    if (!logDirectory.exists())
    {
        logDirectory.mkpath(".");
    }
    return logDirectory.absolutePath();
}

QString ApplicationPaths::logFile()
{
    return QDir(logDirectory()).filePath("ez4connect.log");
}

ApplicationPaths::DebugArtifactPaths ApplicationPaths::createDebugArtifactFiles(
    const QString &profileId,
    bool createPcap,
    bool createTlsLog
)
{
    const QString baseName = QString("debug-%1-%2")
        .arg(
            safeProfileName(profileId),
            QDateTime::currentDateTime().toString("yyyyMMdd-HHmmss-zzz")
        );
    const QDir directory(logDirectory());

    DebugArtifactPaths paths;
    if (createPcap)
    {
        paths.pcapFile = directory.filePath(baseName + ".pcap");
        createPrivateFile(paths.pcapFile);
    }
    if (createTlsLog)
    {
        paths.tlsLogFile = directory.filePath(baseName + ".keys.log");
        createPrivateFile(paths.tlsLogFile);
    }
    return paths;
}
