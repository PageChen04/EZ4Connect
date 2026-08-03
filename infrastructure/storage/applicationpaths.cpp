#include "applicationpaths.h"

#include <QDir>
#include <QFile>
#include <QStandardPaths>

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

QString ApplicationPaths::logFile()
{
    QDir logDirectory(
        QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation)
    );
    if (!logDirectory.exists())
    {
        logDirectory.mkpath(".");
    }
    return logDirectory.filePath("ez4connect.log");
}
