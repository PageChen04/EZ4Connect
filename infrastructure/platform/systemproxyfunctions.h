#ifndef SYSTEMPROXYFUNCTIONS_H
#define SYSTEMPROXYFUNCTIONS_H

#include <QString>

namespace PlatformSystemProxy
{
bool isSet(int httpPort = -1, int socksPort = -1);
void set(int httpPort, int socksPort, const QString &bypass);
void clear();
}

#endif // SYSTEMPROXYFUNCTIONS_H
