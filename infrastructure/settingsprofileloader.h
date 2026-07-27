#ifndef SETTINGSPROFILELOADER_H
#define SETTINGSPROFILELOADER_H

#include <QSettings>

#include "core/connectionprofile.h"

class SettingsProfileLoader
{
public:
    static ConnectionProfile load(
        const QSettings &settings,
        const QString &profileId,
        const QString &username,
        const QString &password
    );
};

#endif // SETTINGSPROFILELOADER_H
