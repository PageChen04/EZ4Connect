#include "coreoutputbuffer.h"

QList<QByteArray> CoreOutputBuffer::append(const QByteArray &data)
{
    buffer.append(data);

    QList<QByteArray> lines;
    qsizetype lineStart = 0;
    qsizetype lineEnd = buffer.indexOf('\n', lineStart);
    while (lineEnd >= 0)
    {
        QByteArray line = buffer.mid(lineStart, lineEnd - lineStart);
        if (line.endsWith('\r'))
        {
            line.chop(1);
        }
        lines.append(line);
        lineStart = lineEnd + 1;
        lineEnd = buffer.indexOf('\n', lineStart);
    }

    if (lineStart > 0)
    {
        buffer.remove(0, lineStart);
    }
    return lines;
}

bool CoreOutputBuffer::hasPendingData() const
{
    return !buffer.isEmpty();
}

const QByteArray &CoreOutputBuffer::pendingData() const
{
    return buffer;
}

QByteArray CoreOutputBuffer::takePendingData()
{
    QByteArray pending = buffer;
    buffer.clear();
    return pending;
}
