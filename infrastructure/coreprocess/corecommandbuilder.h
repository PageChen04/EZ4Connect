#ifndef CORECOMMANDBUILDER_H
#define CORECOMMANDBUILDER_H

#include <QStringList>

#include "core/connectionprofile.h"

struct CoreRuntimePaths
{
    QString graphCodeFile;
    QString clientDataFile;
};

struct CoreCommand
{
    QStringList arguments;
    QStringList loggableArguments;

    QString loggableCommandLine() const;
};

class CoreCommandBuilder
{
public:
    static CoreCommand build(const ConnectionProfile &profile, const CoreRuntimePaths &runtimePaths = {});
};

#endif // CORECOMMANDBUILDER_H
