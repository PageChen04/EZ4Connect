#ifndef PROFILESERVICE_H
#define PROFILESERVICE_H

#include <QObject>
#include <QStringList>

#include <memory>

class ProfileBackend;
class QSettings;

class ProfileService : public QObject
{
    Q_OBJECT

public:
    explicit ProfileService(
        std::unique_ptr<ProfileBackend> backend,
        const QString &overrideConfigPath = {},
        QObject *parent = nullptr
    );
    ~ProfileService() override;

    QSettings *settings() const;
    QString currentProfileId() const;
    QStringList profiles() const;
    bool usesOverrideConfiguration() const;

    bool switchTo(const QString &profileId);
    QString createAndSwitch(const QString &requestedName);
    bool renameCurrent(const QString &newProfileId);
    bool removeCurrentAndSwitchToDefault();

    QString normalizeProfileId(const QString &name) const;
    bool silentStartEnabled() const;
    void migrateAutoStartSetting(bool enabled);

private:
    std::unique_ptr<ProfileBackend> profileBackend;
    QSettings *currentSettings;
    QString currentId;
    bool overrideConfiguration;
};

#endif // PROFILESERVICE_H
