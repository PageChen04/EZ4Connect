#include <QCoreApplication>
#include <QDebug>
#include <QSettings>
#include <QTemporaryDir>

#include "application/settingsmigrator.h"

namespace
{
bool migratesKnownVersions()
{
    QTemporaryDir directory;
    QSettings settings(directory.filePath("profile.ini"), QSettings::IniFormat);
    settings.setValue("Common/ConfigVersion", 4);

    const auto action = SettingsMigrator::prepare(settings);
    const bool passed =
        action == SettingsMigrationAction::None
        && settings.value("ZJUConnect/Protocol").toString() == "easyconnect";
    if (!passed)
    {
        qCritical() << "migratesKnownVersions failed";
    }
    return passed;
}

bool recommendsResetForUnsupportedVersions()
{
    QTemporaryDir directory;
    QSettings settings(directory.filePath("profile.ini"), QSettings::IniFormat);
    settings.setValue("Common/ConfigVersion", 5);
    const bool passed =
        SettingsMigrator::prepare(settings) ==
        SettingsMigrationAction::RecommendReset;
    if (!passed)
    {
        qCritical() << "recommendsResetForUnsupportedVersions failed";
    }
    return passed;
}

bool removesTcpTunnelModeFromVersionEightProfiles()
{
    QTemporaryDir directory;
    QSettings settings(directory.filePath("profile.ini"), QSettings::IniFormat);
    settings.setValue("Common/ConfigVersion", 8);
    settings.setValue("ZJUConnect/TCPTunnelMode", true);

    const bool passed =
        SettingsMigrator::prepare(settings) == SettingsMigrationAction::None
        && !settings.contains("ZJUConnect/TCPTunnelMode");
    if (!passed)
    {
        qCritical() << "removesTcpTunnelModeFromVersionEightProfiles failed";
    }
    return passed;
}
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    return migratesKnownVersions()
        && recommendsResetForUnsupportedVersions()
        && removesTcpTunnelModeFromVersionEightProfiles() ? 0 : 1;
}
