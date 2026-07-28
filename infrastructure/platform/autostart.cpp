#include "autostart.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QProcess>
#include <QSettings>
#include <QStandardPaths>
#include <QTextStream>

namespace
{
QString nativeApplicationPath()
{
    return QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
}

QString macApplicationBundlePath()
{
#ifdef Q_OS_MAC
    QDir directory(QCoreApplication::applicationDirPath());
    directory.cdUp();
    directory.cdUp();
    return directory.absolutePath();
#else
    return {};
#endif
}

QString macApplicationBundleName()
{
#ifdef Q_OS_MAC
    return QFileInfo(macApplicationBundlePath()).baseName();
#else
    return {};
#endif
}
}

void AutoStart::setEnabled(bool enabled)
{
#if defined(Q_OS_WINDOWS)
    QSettings settings(
        R"(HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Run)",
        QSettings::NativeFormat
    );
    if (enabled)
    {
        settings.setValue(QApplication::applicationName(), nativeApplicationPath());
    }
    else
    {
        settings.remove(QApplication::applicationName());
    }
    settings.sync();
#elif defined(Q_OS_MACOS)
    {
        const QStringList arguments{
            "-e",
            "tell application \"System Events\" to delete login item \"" +
                macApplicationBundleName() + "\""
        };
        QProcess process;
        process.start("osascript", arguments);
        process.waitForFinished();
        const QString error = process.readAllStandardError();
        if (!enabled && process.error() != QProcess::UnknownError)
        {
            QMessageBox::critical(
                nullptr,
                "取消开机自启动失败",
                "无法删除登录项：" + process.errorString()
            );
            return;
        }
        if (!enabled && process.exitCode() != 0)
        {
            QMessageBox::critical(nullptr, "取消开机自启动失败", "无法删除登录项：" + error);
            return;
        }
    }
    if (enabled)
    {
        const QStringList arguments{
            "-e",
            "tell application \"System Events\" to make login item at end with properties "
            "{path:\"" + macApplicationBundlePath() + "\", hidden:false}"
        };
        QProcess process;
        process.start("osascript", arguments);
        process.waitForFinished();
        if (process.error() != QProcess::UnknownError)
        {
            QMessageBox::critical(
                nullptr,
                "设置开机自启动失败",
                "无法创建登录项：" + process.errorString()
            );
            return;
        }
        if (process.exitCode() != 0)
        {
            QMessageBox::critical(
                nullptr,
                "设置开机自启动失败",
                "无法创建登录项：" + process.readAllStandardError()
            );
        }
    }
#elif defined(Q_OS_LINUX)
    const QString directoryPath =
        QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/autostart/";
    QDir directory(directoryPath);
    QFile desktopFile(directoryPath + QApplication::applicationName() + ".desktop");

    if (directory.exists() && desktopFile.exists() && !desktopFile.remove())
    {
        QMessageBox::critical(
            nullptr,
            "取消开机自启动失败",
            "无法删除 .desktop 文件：" + desktopFile.fileName()
        );
        return;
    }
    if (!enabled)
    {
        return;
    }
    if (!directory.exists() && !directory.mkpath("."))
    {
        QMessageBox::critical(
            nullptr,
            "设置开机自启动失败",
            "无法创建 autostart 目录：" + directoryPath
        );
        return;
    }
    if (!desktopFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::critical(
            nullptr,
            "设置开机自启动失败",
            "无法创建 .desktop 文件：" + desktopFile.fileName()
        );
        return;
    }

    QTextStream output(&desktopFile);
    output << "[Desktop Entry]\n";
    output << "Type=Application\n";
    output << "Name=" << QApplication::applicationName() << "\n";
    output << "Exec=" << nativeApplicationPath() << "\n";
    output << "X-GNOME-Autostart-enabled=true\n";
#else
    Q_UNUSED(enabled)
#endif
}
