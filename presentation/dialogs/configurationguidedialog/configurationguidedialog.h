#ifndef CONFIGURATIONGUIDEDIALOG_H
#define CONFIGURATIONGUIDEDIALOG_H

#include <QDialog>
#include <QString>

class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;
class QSettings;
class QSpinBox;
class QStackedWidget;
class QWidget;

class ConfigurationGuideDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigurationGuideDialog(
        QWidget *parent,
        const QSettings *settings
    );

    void applyTo(QSettings &settings) const;

private slots:
    void goBack();

    void goNext();

    void updateProtocolPage();

    void fetchAuthenticationMethods();

private:
    QWidget *createServerPage();

    QWidget *createProtocolPage();

    QWidget *createAuthenticationPage();

    QWidget *createCredentialsPage();

    bool validateCurrentPage();

    void updateNavigation();

    void updateCredentialsPage();

    void browseCertificateFile();

    void selectAuthenticationMethod(
        const QString &authType,
        const QString &loginDomain,
        const QString &loginUrl
    );

    QString authenticationMethodName(const QString &authType) const;

    const QSettings *sourceSettings;

    QLabel *stepLabel;
    QLabel *titleLabel;
    QLabel *descriptionLabel;
    QStackedWidget *pages;
    QPushButton *backButton;
    QPushButton *nextButton;

    QLineEdit *serverAddressLineEdit;
    QSpinBox *serverPortSpinBox;

    QRadioButton *atrustRadioButton;
    QRadioButton *easyconnectRadioButton;

    QStackedWidget *authenticationPages;
    QLabel *selectedAuthenticationLabel;
    QPushButton *fetchAuthenticationButton;
    QRadioButton *passwordAuthenticationRadioButton;
    QRadioButton *certificateAuthenticationRadioButton;

    QStackedWidget *credentialPages = nullptr;
    QLineEdit *usernameLineEdit;
    QLineEdit *passwordLineEdit;
    QLineEdit *totpSecretLineEdit;
    QLineEdit *countryCodeLineEdit;
    QLineEdit *phoneNumberLineEdit;
    QLineEdit *certificateFileLineEdit;
    QLineEdit *certificatePasswordLineEdit;
    QLineEdit *certificateTotpSecretLineEdit;

    QString selectedAuthType;
    QString selectedLoginDomain;
    QString selectedLoginUrl;
};

#endif // CONFIGURATIONGUIDEDIALOG_H
