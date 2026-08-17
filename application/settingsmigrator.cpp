#include "settingsmigrator.h"

#include <QSettings>

#include "application/applicationconstants.h"
#include "application/defaultsettings.h"

SettingsMigrationAction SettingsMigrator::prepare(QSettings &settings)
{
    const int configVersion = settings.value("Common/ConfigVersion", -1).toInt();
    if (configVersion == -1)
    {
        DefaultSettings::reset(settings);
        return SettingsMigrationAction::None;
    }
    if (configVersion == 4)
    {
        settings.setValue("ZJUConnect/Protocol", "easyconnect");
        return SettingsMigrationAction::None;
    }
    if (configVersion == 6)
    {
        return SettingsMigrationAction::MigrateAutoStart;
    }
    if (configVersion == 8)
    {
        settings.remove("ZJUConnect/TCPTunnelMode");
        return SettingsMigrationAction::None;
    }
    if (configVersion < ApplicationConstants::ConfigVersion)
    {
        return SettingsMigrationAction::RecommendReset;
    }
    return SettingsMigrationAction::None;
}

void SettingsMigrator::finish(QSettings &settings, bool resetToDefaults)
{
    if (resetToDefaults)
    {
        settings.clear();
        DefaultSettings::reset(settings);
    }
    settings.setValue(
        "Common/ConfigVersion",
        ApplicationConstants::ConfigVersion
    );
    settings.sync();
}
