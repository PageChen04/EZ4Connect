#include <QApplication>
#include <QDebug>
#include <QDialogButtonBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLineEdit>
#include <QPixmap>
#include <QPushButton>
#include <QTemporaryDir>

#include "presentation/dialogs/graphcaptchawindow/graphcaptchawindow.h"

namespace
{
QString createCaptchaImage(QTemporaryDir &directory)
{
    const QString path = directory.filePath("captcha.png");
    QPixmap image(120, 40);
    image.fill(Qt::white);
    return image.save(path) ? path : QString{};
}

bool submitsTextCaptcha()
{
    QTemporaryDir directory;
    const QString imagePath = createCaptchaImage(directory);
    GraphCaptchaWindow window;
    window.setGraph(imagePath, true);

    auto *codeEdit = window.findChild<QLineEdit *>("codeEdit");
    auto *buttonBox = window.findChild<QDialogButtonBox *>("buttonBox");
    QByteArray submitted;
    QObject::connect(&window, &GraphCaptchaWindow::finishCaptcha,
                     [&](const QByteArray &captcha) { submitted = captcha; });

    if (imagePath.isEmpty() || codeEdit == nullptr || buttonBox == nullptr
        || codeEdit->isHidden())
    {
        qCritical() << "text captcha controls were not configured";
        return false;
    }

    codeEdit->setText(" A1b2 ");
    buttonBox->button(QDialogButtonBox::Ok)->click();
    if (submitted != "A1b2")
    {
        qCritical() << "text captcha submission failed:" << submitted;
        return false;
    }
    return true;
}

bool cancelsWithoutSubmittingPlaceholder()
{
    QTemporaryDir directory;
    GraphCaptchaWindow window;
    window.setGraph(createCaptchaImage(directory), true);

    int submissions = 0;
    int cancellations = 0;
    QObject::connect(&window, &GraphCaptchaWindow::finishCaptcha,
                     [&](const QByteArray &) { ++submissions; });
    QObject::connect(&window, &GraphCaptchaWindow::cancelled,
                     [&]() { ++cancellations; });

    window.findChild<QDialogButtonBox *>("buttonBox")
        ->button(QDialogButtonBox::Cancel)->click();
    if (submissions != 0 || cancellations != 1)
    {
        qCritical() << "captcha cancellation emitted an invalid response";
        return false;
    }
    return true;
}

bool preservesCoordinateCaptchaResponse()
{
    QTemporaryDir directory;
    GraphCaptchaWindow window;
    window.setGraph(createCaptchaImage(directory), false);

    QByteArray submitted;
    QObject::connect(&window, &GraphCaptchaWindow::finishCaptcha,
                     [&](const QByteArray &captcha) { submitted = captcha; });
    window.findChild<QDialogButtonBox *>("buttonBox")
        ->button(QDialogButtonBox::Ok)->click();

    const QJsonObject response = QJsonDocument::fromJson(submitted).object();
    if (response.value("width").toInt() != 120
        || response.value("height").toInt() != 40
        || !response.value("coordinates").isArray())
    {
        qCritical() << "coordinate captcha response changed:" << submitted;
        return false;
    }
    return true;
}
}

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);
    return submitsTextCaptcha()
        && cancelsWithoutSubmittingPlaceholder()
        && preservesCoordinateCaptchaResponse()
        ? 0
        : 1;
}
