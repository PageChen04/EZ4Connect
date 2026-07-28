#ifndef PROFILEBACKEND_H
#define PROFILEBACKEND_H

#include <QString>
#include <QStringList>

class ProfileBackend
{
public:
    virtual ~ProfileBackend() = default;

    virtual QStringList listProfiles() const = 0;
    virtual QString activeProfile() const = 0;
    virtual bool setActiveProfile(const QString &profileId) const = 0;
    virtual QString profilePath(const QString &profileId) const = 0;
    virtual QString createProfile(
        const QString &requestedName,
        const QString &sourcePath = {}
    ) const = 0;
    virtual bool renameProfile(
        const QString &oldId,
        const QString &newId
    ) const = 0;
    virtual bool removeProfile(const QString &profileId) const = 0;
    virtual QString normalizeProfileId(const QString &name) const = 0;
    virtual bool silentStartEnabled() const = 0;
    virtual void setAutoStartEnabled(bool enabled) const = 0;
};

#endif // PROFILEBACKEND_H
