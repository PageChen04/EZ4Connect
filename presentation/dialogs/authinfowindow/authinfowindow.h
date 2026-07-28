#pragma once

#include "ui_authinfowindow.h"
#include <QDialog>
#include <QProcess>

class AuthInfoWindow : public QDialog
{
	Q_OBJECT

public:
	AuthInfoWindow(QWidget *parent = nullptr);

    ~AuthInfoWindow() override;

    void fetchAuthInfo(const QString& serverAddress, int port);

signals:
	void finishAuthInfo(const QString& authType, const QString& loginDomain, const QString& loginUrl);

private:
	Ui::AuthInfoWindow *ui;
    QProcess *proc_ = nullptr;
    QByteArray stdoutBuf_;
    QByteArray stderrBuf_;
};
