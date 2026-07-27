#include "corelogfile.h"

#include <QMetaObject>
#include <QThread>

CoreLogFile::CoreLogFile(const QString &filePath, QObject *parent)
    : QObject(parent),
      file(filePath)
{
    if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
    {
        stream.setDevice(&file);
        stream.setEncoding(QStringConverter::Utf8);
    }
}

CoreLogFile::~CoreLogFile()
{
    if (file.isOpen())
    {
        stream.flush();
        file.close();
    }
}

QString CoreLogFile::filePath() const
{
    return file.fileName();
}

bool CoreLogFile::isOpen() const
{
    return file.isOpen();
}

void CoreLogFile::appendOutput(const QString &output)
{
    if (QThread::currentThread() != thread())
    {
        QMetaObject::invokeMethod(
            this,
            [this, output]() { appendOutput(output); },
            Qt::QueuedConnection
        );
        return;
    }

    if (!file.isOpen())
    {
        return;
    }

    stream << output;
    if (!output.endsWith('\n'))
    {
        stream << '\n';
    }
    stream.flush();
}
