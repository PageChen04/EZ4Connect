#include "authinfowindow.h"

#include "infrastructure/coreprocess/consoleoutputdecoder.h"
#include "infrastructure/coreprocess/coreexecutable.h"

#include <QMetaEnum>

#include <QDebug>
#include <QDialogButtonBox>
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

    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    connect(
        ui->authInfoListWidget,
        &QListWidget::currentItemChanged,
        this,
        [this](QListWidgetItem *current)
        {
            ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(
                current != nullptr
            );
        }
    );

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
                QString output = ConsoleOutputDecoder::decode(stdoutBuf_);
                qInfo().noquote() << "可用认证方式：\n" + output;
                QJsonParseError jsonError;
                QJsonDocument doc = QJsonDocument::fromJson(output.toUtf8(), &jsonError);
                if (jsonError.error != QJsonParseError::NoError)
                {
                    qWarning().noquote() << "解析可用认证方式失败：" + jsonError.errorString();
                    ui->label->setText("获取认证方式失败，请检查服务器信息后重试。");
                    return;
                }
                if (!doc.isArray())
                {
                    qWarning().noquote() << "解析可用认证方式失败：可用认证方式不是列表";
                    ui->label->setText("服务器没有返回有效的认证方式列表。");
                    return;
                }
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
                ui->label->setText(
                    arr.isEmpty()
                        ? "服务器没有返回可用的认证方式。"
                        : "请选择可用的认证方式："
                );
            });
    connect(proc_, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
        qWarning().noquote()
            << QString("获取可用认证方式失败：")
                   + QMetaEnum::fromType<QProcess::ProcessError>().valueToKey(error);
        ui->label->setText("获取认证方式失败，请检查核心程序和服务器信息。");
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
    ui->authInfoListWidget->clear();
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    ui->label->setText("正在获取认证方式，请稍候...");
    proc_->start(CoreExecutable::path(),
                 {"-protocol", "atrust", "-server", serverAddress, "-port", QString::number(port), "-auth-info"});
    qInfo().noquote() << "正在获取可用认证的方式...";
}
