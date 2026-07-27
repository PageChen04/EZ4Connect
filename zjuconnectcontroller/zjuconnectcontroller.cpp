#include "zjuconnectcontroller.h"
#include "core/corecommandbuilder.h"
#include "core/coreoutputparser.h"
#include "utils/utils.h"
#include <qcontainerfwd.h>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>

ZjuConnectController::ZjuConnectController(QObject *parent) : QObject(parent)
{
    zjuConnectProcess = new QProcess(this);

    // 初始化日志文件
    logFile = new QFile(Utils::getLogFilePath());
    if (logFile->open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
    {
        logStream = new QTextStream(logFile);
        logStream->setEncoding(QStringConverter::Utf8);
        QString startMsg = "=== Log started at " + QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss") +
                           " with " + QCoreApplication::applicationName() + " " + QCoreApplication::applicationVersion() +
                           " ===\n";
        *logStream << startMsg;
        logStream->flush();
    }

    auto outputProcess = [&](const QString &output)
        {
            emit outputRead(output);

            // 写入日志文件
            if (logStream != nullptr)
            {
                *logStream << output;
                if (!output.endsWith('\n'))
                {
                    *logStream << '\n';
                }
                logStream->flush();
            }

            switch (CoreOutputParser::parse(output))
            {
            case CoreOutputEvent::AskSudoPassword:
                emit askSudoPass();
                break;
            case CoreOutputEvent::GraphCaptcha:
                emit graphCaptcha(graphFile);
                break;
            case CoreOutputEvent::SmsCodeWithSkipOption:
                emit smsCode(true);
                break;
            case CoreOutputEvent::SmsCode:
                emit smsCode(false);
                break;
            case CoreOutputEvent::TotpCode:
                emit totpCode();
                break;
            case CoreOutputEvent::SsoCallback:
                emit ssoAuth();
                break;
            case CoreOutputEvent::CaptchaFailed:
                emit error(ZJU_ERROR::CAPTCHA_FAILED);
                break;
            case CoreOutputEvent::AccessDenied:
                emit error(ZJU_ERROR::ACCESS_DENIED);
                break;
            case CoreOutputEvent::ListenFailed:
                emit error(ZJU_ERROR::LISTEN_FAILED);
                break;
            case CoreOutputEvent::InvalidCredentials:
                emit error(ZJU_ERROR::INVALID_DETAIL);
                break;
            case CoreOutputEvent::BruteForceBlocked:
                emit error(ZJU_ERROR::BRUTE_FORCE);
                break;
            case CoreOutputEvent::LoginFailed:
                emit error(ZJU_ERROR::OTHER_LOGIN_FAILED);
                break;
            case CoreOutputEvent::InteractiveError:
                emit error(ZJU_ERROR::INTERACTIVE_ERROR);
                break;
            case CoreOutputEvent::AuthNotAvailable:
                emit error(ZJU_ERROR::AUTH_NOT_AVAILABLE);
                break;
            case CoreOutputEvent::AuthExpired:
                emit error(ZJU_ERROR::AUTH_EXPIRED);
                break;
            case CoreOutputEvent::ClientFailed:
                emit error(ZJU_ERROR::CLIENT_FAILED);
                break;
            case CoreOutputEvent::CorePanic:
                emit error(ZJU_ERROR::OTHER);
                break;
            case CoreOutputEvent::None:
                break;
            }
        };

    connect(zjuConnectProcess, &QProcess::readyReadStandardOutput, this, [&, outputProcess]()
    {
        QString output = Utils::consoleOutputToQString(zjuConnectProcess->readAllStandardOutput());

		outputProcess(output);
    });

    connect(zjuConnectProcess, &QProcess::readyReadStandardError, this, [&, outputProcess]()
    {
        QString output = Utils::consoleOutputToQString(zjuConnectProcess->readAllStandardError());

		outputProcess(output);
    });

    connect(zjuConnectProcess, &QProcess::errorOccurred, this, [&](QProcess::ProcessError err)
    {
        if (stopRequested)
        {
            return;
        }
        QString timeString = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
        QString errorString = zjuConnectProcess->errorString();
        emit outputRead(timeString + " 退出原因：" + errorString);

        if (errorString.contains("No such file or directory") || errorString.contains("not found") || errorString.contains("找不到"))
        {
            emit outputRead(timeString + " 核心路径：" + zjuConnectProcess->program());
            emit error(ZJU_ERROR::PROGRAM_NOT_FOUND);
        }
    });

    connect(zjuConnectProcess, &QProcess::finished, this, [&]()
    {
        stopRequested = false;
        QString timeString = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
        emit outputRead(timeString + " 退出原因：" "进程已结束");
        emit finished();
    });
}

QString ZjuConnectController::copyCoreForAppImage(const QString &programPath)
{
#if defined(Q_OS_UNIX)
    static QString cachedSourcePath;
    static QString cachedTempPath;

    if (!qEnvironmentVariableIsSet("APPIMAGE")) {
        return programPath;
    }

    if (cachedSourcePath == programPath && !cachedTempPath.isEmpty() && QFileInfo::exists(cachedTempPath)) {
        return cachedTempPath;
    }

    const QFileInfo sourceInfo(programPath);
    if (!sourceInfo.exists() || !sourceInfo.isFile()) {
        return programPath;
    }

    const QString tempRoot = QStandardPaths::writableLocation(QStandardPaths::TempLocation)
                             + "/EZ4Connect-" + QString::number(QCoreApplication::applicationPid());
    QDir().mkpath(tempRoot);

    const QString tempPath = tempRoot + "/" + sourceInfo.fileName();
    if (QFileInfo::exists(tempPath)) {
        QFile::remove(tempPath);
    }

    if (!QFile::copy(programPath, tempPath)) {
        return programPath;
    }

    QFile::setPermissions(tempPath,
                          QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner |
                          QFileDevice::ReadGroup | QFileDevice::ExeGroup |
                          QFileDevice::ReadOther | QFileDevice::ExeOther);
    cachedSourcePath = programPath;
    cachedTempPath = tempPath;
    return tempPath;
#else
    return programPath;
#endif
}

void ZjuConnectController::start(const ConnectionProfile &profile)
{
    CoreRuntimePaths runtimePaths;

    if (profile.endpoint.protocol == "atrust")
    {
        // 图形验证码文件路径
        if (tempDir == nullptr)
        {
            tempDir = new QTemporaryDir;
            tempDir->setAutoRemove(true);
        }
        graphFile = tempDir->filePath("graph.jpg");
        runtimePaths.graphCodeFile = graphFile;
        runtimePaths.clientDataFile = Utils::getClientDataPath(profile.profileId);
    }

    const CoreCommand command = CoreCommandBuilder::build(profile, runtimePaths);
    QString timeString = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
    emit outputRead(timeString + " VPN 启动！参数：" + command.loggableArguments.join(' '));

    if (!profile.credentials.totpSecret.isEmpty())
    {
        emit outputRead(timeString + " 使用了 TOTP");
    }
    if (!profile.credentials.certFile.isEmpty())
    {
        emit outputRead(timeString + " 使用了证书文件");
    }

    QString programToStart = profile.program;
    QStringList finalArgs = command.arguments;

#if defined(Q_OS_UNIX)
    if (profile.tunnel.tunMode && !Utils::isRunningAsAdmin())
    {
        programToStart = copyCoreForAppImage(programToStart);

        QStringList sudoArgs;
        sudoArgs << "-p" << "SUDO_ASK_PASS";
        sudoArgs << "-S";
        sudoArgs << programToStart << finalArgs;
        programToStart = "sudo";
        finalArgs = sudoArgs;
    }
#endif

    zjuConnectProcess->start(programToStart, finalArgs);
    zjuConnectProcess->waitForStarted();
    if (zjuConnectProcess->state() == QProcess::NotRunning)
    {
        emit finished();
    }
    else
    {
        emit started();
    }
}

void ZjuConnectController::stop()
{
    if (zjuConnectProcess->state() == QProcess::NotRunning)
    {
        return;
    }

    if (!stopRequested)
    {
        stopRequested = true;
        zjuConnectProcess->terminate();
    }
    else
    {
        zjuConnectProcess->kill();
    }
}

void ZjuConnectController::writeInput(const QByteArray &data)
{
    zjuConnectProcess->write(data);
}

ZjuConnectController::~ZjuConnectController()
{
    stop();

    // 关闭日志文件
    if (logStream != nullptr)
    {
        QString endMsg = "=== Log ended at " + QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss") + " ===\n";
        *logStream << endMsg;
        logStream->flush();
        delete logStream;
        logStream = nullptr;
    }

    if (logFile != nullptr)
    {
        if (logFile->isOpen())
        {
            logFile->close();
        }
        delete logFile;
        logFile = nullptr;
    }
}
