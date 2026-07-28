#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSettings>
#include <QTemporaryDir>

#include <memory>

#include "application/profilebackend.h"
#include "application/profileservice.h"

namespace
{
class FakeProfileBackend : public ProfileBackend
{
public:
    explicit FakeProfileBackend(const QString &rootPath)
        : rootPath(rootPath)
    {
        QSettings defaultSettings(profilePath(""), QSettings::IniFormat);
        defaultSettings.sync();
    }

    QStringList listProfiles() const override
    {
        QStringList result;
        for (const QFileInfo &file : QDir(rootPath).entryInfoList(
                 {"*.ini"},
                 QDir::Files,
                 QDir::Name
             ))
        {
            if (file.baseName() != "default")
            {
                result << file.baseName();
            }
        }
        return result;
    }

    QString activeProfile() const override
    {
        return activeId;
    }

    bool setActiveProfile(const QString &profileId) const override
    {
        if (!allowActiveProfileChange)
        {
            return false;
        }
        activeId = profileId;
        return true;
    }

    QString profilePath(const QString &profileId) const override
    {
        return QDir(rootPath).filePath(
            (profileId.isEmpty() ? QStringLiteral("default") : profileId) + ".ini"
        );
    }

    QString createProfile(
        const QString &requestedName,
        const QString &sourcePath
    ) const override
    {
        const QString id = normalizeProfileId(requestedName);
        return !id.isEmpty() && QFile::copy(sourcePath, profilePath(id)) ? id : QString();
    }

    bool renameProfile(
        const QString &oldId,
        const QString &newId
    ) const override
    {
        return QFile::rename(profilePath(oldId), profilePath(newId));
    }

    bool removeProfile(const QString &profileId) const override
    {
        return allowProfileRemoval && QFile::remove(profilePath(profileId));
    }

    QString normalizeProfileId(const QString &name) const override
    {
        return name.trimmed();
    }

    bool silentStartEnabled() const override
    {
        return false;
    }

    void setAutoStartEnabled(bool enabled) const override
    {
        migratedAutoStart = enabled;
    }

    mutable QString activeId;
    mutable bool migratedAutoStart = false;
    bool allowActiveProfileChange = true;
    bool allowProfileRemoval = true;

private:
    QString rootPath;
};

bool managesCurrentProfileAsOneUnit()
{
    QTemporaryDir directory;
    auto backend = std::make_unique<FakeProfileBackend>(directory.path());
    FakeProfileBackend *fake = backend.get();
    ProfileService service(std::move(backend));
    service.settings()->setValue("Credential/Username", "alice");
    service.settings()->sync();

    fake->allowActiveProfileChange = false;
    if (!service.createAndSwitch("blocked").isEmpty()
        || service.currentProfileId() != ""
        || QFileInfo::exists(fake->profilePath("blocked")))
    {
        qCritical() << "failed activation left a created profile behind";
        return false;
    }
    fake->allowActiveProfileChange = true;

    if (!service.switchTo("missing"))
    {
        // Expected: missing profiles are rejected.
    }
    else
    {
        qCritical() << "missing profile was accepted";
        return false;
    }

    if (service.createAndSwitch("work") != "work"
        || service.currentProfileId() != "work"
        || service.settings()->value("Credential/Username").toString() != "alice")
    {
        qCritical() << "profile creation failed";
        return false;
    }

    fake->allowActiveProfileChange = false;
    QSettings *workSettings = service.settings();
    if (service.renameCurrent("blocked")
        || service.currentProfileId() != "work"
        || service.settings() != workSettings
        || !QFileInfo::exists(fake->profilePath("work"))
        || QFileInfo::exists(fake->profilePath("blocked")))
    {
        qCritical() << "failed activation did not roll back profile rename";
        return false;
    }
    fake->allowActiveProfileChange = true;

    if (!service.renameCurrent("renamed")
        || service.currentProfileId() != "renamed")
    {
        qCritical() << "profile rename failed";
        return false;
    }

    QSettings *renamedSettings = service.settings();
    fake->allowActiveProfileChange = false;
    if (service.removeCurrentAndSwitchToDefault()
        || service.currentProfileId() != "renamed"
        || service.settings() != renamedSettings
        || !QFileInfo::exists(fake->profilePath("renamed"))
        || fake->activeId != "renamed")
    {
        qCritical() << "failed default activation changed the current profile";
        return false;
    }
    fake->allowActiveProfileChange = true;

    fake->allowProfileRemoval = false;
    if (service.removeCurrentAndSwitchToDefault()
        || service.currentProfileId() != "renamed"
        || service.settings() != renamedSettings
        || !QFileInfo::exists(fake->profilePath("renamed"))
        || fake->activeId != "renamed")
    {
        qCritical() << "failed removal changed the current profile";
        return false;
    }

    fake->allowProfileRemoval = true;
    service.migrateAutoStartSetting(true);
    if (!fake->migratedAutoStart
        || !service.removeCurrentAndSwitchToDefault()
        || !service.currentProfileId().isEmpty())
    {
        qCritical() << "profile removal or global setting migration failed";
        return false;
    }
    return true;
}

bool acceptsCustomAsARegularProfileId()
{
    QTemporaryDir directory;
    auto backend = std::make_unique<FakeProfileBackend>(directory.path());
    FakeProfileBackend *fake = backend.get();
    ProfileService service(std::move(backend));
    service.settings()->setValue("Credential/Username", "custom-user");
    service.settings()->sync();

    if (service.createAndSwitch("custom") != "custom"
        || service.usesOverrideConfiguration()
        || service.currentProfileId() != "custom")
    {
        qCritical() << "custom profile creation failed";
        return false;
    }
    if (!service.renameCurrent("renamed")
        || service.currentProfileId() != "renamed")
    {
        qCritical() << "custom profile rename failed";
        return false;
    }
    if (!service.removeCurrentAndSwitchToDefault())
    {
        qCritical() << "renamed custom profile removal failed";
        return false;
    }
    return fake->activeId.isEmpty();
}

bool switchesAwayFromAnOverrideNamedCustom()
{
    QTemporaryDir directory;
    const QString overridePath = directory.filePath("override.ini");
    const QString profilesPath = directory.filePath("profiles");
    QDir().mkpath(profilesPath);
    {
        QSettings overrideSettings(overridePath, QSettings::IniFormat);
        overrideSettings.setValue("Credential/Username", "override-user");
        overrideSettings.sync();
    }

    auto backend = std::make_unique<FakeProfileBackend>(profilesPath);
    FakeProfileBackend *fake = backend.get();
    ProfileService service(std::move(backend), overridePath);

    if (!service.usesOverrideConfiguration()
        || service.createAndSwitch("custom") != "custom"
        || service.usesOverrideConfiguration()
        || service.currentProfileId() != "custom"
        || service.settings()->fileName() != fake->profilePath("custom")
        || fake->activeId != "custom")
    {
        qCritical() << "override configuration was confused with profile custom";
        return false;
    }
    return true;
}
}

int main(int argc, char *argv[])
{
    QCoreApplication application(argc, argv);
    return managesCurrentProfileAsOneUnit()
        && acceptsCustomAsARegularProfileId()
        && switchesAwayFromAnOverrideNamedCustom() ? 0 : 1;
}
