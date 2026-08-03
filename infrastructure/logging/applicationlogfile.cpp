#include "applicationlogfile.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QMetaObject>
#include <QThread>

namespace
{
QString currentTimestamp()
{
    return QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
}
}

ApplicationLogFile::ApplicationLogFile(const QString &filePath, QObject *parent)
    : QObject(parent),
      file(filePath)
{
    if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
    {
        stream.setDevice(&file);
        stream.setEncoding(QStringConverter::Utf8);
        appendEntry(
            "=== Log started at " + currentTimestamp()
            + " with " + QCoreApplication::applicationName()
            + " " + QCoreApplication::applicationVersion()
            + " ==="
        );
    }
}

ApplicationLogFile::~ApplicationLogFile()
{
    if (file.isOpen())
    {
        appendEntry("=== Log ended at " + currentTimestamp() + " ===");
        stream.flush();
        file.close();
    }
}

QString ApplicationLogFile::filePath() const
{
    return file.fileName();
}

bool ApplicationLogFile::isOpen() const
{
    return file.isOpen();
}

void ApplicationLogFile::appendEntry(const QString &entry)
{
    if (QThread::currentThread() != thread())
    {
        QMetaObject::invokeMethod(
            this,
            [this, entry]() { appendEntry(entry); },
            Qt::QueuedConnection
        );
        return;
    }

    if (!file.isOpen())
    {
        return;
    }

    stream << entry;
    if (!entry.endsWith('\n'))
    {
        stream << '\n';
    }
    stream.flush();
}
