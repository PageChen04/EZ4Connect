#ifndef APPLICATIONPATHS_H
#define APPLICATIONPATHS_H

#include <QString>

namespace ApplicationPaths
{
QString clientDataFile(const QString &profileId);
void clearClientData(const QString &profileId);
QString logFile();
}

#endif // APPLICATIONPATHS_H
