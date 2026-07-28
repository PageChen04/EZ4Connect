#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QTemporaryDir>

#include "infrastructure/logging/corelogfile.h"

namespace
{
bool writesOnlyRawCoreOutput()
{
    QTemporaryDir temporaryDirectory;
    if (!temporaryDirectory.isValid())
    {
        qCritical() << "Unable to create temporary directory";
        return false;
    }

    const QString logPath = temporaryDirectory.filePath("core.log");
    {
        CoreLogFile logFile(logPath);
        if (!logFile.isOpen() || logFile.filePath() != logPath)
        {
            qCritical() << "Core log file did not open the requested path";
            return false;
        }

        logFile.appendOutput("standard output\ncontinued standard output");
        logFile.appendOutput("error output");
    }

    QFile logFile(logPath);
    if (!logFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qCritical() << "Unable to read generated core log";
        return false;
    }

    const QString contents = QString::fromUtf8(logFile.readAll());
    if (contents != "standard output\ncontinued standard output\nerror output\n")
    {
        qCritical().noquote() << "writesOnlyRawCoreOutput failed:\n" << contents;
        return false;
    }
    return true;
}
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    return writesOnlyRawCoreOutput() ? 0 : 1;
}
