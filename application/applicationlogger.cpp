#include "applicationlogger.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QIODevice>
#include <QMetaObject>
#include <QRecursiveMutex>
#include <QThread>

#include <cstdio>

namespace
{
ApplicationLogger *messageHandlerTarget = nullptr;
QRecursiveMutex messageHandlerMutex;

QString currentTimestamp()
{
    return QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
}

QString messageTypeName(QtMsgType type)
{
    switch (type)
    {
    case QtDebugMsg:
        return "DEBUG";
    case QtInfoMsg:
        return "INFO";
    case QtWarningMsg:
        return "WARNING";
    case QtCriticalMsg:
        return "CRITICAL";
    case QtFatalMsg:
        return "FATAL";
    }

    return "UNKNOWN";
}
}

ApplicationLogger::ApplicationLogger(QObject *parent)
    : QObject(parent),
      standardOutput(stdout, QIODevice::WriteOnly)
{
    writeStandardOutput(
        "=== Log started at " + currentTimestamp()
        + " with " + QCoreApplication::applicationName()
        + " " + QCoreApplication::applicationVersion()
        + " ==="
    );

    {
        QMutexLocker locker(&messageHandlerMutex);
        messageHandlerTarget = this;
        previousMessageHandler = qInstallMessageHandler(ApplicationLogger::qtMessageHandler);
    }
}

ApplicationLogger::~ApplicationLogger()
{
    {
        QMutexLocker locker(&messageHandlerMutex);
        if (messageHandlerTarget == this)
        {
            messageHandlerTarget = nullptr;
            qInstallMessageHandler(previousMessageHandler);
        }
    }

    writeStandardOutput("=== Log ended at " + currentTimestamp() + " ===");
}

void ApplicationLogger::appendMessage(const QString &prefix, const QString &message)
{
    if (QThread::currentThread() != thread())
    {
        QMetaObject::invokeMethod(
            this,
            [this, prefix, message]() { appendMessage(prefix, message); },
            Qt::QueuedConnection
        );
        return;
    }

    publishEntry(prefix + " " + currentTimestamp() + " " + message.trimmed());
}

void ApplicationLogger::appendCoreOutput(const QString &output)
{
    if (QThread::currentThread() != thread())
    {
        QMetaObject::invokeMethod(
            this,
            [this, output]() { appendCoreOutput(output); },
            Qt::QueuedConnection
        );
        return;
    }

    QStringList lines = output.split('\n', Qt::KeepEmptyParts);
    for (QString &line : lines)
    {
        line.prepend("[CORE] ");
    }
    publishEntry(lines.join('\n'));
}

void ApplicationLogger::qtMessageHandler(
    QtMsgType type,
    const QMessageLogContext &context,
    const QString &message
)
{
    Q_UNUSED(context)

    QMutexLocker locker(&messageHandlerMutex);
    if (messageHandlerTarget != nullptr)
    {
        messageHandlerTarget->appendQtMessage(type, message);
    }
}

void ApplicationLogger::appendQtMessage(QtMsgType type, const QString &message)
{
    appendMessage("[" + messageTypeName(type) + "]", message);
}

void ApplicationLogger::publishEntry(const QString &entry)
{
    emit entryAdded(entry);
    writeStandardOutput(entry);
}

void ApplicationLogger::writeStandardOutput(const QString &output)
{
    standardOutput << output;
    if (!output.endsWith('\n'))
    {
        standardOutput << '\n';
    }
    standardOutput.flush();
}
