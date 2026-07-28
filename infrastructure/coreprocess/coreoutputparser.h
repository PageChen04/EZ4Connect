#ifndef COREOUTPUTPARSER_H
#define COREOUTPUTPARSER_H

#include <QByteArray>
#include <QString>

enum class CoreOutputEvent
{
    None,
    AskSudoPassword,
    GraphCaptcha,
    SmsCodeWithSkipOption,
    SmsCode,
    TotpCode,
    SsoCallback,
    CaptchaFailed,
    AccessDenied,
    ListenFailed,
    InvalidCredentials,
    BruteForceBlocked,
    LoginFailed,
    InteractiveError,
    AuthNotAvailable,
    AuthExpired,
    ClientFailed,
    CorePanic,
};

class CoreOutputParser
{
public:
    static CoreOutputEvent parse(const QString &output);

    static bool hasInteractivePrompt(const QByteArray &output);
};

#endif // COREOUTPUTPARSER_H
