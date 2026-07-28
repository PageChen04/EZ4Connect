#ifndef SETTINGSMIGRATOR_H
#define SETTINGSMIGRATOR_H

class QSettings;

enum class SettingsMigrationAction
{
    None,
    MigrateAutoStart,
    RecommendReset
};

class SettingsMigrator
{
public:
    static SettingsMigrationAction prepare(QSettings &settings);
    static void finish(QSettings &settings, bool resetToDefaults);
};

#endif // SETTINGSMIGRATOR_H
