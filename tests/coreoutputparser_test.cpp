#include <QCoreApplication>
#include <QDebug>
#include <QList>

#include "core/coreoutputparser.h"

namespace
{
struct ParserCase
{
    const char *output;
    CoreOutputEvent expected;
};

bool recognizesCoreProtocol()
{
    const QList<ParserCase> cases{
        {"SUDO_ASK_PASS", CoreOutputEvent::AskSudoPassword},
        {"Graph check code saved to /tmp/graph.jpg", CoreOutputEvent::GraphCaptcha},
        {"Please enter the SMS verification code: ", CoreOutputEvent::SmsCodeWithSkipOption},
        {"Please enter your SMS code:", CoreOutputEvent::SmsCode},
        {"Please enter your TOTP code:", CoreOutputEvent::TotpCode},
        {"Please enter the callback url:", CoreOutputEvent::SsoCallback},
        {"graph check code still required after second login attempt", CoreOutputEvent::CaptchaFailed},
        {"Access is denied.", CoreOutputEvent::AccessDenied},
        {"listen failed on 127.0.0.1", CoreOutputEvent::ListenFailed},
        {"Invalid username or password!", CoreOutputEvent::InvalidCredentials},
        {"ticket is empty", CoreOutputEvent::InvalidCredentials},
        {"You are trying brute-force login on this IP address.", CoreOutputEvent::BruteForceBlocked},
        {"Login failed", CoreOutputEvent::LoginFailed},
        {"too many login failures", CoreOutputEvent::LoginFailed},
        {"unexpected newline", CoreOutputEvent::InteractiveError},
        {"auth type/login domain combination not found", CoreOutputEvent::AuthNotAvailable},
        {"invalid SID", CoreOutputEvent::AuthExpired},
        {"l3-tunnel tunnel auth failed:", CoreOutputEvent::AuthExpired},
        {"client setup error", CoreOutputEvent::ClientFailed},
        {"panic: runtime error", CoreOutputEvent::CorePanic},
        {"ordinary core output", CoreOutputEvent::None},
    };

    for (const ParserCase &testCase : cases)
    {
        const CoreOutputEvent actual = CoreOutputParser::parse(QString::fromUtf8(testCase.output));
        if (actual != testCase.expected)
        {
            qCritical() << "recognizesCoreProtocol failed for" << testCase.output
                        << "expected" << static_cast<int>(testCase.expected)
                        << "actual" << static_cast<int>(actual);
            return false;
        }
    }
    return true;
}

bool preservesExistingFirstMatchPrecedence()
{
    const QString combinedOutput = "Please enter your TOTP code:\npanic: later line";
    const CoreOutputEvent actual = CoreOutputParser::parse(combinedOutput);
    if (actual != CoreOutputEvent::TotpCode)
    {
        qCritical() << "preservesExistingFirstMatchPrecedence failed";
        return false;
    }
    return true;
}
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    return recognizesCoreProtocol() && preservesExistingFirstMatchPrecedence() ? 0 : 1;
}
