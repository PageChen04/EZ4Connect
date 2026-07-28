#ifndef COREPROCESS_H
#define COREPROCESS_H

#include <QObject>

#include "core/connectionerror.h"
#include "core/connectionprofile.h"

class CoreProcess : public QObject
{
    Q_OBJECT

public:
    explicit CoreProcess(QObject *parent = nullptr)
        : QObject(parent)
    {
    }
    ~CoreProcess() override = default;

    virtual void start(const ConnectionProfile &profile) = 0;
    virtual void stop() = 0;
    virtual void writeInput(const QByteArray &data) = 0;

signals:
    void error(ZJU_ERROR error);
    void outputRead(const QString &output);
    void graphCaptcha(const QString &graphFile);
    void smsCode(bool showSkipSecondaryAuthOption);
    void totpCode();
    void ssoAuth();
    void askSudoPass();
    void started();
    void finished();
};

#endif // COREPROCESS_H
