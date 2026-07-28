#include "coreexecutable.h"

#include <QCoreApplication>
#include <QFileInfo>
#include <QProcess>
#include <QSysInfo>

#include <stdexcept>

QString CoreExecutable::path()
{
    const QString fileName =
        QSysInfo::productType() == "windows" ? "zju-connect.exe" : "zju-connect";
    const QString bundledPath = QCoreApplication::applicationDirPath() + "/" + fileName;
    return QFileInfo::exists(bundledPath) ? bundledPath : fileName;
}

QString CoreExecutable::version(QObject *parent)
{
    QProcess process(parent);
    process.start(path(), {"-version"});
    if (!process.waitForStarted())
    {
        throw std::runtime_error("核心无法启动");
    }
    if (!process.waitForFinished())
    {
        throw std::runtime_error("核心运行超时");
    }

    const QByteArray errorOutput = process.readAllStandardError();
    if (!errorOutput.isEmpty())
    {
        throw std::runtime_error(errorOutput);
    }

    const QString output = process.readAllStandardOutput();
    const QString prefix("ZJU Connect v");
    if (!output.startsWith(prefix))
    {
        throw std::runtime_error("无法解析核心版本号");
    }
    return output.mid(prefix.size()).trimmed();
}
