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
        const QSettings *settings,
        bool requestProfileName = false
    );

    QString profileName() const;

    void applyTo(QSettings &settings) const;

private slots:
    void goBack();

    void goNext();

    void updateProtocolPage();

    void fetchAuthenticationMethods();

private:
    QWidget *createServerPage(bool requestProfileName);

    QWidget *createProtocolPage();

    QWidget *createAuthenticationPage();

    bool validateCurrentPage();

    void updateNavigation();

    void selectAuthenticationMethod(
        const QString &authType,
        const QString &loginDomain,
        const QString &loginUrl
    );

    QString authenticationMethodName(const QString &authType) const;

    const QSettings *sourceSettings;
    bool requestProfileName;

    QLabel *stepLabel;
    QLabel *titleLabel;
    QLabel *descriptionLabel;
    QStackedWidget *pages;
    QPushButton *backButton;
    QPushButton *nextButton;

    QLabel *profileNameLabel;
    QLineEdit *profileNameLineEdit;
    QLineEdit *serverAddressLineEdit;
    QSpinBox *serverPortSpinBox;

    QRadioButton *atrustRadioButton;
    QRadioButton *easyconnectRadioButton;

    QStackedWidget *authenticationPages;
    QLabel *selectedAuthenticationLabel;
    QPushButton *fetchAuthenticationButton;
    QRadioButton *passwordAuthenticationRadioButton;
    QRadioButton *certificateAuthenticationRadioButton;

    QString selectedAuthType;
    QString selectedLoginDomain;
    QString selectedLoginUrl;
};

#endif // CONFIGURATIONGUIDEDIALOG_H
