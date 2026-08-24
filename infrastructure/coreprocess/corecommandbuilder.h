#ifndef CORECOMMANDBUILDER_H
#define CORECOMMANDBUILDER_H

#include <QMap>
#include <QStringList>

#include "core/connectionprofile.h"

struct CoreRuntimePaths
{
    QString graphCodeFile;
    QString clientDataFile;
    QString debugPcapFile;
    QString debugTlsLogFile;
};

struct CoreCommand
{
    QStringList arguments;
    QStringList loggableArguments;
    QStringList clearedEnvironmentVariables;
    QMap<QString, QString> environmentVariables;

    QString loggableCommandLine() const;
};

class CoreCommandBuilder
{
public:
    static CoreCommand build(const ConnectionProfile &profile, const CoreRuntimePaths &runtimePaths = {});
};

#endif // CORECOMMANDBUILDER_H
