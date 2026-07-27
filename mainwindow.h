#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QCloseEvent>
#include <QProcess>
#include <QNetworkReply>
#include <QSettings>
#include <QPointer>

#include "application/connectionsession.h"
#include "application/systemproxysession.h"
#include "loginwindow/loginwindow.h"
#include "sudowindow/sudowindow.h"
#include "ssologinwebview/ssologinwebview.h"
#include "zjuconnectcontroller/zjuconnectcontroller.h"
#include "settingwindow/settingwindow.h"
#include "graphcaptchawindow/graphcaptchawindow.h"
#include "utils/profilemanager.h"

namespace Ui
{
    class MainWindow;
}

class ApplicationLogger;
class CoreLogFile;

class MainWindow : public QMainWindow
{
Q_OBJECT

public:
    explicit MainWindow(ApplicationLogger *logger, QWidget *parent = nullptr);

    ~MainWindow() override;

public slots:

    void cleanUpWhenQuit();

signals:

    void SetModeFinished();

protected:
    void closeEvent(QCloseEvent *e) override;

    void changeEvent(QEvent *event) override;

private:
    void checkUpdate();

    void upgradeSettings();

    void clearLog();

    void resetZjuConnectUi();

    void showNotification(
        const QString &title,
        const QString &content,
        QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::MessageIcon::Information
    );

    void initZjuConnect();

    void updateVersionInfo();

    void setupTrayIcon();

    void setupProfileMenu();

    void refreshProfileMenu();

    bool switchProfile(const QString &profileId);

    void createProfile();

    void renameCurrentProfile();

    void deleteCurrentProfile();

    void gracefullyQuit();

    struct {
        QString ui_version, ui_latest;
        QString core_version, core_latest;
    } versionInfo;

    Ui::MainWindow *ui;
    QSystemTrayIcon *trayIcon;
    QMenu *trayMenu;
    QMenu *trayProfileMenu;
    QAction *trayConnectAction;
    QAction *trayShowAction;
    QAction *trayCloseAction;
    QAction *newProfileAction;
    QAction *renameProfileAction;
    QAction *deleteProfileAction;
    ConnectionSession *connectionSession = nullptr;
    SystemProxySession *systemProxySession = nullptr;
    ApplicationLogger *applicationLogger;
    CoreLogFile *coreLogFile = nullptr;
    QNetworkAccessManager *checkUpdateNAM;
    QNetworkAccessManager *checkCoreUpdateNAM;
    QSettings *settings;
    ProfileManager *profileManager;
    QString currentProfileId;

    QObject *diagnosisContext;

    QPointer<SettingWindow> settingWindow;
    QPointer<LoginWindow> loginWindow;
    QPointer<SudoWindow> sudoWindow;
    QPointer<SsoLoginWebView> ssoLoginWebView;
    QPointer<GraphCaptchaWindow> graphCaptchaWindow;

    bool isFirstTimeSetMode;

};

#endif //MAINWINDOW_H
