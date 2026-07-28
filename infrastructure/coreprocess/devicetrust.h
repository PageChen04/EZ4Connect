#ifndef DEVICETRUST_H
#define DEVICETRUST_H

#include <QString>

class QObject;

namespace DeviceTrust
{
void set(
    QObject *parent,
    const QString &protocol,
    const QString &server,
    int port,
    const QString &profileId,
    bool trusted
);
}

#endif // DEVICETRUST_H
