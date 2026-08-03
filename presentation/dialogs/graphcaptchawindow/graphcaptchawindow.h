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

    void setGraph(const QString &graphFile, bool textInputMode);

signals:
    void finishCaptcha(const QByteArray &captcha);
    void cancelled();

public slots:
    void accept() override;
    void reject() override;

private:
    Ui::GraphCaptchaWindow *ui;
    MainWindow *mainWindow = nullptr;
    bool textInputMode = false;
};
