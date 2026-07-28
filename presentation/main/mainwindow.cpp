#include <QMessageBox>
#include <QSysInfo>
#include <QNetworkInterface>
#include <QClipboard>
#include <QDesktopServices>
#include <QDebug>
#include <QFileInfo>
#include <QCoreApplication>
#include <QActionGroup>
#include <QInputDialog>

#include "mainwindow.h"

#include "application/applicationlogger.h"
#include "application/applicationconstants.h"
#include "application/commandlineoptions.h"
#include "application/settingsmigrator.h"
#include "infrastructure/coreprocess/devicetrust.h"
#include "infrastructure/logging/corelogfile.h"
#include "infrastructure/storage/applicationpaths.h"
#include "infrastructure/update/updatechecker.h"
#include "presentation/coordinators/connectionuicontroller.h"
#include "presentation/presentationhelpers.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(ApplicationLogger *logger, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    applicationLogger(logger)
{
    const QString overrideConfigPath =
        CommandLineOptions::value(QCoreApplication::arguments(), "--config-path");
    coordinator = new MainWindowCoordinator(this, overrideConfigPath, this);
    profileService = coordinator->profiles();
    currentProfileId = profileService->currentProfileId();
    settings = profileService->settings();

    upgradeSettings();

    ui->setupUi(this);
    authenticationDialogs = coordinator->authenticationDialogs();
    connectionSession = coordinator->connection();
    systemProxySession = coordinator->systemProxy();
    coreLogFile = new CoreLogFile(ApplicationPaths::logFile(), this);
    updateChecker = coordinator->updates();
    connect(updateChecker, &UpdateChecker::versionInfoChanged, this,
            [this](const VersionInfo &) { updateVersionInfo(); });
    connect(updateChecker, &UpdateChecker::checkFailed, this,
            [this](UpdateComponent component, const QString &)
            {
                const QString componentName =
                    component == UpdateComponent::Ui ? QStringLiteral("UI") : QStringLiteral("核心");
                ui->versionLabel->setText(
                    "当前版本：" + QApplication::applicationVersion() +
                    "\n检查" + componentName + "更新失败\n"
                );
            });
    connect(updateChecker, &UpdateChecker::uiUpdateAvailable, this,
            [this](const QString &latestVersion)
            {
                QMessageBox msgBox(this);
                msgBox.setText("UI 版本更新");
                msgBox.setInformativeText(
                    "存在 UI 版本更新：" + latestVersion + "\n是否前往 Github 发布页面查看？"
                );
                msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
                msgBox.setDefaultButton(QMessageBox::Ok);

                if (msgBox.exec() == QMessageBox::Ok)
                {
                    QDesktopServices::openUrl(
                        QUrl(
                            "https://github.com/" +
                            ApplicationConstants::RepositoryName +
                            "/releases/latest"
                        )
                    );
                }
            });
    connect(applicationLogger, &ApplicationLogger::entryAdded, this,
            [this](const QString &entry)
            {
                ui->logPlainTextEdit->appendPlainText(entry);
            });
    connect(systemProxySession, &SystemProxySession::busyChanged, this,
            [this](bool busy)
            {
                ui->pushButton2->setEnabled(!busy);
                ui->disableProxyAction->setEnabled(!busy);
            });
    connect(systemProxySession, &SystemProxySession::enabledChanged, this,
            [this](bool enabled)
            {
                ui->pushButton2->setText(enabled ? "清除系统代理" : "设置系统代理");
                if (!enabled && connectionSession != nullptr && !connectionSession->isActive())
                {
                    ui->pushButton2->hide();
                }
            });
    setupTrayIcon();
    setupProfileMenu();

    setWindowIcon(QIcon(QPixmap(":/resource/icon.png").scaled(
        512, 512, Qt::KeepAspectRatio, Qt::SmoothTransformation
    )));

    ui->applicationNameLabel->setText(QApplication::applicationDisplayName());

    updateVersionInfo();


    // 文件-退出
    connect(ui->exitAction, &QAction::triggered, this, &MainWindow::gracefullyQuit);

    // 文件-设置
    connect(ui->settingAction, &QAction::triggered, this,
            [&]()
            {
                settingWindow = new SettingWindow(this, settings, currentProfileId);
                settingWindow->show();
            });

    // 文件-打开日志文件
    connect(ui->openLogAction, &QAction::triggered, this,
            [this]()
            {
                const QString logFilePath = coreLogFile->filePath();
                QFileInfo logFileInfo(logFilePath);

                if (logFileInfo.exists())
                {
                    QDesktopServices::openUrl(QUrl::fromLocalFile(logFilePath));
                }
                else
                {
                    QMessageBox::warning(this, "日志文件", "日志文件创建失败。");
                }
            });

    // 文件-清除系统代理
    connect(ui->disableProxyAction, &QAction::triggered,
            [&]()
            {
                QMessageBox messageBox(this);
                messageBox.setWindowTitle("清理系统代理");
                messageBox.setText("是否清理系统代理？");

                messageBox.addButton(QMessageBox::Yes)->setText("是");
                messageBox.addButton(QMessageBox::No)->setText("否");
                messageBox.setDefaultButton(QMessageBox::Yes);

                if (messageBox.exec() == QMessageBox::No)
                {
                    return;
                }

                connect(systemProxySession, &SystemProxySession::operationFinished, this,
                        [this](bool enabled)
                        {
                            if (!enabled)
                            {
                                qInfo().noquote() << "已清理系统代理设置";
                            }
                        },
                        Qt::SingleShotConnection);
                systemProxySession->disable();
            });

    // 文件-清理登录数据
    connect(ui->clearClientDataAction, &QAction::triggered, this,
            [&]()
            {
                QMessageBox messageBox(this);
                messageBox.setWindowTitle("清理登录缓存");
                messageBox.setText("是否清理登录缓存？");

                messageBox.addButton(QMessageBox::Yes)->setText("是");
                messageBox.addButton(QMessageBox::No)->setText("否");
                messageBox.setDefaultButton(QMessageBox::Yes);

                if (messageBox.exec() == QMessageBox::No)
                {
                    return;
                }

                ApplicationPaths::clearClientData(currentProfileId);
                qInfo().noquote() << "已清理登录缓存";
            });

    // 文件-设置授信设备
    connect(ui->trustDeviceAction, &QAction::triggered, this,
            [&]()
            {
                try
                {
                    DeviceTrust::set(this,
                        settings->value("ZJUConnect/Protocol", "easyconnect").toString(),
                        settings->value("ZJUConnect/ServerAddress").toString(),
                        settings->value("ZJUConnect/ServerPort").toInt(),
                        currentProfileId, true);
                    qInfo().noquote() << "设置授信设备成功";
                    QMessageBox::information(this, "成功", "已设置授信设备");
                }
                catch (const std::runtime_error &e)
                {
                    qWarning().noquote() << "设置授信设备失败：" + QString(e.what());
                    QMessageBox::critical(this, "错误", "设置授信设备失败：\n" + QString(e.what()));
                }
            });

    // 文件-取消授信设备
    connect(ui->untrustDeviceAction, &QAction::triggered, this,
            [&]()
            {
                try
                {
                    DeviceTrust::set(this,
                        settings->value("ZJUConnect/Protocol", "easyconnect").toString(),
                        settings->value("ZJUConnect/ServerAddress").toString(),
                        settings->value("ZJUConnect/ServerPort").toInt(),
                        currentProfileId, false);
                    qInfo().noquote() << "取消授信设备成功";
                    QMessageBox::information(this, "成功", "已取消授信设备");
                }
                catch (const std::runtime_error &e)
                {
                    qWarning().noquote() << "取消授信设备失败：" + QString(e.what());
                    QMessageBox::critical(this, "错误", "取消授信设备失败：\n" + QString(e.what()));
                }
            });

    // 帮助-检查更新
    connect(ui->checkUpdateAction, &QAction::triggered,
            updateChecker, &UpdateChecker::check);

    // 帮助-项目主页
    connect(ui->projectAction, &QAction::triggered,
            [&]()
            {
                QDesktopServices::openUrl(QUrl(
                    "https://github.com/" + ApplicationConstants::RepositoryName
                ));
            });

    // 帮助-关于本软件
    connect(ui->aboutAction, &QAction::triggered,
            [&]()
            {
                PresentationHelpers::showAboutDialog(this);
            });

    // 复制日志
    connect(ui->copyLogPushButton, &QPushButton::clicked,
            [&]()
            {
                auto logText = ui->logPlainTextEdit->toPlainText();
                QApplication::clipboard()->setText(logText);
            }
    );

    // 清空日志
    connect(ui->clearLogPushButton, &QPushButton::clicked,
            [&]()
            {
                ui->logPlainTextEdit->clear();
            }
    );

    clearLog();
    connectionUiController = new ConnectionUiController(
        this,
        ui->pushButton1,
        ui->pushButton2,
        trayConnectAction,
        connectionSession,
        systemProxySession,
        authenticationDialogs,
        applicationLogger,
        coreLogFile,
        [this]() { return settings; },
        [this]() { return currentProfileId; },
        [this](
            const QString &title,
            const QString &content,
            QSystemTrayIcon::MessageIcon icon
        )
        {
            showNotification(title, content, icon);
        },
        this
    );

    bool shouldConnect =
        settings->value("Common/ConnectAfterStart", false).toBool();
    if (qApp->arguments().contains("--connect"))
    {
        shouldConnect = true;
    }
    if (shouldConnect)
    {
        ui->pushButton1->click();
    }

    if (settings->value("Common/CheckUpdateAfterStart", true).toBool())
    {
        updateChecker->check();
    }
    else
    {
        updateChecker->markDisabled();
    }

    if (!profileService->silentStartEnabled())
    {
        show();
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (connectionSession != nullptr && connectionSession->isActive())
    {
        event->ignore();
        hide();
        showNotification("EZ4Connect", "程序已最小化到系统托盘，单击图标可恢复窗口。", QSystemTrayIcon::MessageIcon::Information);
    }
    else
    {
        event->accept();
    }
}

void MainWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::WindowStateChange)
    {
        QWindowStateChangeEvent *stateChangeEvent = static_cast<QWindowStateChangeEvent *>(event);
        if (windowState().testFlag(Qt::WindowMinimized) == true && !(stateChangeEvent->oldState() & Qt::WindowMinimized))
        {
            event->ignore();
            hide();
            showNotification("EZ4Connect", "程序已最小化到系统托盘，单击图标可恢复窗口。", QSystemTrayIcon::MessageIcon::Information);
        }
    }
    else
    {
        event->accept();
    }
}

void MainWindow::clearLog()
{
    ui->logPlainTextEdit->clear();
    ui->logPlainTextEdit->appendPlainText(
        "欢迎使用 " + QApplication::applicationDisplayName() + "\n"
        "当前版本：" + QApplication::applicationVersion() + "\n"
        "系统版本：" + QSysInfo::prettyProductName() + "\n"
        "当前配置：" + (currentProfileId.isEmpty() ? "默认" : currentProfileId) + "\n"
        "配置路径：" + settings->fileName() + "\n");
}

void MainWindow::resetZjuConnectUi()
{
    ui->pushButton1->setText("连接服务器");
    trayConnectAction->setText("连接服务器");
    ui->pushButton2->setText("设置系统代理");
    ui->pushButton2->hide();
}

void MainWindow::setupTrayIcon()
{
    // 系统托盘
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(
        QIcon(QPixmap(":/resource/icon.png").scaled(512, 512, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
    trayIcon->setVisible(true);
    trayIcon->setToolTip(QApplication::applicationName());
    connect(trayIcon, &QSystemTrayIcon::activated, this, [&](QSystemTrayIcon::ActivationReason reason) {
        switch (reason)
        {
        case QSystemTrayIcon::Context:
            trayMenu->popup(QCursor::pos());
            break;
        default:
            if (isHidden())
            {
                show();
            }
            setWindowState(Qt::WindowState::WindowActive);
            setFocus();
            break;
        }
    });
    trayIcon->show();

    trayConnectAction = new QAction("连接服务器", this);
    trayProfileMenu = new QMenu("配置选择", this);
    trayShowAction = new QAction("显示主界面", this);
    trayCloseAction = new QAction("退出 " + QApplication::applicationName(), this);
    trayMenu = new QMenu(this);
    trayMenu->addAction(trayConnectAction);
    trayMenu->addSeparator();
    trayMenu->addMenu(trayProfileMenu);
    trayMenu->addSeparator();
    trayMenu->addAction(trayShowAction);
    trayMenu->addAction(trayCloseAction);
    connect(trayConnectAction, &QAction::triggered, this, [&]() { ui->pushButton1->click(); });
    connect(trayShowAction, &QAction::triggered, this, [&]() {
        show();
        setWindowState(Qt::WindowState::WindowActive);
        setFocus();
    });
    connect(trayCloseAction, &QAction::triggered, this, &MainWindow::gracefullyQuit);
}

void MainWindow::setupProfileMenu()
{
    // 务必在 setupTrayIcon 之后调用，以确保 trayProfileMenu 已正确初始化
    newProfileAction = ui->profileMenu->addAction("新建配置");
    renameProfileAction = ui->profileMenu->addAction("重命名当前配置");
    deleteProfileAction = ui->profileMenu->addAction("删除当前配置");
    ui->profileMenu->addSeparator();

    connect(newProfileAction, &QAction::triggered, this, &MainWindow::createProfile);
    connect(renameProfileAction, &QAction::triggered, this, &MainWindow::renameCurrentProfile);
    connect(deleteProfileAction, &QAction::triggered, this, &MainWindow::deleteCurrentProfile);

    refreshProfileMenu();
}

void MainWindow::refreshProfileMenu()
{
    const QList<QAction *> actions = ui->profileMenu->actions();
    bool remove = false;
    for (QAction *action : actions)
    {
        if (action == newProfileAction || action == renameProfileAction || action == deleteProfileAction)
        {
            continue;
        }
        if (action->isSeparator())
        {
            remove = true;
            continue;
        }
        if (remove)
        {
            ui->profileMenu->removeAction(action);
            delete action;
        }
    }

    QActionGroup *switchGroup = new QActionGroup(ui->profileMenu);
    QActionGroup *traySwitchGroup = nullptr;

    if (trayProfileMenu != nullptr)
    {
        trayProfileMenu->clear();
        traySwitchGroup = new QActionGroup(trayProfileMenu);
        traySwitchGroup->setExclusive(true);
    }

    switchGroup->setExclusive(true);
    QAction *action = ui->profileMenu->addAction("默认");
    action->setCheckable(true);
    action->setChecked(currentProfileId.isEmpty());
    switchGroup->addAction(action);
    connect(action, &QAction::triggered, this, [this]()
    {
        switchProfile("");
    });
    if (trayProfileMenu != nullptr)
    {
        QAction *trayAction = trayProfileMenu->addAction("默认");
        trayAction->setCheckable(true);
        trayAction->setChecked(currentProfileId.isEmpty());
        traySwitchGroup->addAction(trayAction);
        connect(trayAction, &QAction::triggered, this, [this]()
        {
            switchProfile("");
        });
    }

    for (const QString &profileId : profileService->profiles())
    {
        QAction *action = ui->profileMenu->addAction(profileId);
        action->setCheckable(true);
        action->setChecked(
            !profileService->usesOverrideConfiguration()
            && profileId == currentProfileId
        );
        switchGroup->addAction(action);
        connect(action, &QAction::triggered, this, [this, profileId]()
        {
            switchProfile(profileId);
        });

        if (trayProfileMenu != nullptr)
        {
            QAction *trayAction = trayProfileMenu->addAction(profileId);
            trayAction->setCheckable(true);
            trayAction->setChecked(
                !profileService->usesOverrideConfiguration()
                && profileId == currentProfileId
            );
            traySwitchGroup->addAction(trayAction);
            connect(trayAction, &QAction::triggered, this, [this, profileId]()
            {
                switchProfile(profileId);
            });
        }
    }

    const bool currentProfileIsManaged =
        !currentProfileId.isEmpty()
        && !profileService->usesOverrideConfiguration();
    renameProfileAction->setEnabled(currentProfileIsManaged);
    deleteProfileAction->setEnabled(currentProfileIsManaged);
}

bool MainWindow::switchProfile(const QString &profileId)
{
    if (!profileService->usesOverrideConfiguration()
        && profileId == currentProfileId)
    {
        return true;
    }

    if (connectionSession != nullptr && connectionSession->isActive())
    {
        QMessageBox::warning(this, "切换失败", "请先断开 VPN 连接，再切换配置。");
        refreshProfileMenu();
        return false;
    }

    if (settingWindow != nullptr)
    {
        settingWindow->close();
    }

    if (!profileService->switchTo(profileId))
    {
        refreshProfileMenu();
        return false;
    }
    settings = profileService->settings();
    authenticationDialogs->setSettings(settings);
    currentProfileId = profileService->currentProfileId();

    upgradeSettings();
    updateVersionInfo();
    resetZjuConnectUi();
    clearLog();
    refreshProfileMenu();

    qInfo().noquote() << "已切换到配置：" + currentProfileId;
    return true;
}

void MainWindow::createProfile()
{
    if (connectionSession != nullptr && connectionSession->isActive())
    {
        QMessageBox::warning(this, "新建失败", "请先断开 VPN 连接，再新建配置。");
        return;
    }

    bool ok = false;
    QString name = QInputDialog::getText(this, "新建配置", "请输入配置名称：\n（仅支持字母、数字、下划线）", QLineEdit::Normal, "", &ok);
    if (!ok)
    {
        return;
    }

    if (connectionSession != nullptr && connectionSession->isActive())
    {
        QMessageBox::warning(this, "新建失败", "请先断开 VPN 连接，再新建配置。");
        return;
    }

    const QString newProfileId = profileService->createAndSwitch(name);
    if (newProfileId.isEmpty())
    {
        QMessageBox::critical(this, "创建失败", "无法创建新配置。");
        return;
    }

    settings = profileService->settings();
    currentProfileId = profileService->currentProfileId();
    authenticationDialogs->setSettings(settings);
    upgradeSettings();
    updateVersionInfo();
    resetZjuConnectUi();
    clearLog();
    refreshProfileMenu();
}

void MainWindow::renameCurrentProfile()
{
    bool ok = false;
    QString name = QInputDialog::getText(this, "重命名配置", "请输入新配置名称：\n（仅支持字母、数字、下划线）", QLineEdit::Normal, currentProfileId, &ok);
    if (!ok)
    {
        return;
    }

    const QString normalizedName = profileService->normalizeProfileId(name);
    if (normalizedName.isEmpty())
    {
        QMessageBox::warning(this, "重命名失败", "配置名称不能为空。");
        return;
    }
    if (normalizedName == currentProfileId)
    {
        return;
    }
    if (connectionSession != nullptr && connectionSession->isActive())
    {
        QMessageBox::warning(this, "重命名失败", "请先断开 VPN 连接，再重命名配置。");
        return;
    }

    if (settingWindow != nullptr)
    {
        settingWindow->close();
    }
    if (!profileService->renameCurrent(normalizedName))
    {
        QMessageBox::warning(this, "重命名失败", "目标配置已存在，或当前配置不可重命名。");
        return;
    }

    currentProfileId = profileService->currentProfileId();
    settings = profileService->settings();
    authenticationDialogs->setSettings(settings);
    updateVersionInfo();
    refreshProfileMenu();
    clearLog();
    qInfo().noquote() << "当前配置已重命名为：" + currentProfileId;
}

void MainWindow::deleteCurrentProfile()
{
    if (currentProfileId.isEmpty())
    {
        QMessageBox::warning(this, "删除失败", "默认配置不可删除。");
        return;
    }

    QMessageBox messageBox(this);
    messageBox.setWindowTitle("删除配置");
    messageBox.setText("确认删除当前配置 \"" + currentProfileId + "\" 吗？");
    messageBox.addButton(QMessageBox::Yes)->setText("是");
    messageBox.addButton(QMessageBox::No)->setText("否");
    messageBox.setDefaultButton(QMessageBox::No);
    if (messageBox.exec() != QMessageBox::Yes)
    {
        return;
    }

    const QString removedProfileId = currentProfileId;
    if (connectionSession != nullptr && connectionSession->isActive())
    {
        QMessageBox::warning(this, "删除失败", "请先断开 VPN 连接，再删除配置。");
        return;
    }
    if (settingWindow != nullptr)
    {
        settingWindow->close();
    }
    if (!profileService->removeCurrentAndSwitchToDefault())
    {
        QMessageBox::warning(this, "删除失败", "无法删除该配置文件。");
        return;
    }

    currentProfileId = profileService->currentProfileId();
    settings = profileService->settings();
    authenticationDialogs->setSettings(settings);
    upgradeSettings();
    updateVersionInfo();
    resetZjuConnectUi();
    clearLog();
    refreshProfileMenu();
    qInfo().noquote() << "已删除配置：" + removedProfileId;
}

void MainWindow::upgradeSettings()
{
    const SettingsMigrationAction action = SettingsMigrator::prepare(*settings);
    if (action == SettingsMigrationAction::MigrateAutoStart)
    {
        profileService->migrateAutoStartSetting(
            settings->value("Common/AutoStart", false).toBool()
        );
    }
    else if (action == SettingsMigrationAction::RecommendReset)
    {
        QMessageBox msgBox;
        msgBox.setText("存在配置更新");
        msgBox.setInformativeText("建议恢复默认设置，以使用优化的配置。\n\n是否恢复默认设置？");
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);

        const bool reset = msgBox.exec() == QMessageBox::Ok;
        SettingsMigrator::finish(*settings, reset);
        if (reset)
        {
            QMessageBox::information(this, "完成", "已恢复默认设置。");
        }
        return;
    }
    SettingsMigrator::finish(*settings, false);
}

void MainWindow::updateVersionInfo()
{
    const VersionInfo &versionInfo = updateChecker->versionInfo();
	ui->versionLabel->setText(
		"UI 版本：" + versionInfo.uiVersion + " 最新：" + versionInfo.uiLatest + "\n"
		"核心版本：" + versionInfo.coreVersion + " 最新：" + versionInfo.coreLatest + "\n"
        "当前配置：" + (currentProfileId.isEmpty() ? "默认" : currentProfileId)
	);
}

void MainWindow::showNotification(const QString &title, const QString &content, QSystemTrayIcon::MessageIcon icon)
{
    disconnect(trayIcon, &QSystemTrayIcon::messageClicked, nullptr, nullptr);
    trayIcon->showMessage(
        title,
        content,
        icon,
        10000
    );

    connect(trayIcon, &QSystemTrayIcon::messageClicked, this, [&]()
    {
        disconnect(trayIcon, &QSystemTrayIcon::messageClicked, nullptr, nullptr);

        show();
        setWindowState(Qt::WindowState::WindowActive);
    });
}

void MainWindow::cleanUpWhenQuit()
{
    // 保存配置
    if (settings->value("Common/ConfigVersion", 0).toInt() <=
        ApplicationConstants::ConfigVersion)
    {
        settings->setValue(
            "Common/ConfigVersion",
            ApplicationConstants::ConfigVersion
        );
    }
    settings->sync();

    // 清除系统代理
    coordinator->prepareForShutdown();
}

void MainWindow::gracefullyQuit()
{
    if (connectionSession != nullptr && connectionSession->isActive())
    {
        connect(connectionSession, &ConnectionSession::finished, qApp,
                [](ZJU_ERROR) { QApplication::quit(); });
        ui->pushButton1->click();
    }
    else
    {
        qApp->quit();
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}
