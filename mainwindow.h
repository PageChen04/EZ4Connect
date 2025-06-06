#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QCloseEvent>
#include <QProcess>
#include <QNetworkReply>
#include <QSettings>

#include "loginwindow/loginwindow.h"
#include "extrasettingwindow/extrasettingwindow.h"
#include "zjuconnectcontroller/zjuconnectcontroller.h"
#include "settingwindow/settingwindow.h"

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow() override;

public slots:

    void cleanUpWhenQuit();

signals:

    void SetModeFinished();

protected:
    void closeEvent(QCloseEvent *e) override;

private:
    void checkUpdate();

    void upgradeSettings();

    void clearLog();

    void addLog(const QString &log);

    void showNotification(
        const QString &title,
        const QString &content,
        QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::MessageIcon::Information
    );

    void initZjuConnect();

    void updateVersionInfo();

    struct {
        QString ui_version, ui_latest;
        QString core_version, core_latest;
    } versionInfo;

    Ui::MainWindow *ui;
    QSystemTrayIcon *trayIcon;
    QMenu *trayMenu;
    QAction *trayShowAction;
    QAction *trayCloseAction;
    ZjuConnectController *zjuConnectController;
    QNetworkAccessManager *checkUpdateNAM;
    QNetworkAccessManager *checkCoreUpdateNAM;
    QSettings *settings;
    QProcess *process;
    QProcess *processForL2tp;
    QProcess *processForL2tpCheck;
    QProcess *processForWebLogin;
    QTimer *l2tpCheckTimer;

    QObject *diagnosisContext;

    SettingWindow *settingWindow;
    LoginWindow *login_window;

    bool isFirstTimeSetMode;

    bool isZjuConnectLinked;
    bool isSystemProxySet;
    ZJU_ERROR zjuConnectError;
};

#endif //MAINWINDOW_H
