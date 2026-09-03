#include "mainwindowcoordinator.h"

#include <memory>

#include "application/connectionsession.h"
#include "application/profileservice.h"
#include "application/systemproxysession.h"
#include "infrastructure/coreprocess/zjuconnectprocess.h"
#include "infrastructure/platform/platformsystemproxybackend.h"
#include "infrastructure/settings/profilemanager.h"
#include "infrastructure/update/updatechecker.h"
#include "presentation/coordinators/authdialogcoordinator.h"

MainWindowCoordinator::MainWindowCoordinator(
    QWidget *parentWidget,
    const QString &overrideConfigPath,
    QObject *parent
)
    : QObject(parent),
      profileService(new ProfileService(
          std::make_unique<ProfileManager>(),
          overrideConfigPath,
          this
      )),
      connectionSession(new ConnectionSession(new ZjuConnectProcess(), this)),
      systemProxySession(new SystemProxySession(
          std::make_unique<PlatformSystemProxyBackend>(),
          this
      )),
      updateChecker(new UpdateChecker(this)),
      authenticationCoordinator(new AuthDialogCoordinator(
          parentWidget,
          profileService->settings(),
          this
      ))
{
    connect(
        connectionSession,
        &ConnectionSession::askSudoPass,
        authenticationCoordinator,
        &AuthDialogCoordinator::requestSudoPassword
    );
    connect(
        connectionSession,
        &ConnectionSession::graphCaptcha,
        authenticationCoordinator,
        &AuthDialogCoordinator::requestGraphCaptcha
    );
    connect(
        connectionSession,
        &ConnectionSession::smsCode,
        authenticationCoordinator,
        &AuthDialogCoordinator::requestSmsCode
    );
    connect(
        connectionSession,
        &ConnectionSession::totpCode,
        authenticationCoordinator,
        &AuthDialogCoordinator::requestTotpCode
    );
    // aTrust 等协议的“动态/随机码”二次认证（核心会输出 "Please enter rand
    // code:"）。该提示此前只被 hasInteractivePrompt() 识别（会冲刷输出行），
    // 却没有对应的事件分支与对话框，导致验证码窗口不弹出。这里复用短信验证
    // 码输入框（输入格式一致：验证码 + 换行），让用户输入收到的短信验证码。
    connect(
        connectionSession,
        &ConnectionSession::randCode,
        authenticationCoordinator,
        [this]() { authenticationCoordinator->requestSmsCode(false); }
    );
    // aTrust RADIUS Access-Challenge：核心输出提示后，弹验证码输入框并把输入写回核心。
    connect(
        connectionSession,
        &ConnectionSession::radiusCode,
        authenticationCoordinator,
        &AuthDialogCoordinator::requestRadiusCode
    );
    connect(
        connectionSession,
        &ConnectionSession::ssoAuth,
        authenticationCoordinator,
        &AuthDialogCoordinator::requestSsoLogin
    );
    connect(
        authenticationCoordinator,
        &AuthDialogCoordinator::sudoPasswordSubmitted,
        connectionSession,
        &ConnectionSession::submitSudoPassword
    );
    connect(
        authenticationCoordinator,
        &AuthDialogCoordinator::interactiveInputSubmitted,
        connectionSession,
        &ConnectionSession::submitInput
    );
    connect(
        authenticationCoordinator,
        &AuthDialogCoordinator::interactiveInputCancelled,
        connectionSession,
        &ConnectionSession::cancelInteractiveInput
    );

    connect(
        systemProxySession,
        &SystemProxySession::operationFinished,
        this,
        [this](bool enabled)
        {
            if (enabled && !connectionSession->isActive())
            {
                systemProxySession->disable();
            }
        }
    );
    connect(
        connectionSession,
        &ConnectionSession::finished,
        this,
        [this](ZJU_ERROR)
        {
            if (systemProxySession->isEnabled())
            {
                systemProxySession->disable();
            }
        }
    );
}

ProfileService *MainWindowCoordinator::profiles() const
{
    return profileService;
}

ConnectionSession *MainWindowCoordinator::connection() const
{
    return connectionSession;
}

SystemProxySession *MainWindowCoordinator::systemProxy() const
{
    return systemProxySession;
}

UpdateChecker *MainWindowCoordinator::updates() const
{
    return updateChecker;
}

AuthDialogCoordinator *
MainWindowCoordinator::authenticationDialogs() const
{
    return authenticationCoordinator;
}

void MainWindowCoordinator::prepareForShutdown()
{
    systemProxySession->clearBeforeShutdown();
}
