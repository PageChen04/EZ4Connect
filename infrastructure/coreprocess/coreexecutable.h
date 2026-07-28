#ifndef COREEXECUTABLE_H
#define COREEXECUTABLE_H

#include <QString>

class QObject;

namespace CoreExecutable
{
QString path();
QString version(QObject *parent);
}

#endif // COREEXECUTABLE_H
