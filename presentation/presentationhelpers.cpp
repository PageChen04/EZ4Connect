#include "presentationhelpers.h"

#include <QApplication>
#include <QMessageBox>
#include <QPixmap>
#include <QSizePolicy>
#include <QWidget>

#include "application/applicationconstants.h"

void PresentationHelpers::retainSizeWhenHidden(QWidget *widget)
{
    QSizePolicy policy = widget->sizePolicy();
    policy.setRetainSizeWhenHidden(true);
    widget->setSizePolicy(policy);
}

void PresentationHelpers::showAboutDialog(QWidget *parent)
{
    QMessageBox messageBox(parent);
    messageBox.setWindowTitle("关于");
    messageBox.setTextFormat(Qt::RichText);
    const QString repository = ApplicationConstants::RepositoryName;
    messageBox.setText(
        QApplication::applicationDisplayName() + " " + QApplication::applicationVersion() +
        "<br>改进的 ZJU-Connect 图形界面" +
        "<br>作者：<a href='https://github.com/chenx-dust'>Chenx Dust</a>" +
        "<br>项目主页：<a href='https://github.com/" + repository +
        "'>https://github.com/" + repository + "</a>" +
        "<br><br>致谢：" +
        "<br><br>ZJU-Connect-for-Windows" +
        "<br>基于 Qt 编写的 ZJU 网络客户端" +
        "<br>作者：<a href='https://myth.cx'>Myth</a>" +
        "<br>项目主页：<a href='https://github.com/Mythologyli/ZJU-Connect-for-Windows'>"
        "https://github.com/Mythologyli/ZJU-Connect-for-Windows</a>" +
        "<br><br>zju-connect" +
        "<br>ZJU RVPN 客户端的 Go 语言实现" +
        "<br>作者：<a href='https://myth.cx'>Myth</a>" +
        "<br>项目主页：<a href='https://github.com/Mythologyli/zju-connect'>"
        "https://github.com/Mythologyli/zju-connect</a>" +
        "<br><br>EasierConnect" +
        "<br>EasyConnect 客户端的开源实现" +
        "<br>作者：<a href='https://github.com/lyc8503'>lyc8503</a>" +
        "<br>项目主页：<a href='https://github.com/lyc8503/EasierConnect'>"
        "https://github.com/lyc8503/EasierConnect</a>"
    );
    messageBox.setIconPixmap(QPixmap(":/resource/icon.png").scaled(
        100,
        100,
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation
    ));
    messageBox.exec();
}

bool PresentationHelpers::confirmCredentials(
    const QString &username,
    const QString &password
)
{
    if (username.isEmpty() || password.isEmpty())
    {
        return QMessageBox::warning(
            nullptr,
            "警告",
            "账号或密码为空！\n\n是否继续？",
            QMessageBox::Ok,
            QMessageBox::Cancel
        ) == QMessageBox::Ok;
    }

    const auto containsNonAscii = [](const QString &value)
    {
        for (const QChar character : value)
        {
            if (character.unicode() > 127)
            {
                return true;
            }
        }
        return false;
    };
    if (containsNonAscii(username) || containsNonAscii(password))
    {
        return QMessageBox::warning(
            nullptr,
            "警告",
            "账号或密码存在非 ASCII 字符！\n建议检查输入法设置。\n\n是否继续？",
            QMessageBox::Ok,
            QMessageBox::Cancel
        ) == QMessageBox::Ok;
    }
    return true;
}
