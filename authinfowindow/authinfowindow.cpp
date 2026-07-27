#include "authinfowindow.h"

#include "utils/utils.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QPushButton>
#include <QKeyEvent>
#include <QProcess>

AuthInfoWindow::AuthInfoWindow(QWidget *parent)
    : QDialog(parent),
    ui(new Ui::AuthInfoWindow)
{
    ui->setupUi(this);

    setWindowModality(Qt::WindowModal);
    setAttribute(Qt::WA_DeleteOnClose);

    connect(this, &QDialog::accepted, [&]() {
        QListWidgetItem *selectedItem = ui->authInfoListWidget->currentItem();
        if (!selectedItem)
            return;
        emit finishAuthInfo(selectedItem->data(Qt::UserRole).toString(),
                            selectedItem->data(Qt::UserRole + 1).toString(),
                            selectedItem->data(Qt::UserRole + 2).toString());
    });

    proc_ = new QProcess(this);
    connect(proc_, &QProcess::readyReadStandardOutput, this,
            [this]() { stdoutBuf_ += proc_->readAllStandardOutput(); });
    connect(proc_, &QProcess::readyReadStandardError, this,
            [this]() { stdoutBuf_ += proc_->readAllStandardError(); });
    connect(proc_, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
            [this](int exitCode, QProcess::ExitStatus exitStatus) {
                QString output = Utils::consoleOutputToQString(stdoutBuf_);
                qInfo().noquote() << "可用认证方式：\n" + output;
                QJsonParseError jsonError;
                QJsonDocument doc = QJsonDocument::fromJson(output.toUtf8(), &jsonError);
                if (jsonError.error != QJsonParseError::NoError)
                    qWarning().noquote() << "解析可用认证方式失败：" + jsonError.errorString();
                if (!doc.isArray())
                    qWarning().noquote() << "解析可用认证方式失败：可用认证方式不是列表";
                QJsonArray arr = doc.array();
                for (QJsonValueRef v : arr) {
                    QJsonObject obj = v.toObject();
                    QString authName = obj.value("authName").toString();
                    QString authType = obj.value("authType").toString();
                    QString loginDomain = obj.value("loginDomain").toString();
                    QString loginUrl = obj.value("loginUrl").toString();
                    QListWidgetItem *item =
                        new QListWidgetItem(QString("%1 - %2 - %3 - %4").arg(authName, authType, loginDomain, loginUrl.isEmpty()? "无" : loginUrl));
                    item->setData(Qt::UserRole, authType);
                    item->setData(Qt::UserRole + 1, loginDomain);
                    item->setData(Qt::UserRole + 2, loginUrl);
                    ui->authInfoListWidget->addItem(item);
                }
            });
    connect(proc_, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
        qWarning().noquote()
            << QString("获取可用认证方式失败：")
                   + QMetaEnum::fromType<QProcess::ProcessError>().valueToKey(error);
    });
}

AuthInfoWindow::~AuthInfoWindow()
{
    delete ui;
}

void AuthInfoWindow::fetchAuthInfo(const QString& serverAddress, int port)
{
    stdoutBuf_.clear();
    stderrBuf_.clear();
    proc_->start(Utils::getCorePath(),
                 {"-protocol", "atrust", "-server", serverAddress, "-port", QString::number(port), "-auth-info"});
    qInfo().noquote() << "正在获取可用认证的方式...";
}
