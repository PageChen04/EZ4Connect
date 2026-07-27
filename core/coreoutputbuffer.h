#ifndef COREOUTPUTBUFFER_H
#define COREOUTPUTBUFFER_H

#include <QByteArray>
#include <QList>

class CoreOutputBuffer
{
public:
    QList<QByteArray> append(const QByteArray &data);
    bool hasPendingData() const;
    const QByteArray &pendingData() const;
    QByteArray takePendingData();

private:
    QByteArray buffer;
};

#endif // COREOUTPUTBUFFER_H
