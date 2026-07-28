#include <QCoreApplication>
#include <QDebug>

#include "infrastructure/coreprocess/coreoutputbuffer.h"

namespace
{
bool buffersFragmentedLines()
{
    CoreOutputBuffer buffer;
    const bool firstChunkBuffered = buffer.append("first half").isEmpty();
    const QList<QByteArray> lines = buffer.append(" second half\nnext");
    const bool completeLineReturned = lines == QList<QByteArray>{"first half second half"};
    const bool remainderPreserved = buffer.pendingData() == "next";

    if (!firstChunkBuffered || !completeLineReturned || !remainderPreserved)
    {
        qCritical() << "buffersFragmentedLines failed";
        return false;
    }
    return true;
}

bool extractsMultipleLinesAndNormalizesCrLf()
{
    CoreOutputBuffer buffer;
    const QList<QByteArray> lines = buffer.append("first\r\nsecond\n\n");
    const QList<QByteArray> expected{"first", "second", ""};
    if (lines != expected || buffer.hasPendingData())
    {
        qCritical() << "extractsMultipleLinesAndNormalizesCrLf failed";
        return false;
    }
    return true;
}

bool preservesSplitUtf8Characters()
{
    CoreOutputBuffer buffer;
    const QByteArray output = QStringLiteral("日志内容").toUtf8() + '\n';
    const qsizetype splitPosition = 2;

    const bool firstChunkBuffered = buffer.append(output.first(splitPosition)).isEmpty();
    const QList<QByteArray> lines = buffer.append(output.sliced(splitPosition));
    if (!firstChunkBuffered || lines != QList<QByteArray>{QStringLiteral("日志内容").toUtf8()})
    {
        qCritical() << "preservesSplitUtf8Characters failed";
        return false;
    }
    return true;
}

bool takesPendingData()
{
    CoreOutputBuffer buffer;
    buffer.append("prompt without newline");
    const QByteArray pending = buffer.takePendingData();
    if (pending != "prompt without newline" || buffer.hasPendingData())
    {
        qCritical() << "takesPendingData failed";
        return false;
    }
    return true;
}
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    return buffersFragmentedLines()
               && extractsMultipleLinesAndNormalizesCrLf()
               && preservesSplitUtf8Characters()
               && takesPendingData()
           ? 0
           : 1;
}
