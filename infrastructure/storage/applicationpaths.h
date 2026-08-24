#ifndef APPLICATIONPATHS_H
#define APPLICATIONPATHS_H

#include <QString>

namespace ApplicationPaths
{
struct DebugArtifactPaths
{
    QString pcapFile;
    QString tlsLogFile;
};

QString clientDataFile(const QString &profileId);
void clearClientData(const QString &profileId);
QString logDirectory();
QString logFile();
DebugArtifactPaths createDebugArtifactFiles(
    const QString &profileId,
    bool createPcap,
    bool createTlsLog
);
}

#endif // APPLICATIONPATHS_H
