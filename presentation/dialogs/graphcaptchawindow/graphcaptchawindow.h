#pragma once

#include "ui_graphcaptchawindow.h"
#include <QDialog>

class MainWindow;

class GraphCaptchaWindow : public QDialog
{
    Q_OBJECT

public:
    GraphCaptchaWindow(QWidget *parent = nullptr);

    ~GraphCaptchaWindow() override;

    void setGraph(const QString &graphFile);

signals:
    void finishCaptcha(const QByteArray &captcha);

private:
    Ui::GraphCaptchaWindow *ui;
    MainWindow *mainWindow = nullptr;
};
