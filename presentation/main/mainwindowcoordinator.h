#ifndef MAINWINDOWCOORDINATOR_H
#define MAINWINDOWCOORDINATOR_H

#include <QObject>

class AuthDialogCoordinator;
class ConnectionSession;
class ProfileService;
class SystemProxySession;
class UpdateChecker;
class QWidget;

class MainWindowCoordinator : public QObject
{
    Q_OBJECT

public:
    MainWindowCoordinator(
        QWidget *parentWidget,
        const QString &overrideConfigPath,
        QObject *parent = nullptr
    );

    ProfileService *profiles() const;
    ConnectionSession *connection() const;
    SystemProxySession *systemProxy() const;
    UpdateChecker *updates() const;
    AuthDialogCoordinator *authenticationDialogs() const;

    void prepareForShutdown();

private:
    ProfileService *profileService;
    ConnectionSession *connectionSession;
    SystemProxySession *systemProxySession;
    UpdateChecker *updateChecker;
    AuthDialogCoordinator *authenticationCoordinator;
};

#endif // MAINWINDOWCOORDINATOR_H
