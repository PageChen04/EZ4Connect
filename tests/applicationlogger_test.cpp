#include <QCoreApplication>
#include <QDebug>
#include <QLoggingCategory>
#include <QRegularExpression>
#include <QThread>

#include "application/applicationlogger.h"

Q_LOGGING_CATEGORY(backgroundLog, "logger.background")

namespace
{
bool publishesQtAndCoreEntries()
{
    QStringList entries;
    {
        ApplicationLogger logger;
        QObject::connect(&logger, &ApplicationLogger::entryAdded,
                         [&entries](const QString &entry) { entries.append(entry); });

        qInfo().noquote() << "application event";
        QThread *loggingThread = QThread::create(
            []() { qCWarning(backgroundLog).noquote() << "background warning"; }
        );
        loggingThread->start();
        loggingThread->wait();
        delete loggingThread;
        QCoreApplication::processEvents();

        logger.appendCoreOutput("standard output\ncontinued standard output");
        logger.appendCoreOutput("error output");
    }

    const QString timestampPattern = R"(\d{4}/\d{2}/\d{2} \d{2}:\d{2}:\d{2})";
    const bool entriesArePublished = entries.size() == 4
        && QRegularExpression(
               "^\\[INFO\\] " + timestampPattern + " application event$"
           ).match(entries.at(0)).hasMatch()
        && QRegularExpression(
               "^\\[WARNING\\] " + timestampPattern + " background warning$"
           ).match(entries.at(1)).hasMatch()
        && entries.at(2) == "[CORE] standard output\n[CORE] continued standard output"
        && entries.at(3) == "[CORE] error output";

    if (!entriesArePublished)
    {
        qCritical() << "publishesQtAndCoreEntries failed";
        return false;
    }
    return true;
}
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("LoggerTest");
    QCoreApplication::setApplicationVersion("1.0");
    return publishesQtAndCoreEntries() ? 0 : 1;
}
