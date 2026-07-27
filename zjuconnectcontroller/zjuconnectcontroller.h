#ifndef ZJUCONNECTCONTROLLER_H
#define ZJUCONNECTCONTROLLER_H

#include <QtCore>

#include "core/connectionprofile.h"

enum class ZJU_ERROR
{
    NONE,
    INVALID_DETAIL,
    BRUTE_FORCE,
    OTHER_LOGIN_FAILED,
    ACCESS_DENIED,
    LISTEN_FAILED,
    CLIENT_FAILED,
    CAPTCHA_FAILED,
    PROGRAM_NOT_FOUND,
    INTERACTIVE_ERROR,
    AUTH_NOT_AVAILABLE,
    AUTH_EXPIRED,
    OTHER,
};

class ZjuConnectController : public QObject
{
Q_OBJECT

public:
    ZjuConnectController(QWidget* parent);

    ~ZjuConnectController() override;

    void start(const ConnectionProfile &profile);

    void stop();


signals:

    void error(ZJU_ERROR err);

    void outputRead(const QString &output);

    void graphCaptcha(const QString &graphFile);

    void smsCode(bool showSkipSecondaryAuthOption);

    void totpCode();

    void ssoAuth();

    void askSudoPass();

    void finished();

    void write(const QByteArray &data);

private:
    QString copyCoreForAppImage(const QString &programPath);

    QProcess *zjuConnectProcess;

    QTemporaryDir *tempDir = nullptr;

    QString graphFile;

    QFile *logFile = nullptr;
    QTextStream *logStream = nullptr;
    bool stopRequested = false;

public:
    bool savedSudoPassword;
    bool enteredSudoPassword;
    QString sudoPassword;
};

#endif //ZJUCONNECTCONTROLLER_H
