#ifndef ZJUCONNECTCONTROLLER_H
#define ZJUCONNECTCONTROLLER_H

#include <QtCore>

#include "core/connectionerror.h"
#include "core/connectionprofile.h"
#include "core/coreoutputbuffer.h"

class ZjuConnectController : public QObject
{
Q_OBJECT

public:
    explicit ZjuConnectController(QObject *parent = nullptr);

    ~ZjuConnectController() override;

    void start(const ConnectionProfile &profile);

    void stop();

    void writeInput(const QByteArray &data);

signals:

    void error(ZJU_ERROR err);

    void outputRead(const QString &output);

    void graphCaptcha(const QString &graphFile);

    void smsCode(bool showSkipSecondaryAuthOption);

    void totpCode();

    void ssoAuth();

    void askSudoPass();

    void started();

    void finished();

private:
    QString copyCoreForAppImage(const QString &programPath);

    void processOutput(CoreOutputBuffer &buffer, const QByteArray &data, bool flushPending = false);

    void processOutputLines(const QList<QByteArray> &lines);

    QProcess *zjuConnectProcess;

    QTemporaryDir *tempDir = nullptr;

    QString graphFile;

    CoreOutputBuffer standardOutputBuffer;
    CoreOutputBuffer standardErrorBuffer;
    bool stopRequested = false;

};

#endif //ZJUCONNECTCONTROLLER_H
