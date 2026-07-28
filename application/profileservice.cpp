#include "profileservice.h"

#include <QFileInfo>
#include <QSettings>

#include <utility>

#include "application/profilebackend.h"

ProfileService::ProfileService(
    std::unique_ptr<ProfileBackend> backend,
    const QString &overrideConfigPath,
    QObject *parent
)
    : QObject(parent),
      profileBackend(std::move(backend)),
      currentSettings(nullptr),
      overrideConfiguration(!overrideConfigPath.isEmpty())
{
    if (!overrideConfiguration)
    {
        currentId = profileBackend->activeProfile();
        currentSettings = new QSettings(
            profileBackend->profilePath(currentId),
            QSettings::IniFormat
        );
    }
    else
    {
        currentId = "custom";
        currentSettings = new QSettings(overrideConfigPath, QSettings::IniFormat);
    }
}

ProfileService::~ProfileService()
{
    currentSettings->sync();
    delete currentSettings;
}

QSettings *ProfileService::settings() const
{
    return currentSettings;
}

QString ProfileService::currentProfileId() const
{
    return currentId;
}

QStringList ProfileService::profiles() const
{
    return profileBackend->listProfiles();
}

bool ProfileService::usesOverrideConfiguration() const
{
    return overrideConfiguration;
}

bool ProfileService::switchTo(const QString &profileId)
{
    if (!overrideConfiguration && profileId == currentId)
    {
        return true;
    }

    if (!QFileInfo::exists(profileBackend->profilePath(profileId)))
    {
        return false;
    }

    currentSettings->sync();
    if (!profileBackend->setActiveProfile(profileId))
    {
        return false;
    }

    delete currentSettings;
    currentSettings = new QSettings(
        profileBackend->profilePath(profileId),
        QSettings::IniFormat
    );
    currentId = profileId;
    overrideConfiguration = false;
    return true;
}

QString ProfileService::createAndSwitch(const QString &requestedName)
{
    currentSettings->sync();
    const QString profileId = profileBackend->createProfile(
        requestedName,
        currentSettings->fileName()
    );
    if (profileId.isEmpty() || !switchTo(profileId))
    {
        if (!profileId.isEmpty())
        {
            profileBackend->removeProfile(profileId);
        }
        return {};
    }
    return profileId;
}

bool ProfileService::renameCurrent(const QString &newProfileId)
{
    if (currentId.isEmpty() || overrideConfiguration)
    {
        return false;
    }

    currentSettings->sync();
    if (!profileBackend->renameProfile(currentId, newProfileId))
    {
        return false;
    }

    const QString previousId = currentId;
    if (!profileBackend->setActiveProfile(newProfileId))
    {
        profileBackend->renameProfile(newProfileId, previousId);
        return false;
    }

    currentId = newProfileId;
    delete currentSettings;
    currentSettings = new QSettings(
        profileBackend->profilePath(currentId),
        QSettings::IniFormat
    );
    return true;
}

bool ProfileService::removeCurrentAndSwitchToDefault()
{
    if (currentId.isEmpty() || overrideConfiguration)
    {
        return false;
    }

    const QString removedProfileId = currentId;
    const QString defaultProfilePath = profileBackend->profilePath("");
    if (!QFileInfo::exists(defaultProfilePath))
    {
        return false;
    }

    currentSettings->sync();
    if (!profileBackend->setActiveProfile(""))
    {
        return false;
    }

    if (!profileBackend->removeProfile(removedProfileId))
    {
        profileBackend->setActiveProfile(removedProfileId);
        return false;
    }

    delete currentSettings;
    currentSettings = new QSettings(defaultProfilePath, QSettings::IniFormat);
    currentId.clear();
    return true;
}

QString ProfileService::normalizeProfileId(const QString &name) const
{
    return profileBackend->normalizeProfileId(name);
}

bool ProfileService::silentStartEnabled() const
{
    return profileBackend->silentStartEnabled();
}

void ProfileService::migrateAutoStartSetting(bool enabled)
{
    profileBackend->setAutoStartEnabled(enabled);
}
