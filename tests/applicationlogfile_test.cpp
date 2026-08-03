#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QTemporaryDir>

#include "infrastructure/logging/applicationlogfile.h"

namespace
{
bool writesApplicationLogEntries()
{
    QTemporaryDir temporaryDirectory;
    if (!temporaryDirectory.isValid())
    {
        qCritical() << "Unable to create temporary directory";
        return false;
    }

    const QString logPath = temporaryDirectory.filePath("core.log");
    {
        ApplicationLogFile logFile(logPath);
        if (!logFile.isOpen() || logFile.filePath() != logPath)
        {
            qCritical() << "Application log file did not open the requested path";
            return false;
        }

        logFile.appendEntry("[INFO] application event");
        logFile.appendEntry("[CORE] standard output\n[CORE] continued standard output");
    }

    QFile logFile(logPath);
    if (!logFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qCritical() << "Unable to read generated application log";
        return false;
    }

    const QString contents = QString::fromUtf8(logFile.readAll());
    if (!contents.contains("=== Log started at ")
        || !contents.contains(" with LogFileTest 1.0 ===\n")
        || !contents.contains("[INFO] application event\n")
        || !contents.contains("[CORE] standard output\n[CORE] continued standard output\n")
        || !contents.contains("=== Log ended at "))
    {
        qCritical().noquote() << "writesApplicationLogEntries failed:\n" << contents;
        return false;
    }
    return true;
}
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("LogFileTest");
    QCoreApplication::setApplicationVersion("1.0");
    return writesApplicationLogEntries() ? 0 : 1;
}
