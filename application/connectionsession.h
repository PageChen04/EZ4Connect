#ifndef CONNECTIONSESSION_H
#define CONNECTIONSESSION_H

#include <QObject>
#include <QTimer>

#include "core/connectionprofile.h"
#include "core/connectionsessionstate.h"
#include "zjuconnectcontroller/zjuconnectcontroller.h"

class ConnectionSession : public QObject
{
Q_OBJECT

public:
    explicit ConnectionSession(QObject *parent = nullptr);

    bool start(const ConnectionProfile &profile, const ReconnectPolicy &policy);
    void stop();
    void submitInput(const QByteArray &data);
    void submitSudoPassword(const QString &password, bool remember);

    ConnectionState state() const;
    bool isActive() const;

signals:
    void stateChanged(ConnectionState state);
    void outputRead(const QString &output);
    void graphCaptcha(const QString &graphFile);
    void smsCode(bool showSkipSecondaryAuthOption);
    void totpCode();
    void ssoAuth();
    void askSudoPass();
    void savedSudoPasswordRejected();
    void reconnectScheduled(int delayMs);
    void finished(ZJU_ERROR error);

private:
    void startCore();
    void handleCoreFinished();
    void handleSudoPasswordRequest();

    ZjuConnectController *controller;
    QTimer reconnectTimer;
    ConnectionSessionState sessionState;
    ConnectionProfile currentProfile;
    QString savedSudoPassword;
    bool sudoPasswordSubmitted = false;
};

#endif // CONNECTIONSESSION_H
