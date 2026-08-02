#include <QApplication>
#include <QDebug>
#include <QLineEdit>
#include <QSettings>
#include <QStackedWidget>
#include <QTemporaryDir>

#include "presentation/dialogs/configurationguidedialog/configurationguidedialog.h"

namespace
{
QLineEdit *lineEdit(ConfigurationGuideDialog &dialog, const char *name)
{
    QLineEdit *field = dialog.findChild<QLineEdit *>(name);
    if (field == nullptr)
    {
        qCritical() << "Missing guide field" << name;
    }
    return field;
}

QStackedWidget *credentialPages(ConfigurationGuideDialog &dialog)
{
    QStackedWidget *pages = dialog.findChild<QStackedWidget *>(
        "credentialPages"
    );
    if (pages == nullptr)
    {
        qCritical() << "Missing credential pages";
    }
    return pages;
}

bool savesPasswordCredentials(const QString &settingsPath)
{
    QSettings settings(settingsPath, QSettings::IniFormat);
    settings.setValue("ZJUConnect/Protocol", "easyconnect");
    settings.setValue("ZJUConnect/EasyConnectAuthType", "password");
    settings.setValue("Credential/Username", "old-user");
    settings.setValue(
        "Credential/Password",
        QString(QStringLiteral("old-password").toUtf8().toBase64())
    );
    settings.setValue("Credential/TOTPSecret", "old-totp");

    ConfigurationGuideDialog dialog(nullptr, &settings);
    QStackedWidget *pages = credentialPages(dialog);
    QLineEdit *username = lineEdit(dialog, "guideUsernameLineEdit");
    QLineEdit *password = lineEdit(dialog, "guidePasswordLineEdit");
    QLineEdit *totp = lineEdit(dialog, "guideTotpSecretLineEdit");
    if (pages == nullptr || username == nullptr || password == nullptr
        || totp == nullptr)
    {
        return false;
    }
    if (pages->currentIndex() != 0 || username->text() != "old-user"
        || password->text() != "old-password" || totp->text() != "old-totp")
    {
        qCritical() << "Password credentials were not loaded into the guide";
        return false;
    }

    username->setText("alice");
    password->setText("secret");
    totp->setText("totp-key");
    dialog.applyTo(settings);

    const QString savedPassword = QByteArray::fromBase64(
        settings.value("Credential/Password").toByteArray()
    );
    const bool passed = settings.value("Credential/Username") == "alice"
        && savedPassword == "secret"
        && settings.value("Credential/TOTPSecret") == "totp-key";
    if (!passed)
    {
        qCritical() << "Password credentials were not saved by the guide";
    }
    return passed;
}

bool savesSmsCredentials(const QString &settingsPath)
{
    QSettings settings(settingsPath, QSettings::IniFormat);
    settings.setValue("ZJUConnect/Protocol", "atrust");
    settings.setValue("ZJUConnect/AuthType", "smsCheckCode");
    settings.setValue("ZJUConnect/PhoneCountryCode", "86");
    settings.setValue("ZJUConnect/PhoneNumber", "123456");

    ConfigurationGuideDialog dialog(nullptr, &settings);
    QStackedWidget *pages = credentialPages(dialog);
    QLineEdit *countryCode = lineEdit(dialog, "guideCountryCodeLineEdit");
    QLineEdit *phoneNumber = lineEdit(dialog, "guidePhoneNumberLineEdit");
    if (pages == nullptr || countryCode == nullptr || phoneNumber == nullptr)
    {
        return false;
    }
    if (pages->currentIndex() != 1 || countryCode->text() != "86"
        || phoneNumber->text() != "123456")
    {
        qCritical() << "SMS credentials were not loaded into the guide";
        return false;
    }

    countryCode->setText("852");
    phoneNumber->setText("98765432");
    dialog.applyTo(settings);

    const bool passed = settings.value("ZJUConnect/PhoneCountryCode") == "852"
        && settings.value("ZJUConnect/PhoneNumber") == "98765432";
    if (!passed)
    {
        qCritical() << "SMS credentials were not saved by the guide";
    }
    return passed;
}

bool savesCertificateCredentials(const QString &settingsPath)
{
    QSettings settings(settingsPath, QSettings::IniFormat);
    settings.setValue("ZJUConnect/Protocol", "easyconnect");
    settings.setValue("ZJUConnect/EasyConnectAuthType", "certificate");
    settings.setValue("Credential/CertFile", "/tmp/old.p12");
    settings.setValue(
        "Credential/CertPassword",
        QString(QStringLiteral("old-cert-password").toUtf8().toBase64())
    );
    settings.setValue("Credential/TOTPSecret", "old-cert-totp");

    ConfigurationGuideDialog dialog(nullptr, &settings);
    QStackedWidget *pages = credentialPages(dialog);
    QLineEdit *certificateFile = lineEdit(
        dialog,
        "guideCertificateFileLineEdit"
    );
    QLineEdit *certificatePassword = lineEdit(
        dialog,
        "guideCertificatePasswordLineEdit"
    );
    QLineEdit *certificateTotp = lineEdit(
        dialog,
        "guideCertificateTotpSecretLineEdit"
    );
    if (pages == nullptr || certificateFile == nullptr
        || certificatePassword == nullptr || certificateTotp == nullptr)
    {
        return false;
    }
    if (pages->currentIndex() != 2 || certificateFile->text() != "/tmp/old.p12"
        || certificatePassword->text() != "old-cert-password"
        || certificateTotp->text() != "old-cert-totp")
    {
        qCritical() << "Certificate credentials were not loaded into the guide";
        return false;
    }

    certificateFile->setText("/tmp/client.pfx");
    certificatePassword->setText("cert-secret");
    certificateTotp->setText("new-cert-totp");
    dialog.applyTo(settings);

    const QString savedPassword = QByteArray::fromBase64(
        settings.value("Credential/CertPassword").toByteArray()
    );
    const bool passed =
        settings.value("Credential/CertFile") == "/tmp/client.pfx"
        && savedPassword == "cert-secret"
        && settings.value("Credential/TOTPSecret") == "new-cert-totp";
    if (!passed)
    {
        qCritical() << "Certificate credentials were not saved by the guide";
    }
    return passed;
}

bool showsSsoWithoutCredentialFields(const QString &settingsPath)
{
    QSettings settings(settingsPath, QSettings::IniFormat);
    settings.setValue("ZJUConnect/Protocol", "atrust");
    settings.setValue("ZJUConnect/AuthType", "cas");

    ConfigurationGuideDialog dialog(nullptr, &settings);
    QStackedWidget *pages = credentialPages(dialog);
    const bool passed = pages != nullptr && pages->currentIndex() == 3;
    if (!passed)
    {
        qCritical() << "CAS did not select the credential-free guide page";
    }
    return passed;
}
}

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);
    QTemporaryDir directory;
    if (!directory.isValid())
    {
        qCritical() << "Unable to create temporary directory";
        return 1;
    }

    const bool passed =
        savesPasswordCredentials(directory.filePath("password.ini"))
        && savesSmsCredentials(directory.filePath("sms.ini"))
        && savesCertificateCredentials(directory.filePath("certificate.ini"))
        && showsSsoWithoutCredentialFields(directory.filePath("sso.ini"));
    return passed ? 0 : 1;
}
