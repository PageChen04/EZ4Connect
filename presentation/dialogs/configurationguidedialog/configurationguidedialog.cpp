#include "configurationguidedialog.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QFont>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QRegularExpression>
#include <QSettings>
#include <QSpinBox>
#include <QStackedWidget>
#include <QStringList>
#include <QVBoxLayout>

#include "presentation/dialogs/authinfowindow/authinfowindow.h"

namespace
{
QString normalizedAuthType(const QString &authType)
{
    QString normalized = authType;
    if (normalized.startsWith("auth/"))
    {
        normalized.remove(0, 5);
    }
    return normalized;
}
}

ConfigurationGuideDialog::ConfigurationGuideDialog(
    QWidget *parent,
    const QSettings *settings,
    bool requestProfileName
)
    : QDialog(parent),
      sourceSettings(settings),
      requestProfileName(requestProfileName)
{
    setWindowTitle(requestProfileName ? "新建配置" : "配置引导");
    setWindowModality(Qt::WindowModal);
    setMinimumSize(520, 340);

    auto *layout = new QVBoxLayout(this);

    stepLabel = new QLabel(this);
    titleLabel = new QLabel(this);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(titleFont.pointSize() + 4);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);

    descriptionLabel = new QLabel(this);
    descriptionLabel->setWordWrap(true);

    layout->addWidget(stepLabel);
    layout->addWidget(titleLabel);
    layout->addWidget(descriptionLabel);

    pages = new QStackedWidget(this);
    pages->addWidget(createProtocolPage());
    pages->addWidget(createServerPage(requestProfileName));
    pages->addWidget(createAuthenticationPage());
    layout->addWidget(pages, 1);

    auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel, this);
    backButton = buttonBox->addButton("上一步", QDialogButtonBox::ActionRole);
    nextButton = buttonBox->addButton("下一步", QDialogButtonBox::AcceptRole);
    layout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(backButton, &QPushButton::clicked, this, &ConfigurationGuideDialog::goBack);
    connect(nextButton, &QPushButton::clicked, this, &ConfigurationGuideDialog::goNext);
    connect(
        pages,
        &QStackedWidget::currentChanged,
        this,
        &ConfigurationGuideDialog::updateNavigation
    );

    const QString protocol = sourceSettings->value(
        "ZJUConnect/Protocol",
        "atrust"
    ).toString();
    if (protocol == "easyconnect")
    {
        easyconnectRadioButton->setChecked(true);
    }
    else
    {
        atrustRadioButton->setChecked(true);
    }

    selectAuthenticationMethod(
        sourceSettings->value("ZJUConnect/AuthType", "psw").toString(),
        sourceSettings->value("ZJUConnect/LoginDomain").toString(),
        sourceSettings->value("ZJUConnect/LoginURL").toString()
    );

    const QString easyconnectAuthType = sourceSettings->value(
        "ZJUConnect/EasyConnectAuthType",
        sourceSettings->value("Credential/CertFile").toString().isEmpty()
            ? "password"
            : "certificate"
    ).toString();
    certificateAuthenticationRadioButton->setChecked(
        easyconnectAuthType == "certificate"
    );
    passwordAuthenticationRadioButton->setChecked(
        easyconnectAuthType != "certificate"
    );

    updateProtocolPage();
    updateNavigation();
}

QString ConfigurationGuideDialog::profileName() const
{
    return profileNameLineEdit->text().trimmed();
}

void ConfigurationGuideDialog::applyTo(QSettings &settings) const
{
    settings.setValue(
        "ZJUConnect/ServerAddress",
        serverAddressLineEdit->text().trimmed()
    );
    settings.setValue("ZJUConnect/ServerPort", serverPortSpinBox->value());

    if (atrustRadioButton->isChecked())
    {
        settings.setValue("ZJUConnect/Protocol", "atrust");
        settings.setValue("ZJUConnect/AuthType", selectedAuthType);
        settings.setValue("ZJUConnect/LoginDomain", selectedLoginDomain);
        settings.setValue("ZJUConnect/LoginURL", selectedLoginUrl);
    }
    else
    {
        settings.setValue("ZJUConnect/Protocol", "easyconnect");
        settings.setValue(
            "ZJUConnect/EasyConnectAuthType",
            certificateAuthenticationRadioButton->isChecked()
                ? "certificate"
                : "password"
        );
    }
    settings.sync();
}

QWidget *ConfigurationGuideDialog::createServerPage(bool showProfileName)
{
    auto *page = new QWidget(this);
    auto *pageLayout = new QVBoxLayout(page);
    auto *formLayout = new QFormLayout();
    formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);

    profileNameLabel = new QLabel("配置名称", page);
    profileNameLineEdit = new QLineEdit(page);
    profileNameLineEdit->setPlaceholderText("例如：school_vpn");
    profileNameLabel->setVisible(showProfileName);
    profileNameLineEdit->setVisible(showProfileName);
    formLayout->addRow(profileNameLabel, profileNameLineEdit);

    serverAddressLineEdit = new QLineEdit(page);
    serverAddressLineEdit->setPlaceholderText("例如：vpn.example.edu.cn");
    serverAddressLineEdit->setText(
        sourceSettings->value("ZJUConnect/ServerAddress").toString()
    );
    formLayout->addRow("服务器地址", serverAddressLineEdit);

    serverPortSpinBox = new QSpinBox(page);
    serverPortSpinBox->setRange(1, 65535);
    serverPortSpinBox->setValue(
        sourceSettings->value("ZJUConnect/ServerPort", 443).toInt()
    );
    formLayout->addRow("服务器端口", serverPortSpinBox);

    pageLayout->addLayout(formLayout);
    pageLayout->addStretch();
    return page;
}

QWidget *ConfigurationGuideDialog::createProtocolPage()
{
    auto *page = new QWidget(this);
    auto *pageLayout = new QVBoxLayout(page);

    auto *protocolGroup = new QGroupBox("服务器协议", page);
    auto *protocolLayout = new QVBoxLayout(protocolGroup);

    atrustRadioButton = new QRadioButton("aTrust", protocolGroup);
    auto *atrustDescription = new QLabel(
        "适用于新版深信服 aTrust 服务器，可从服务器获取认证方式。",
        protocolGroup
    );
    atrustDescription->setWordWrap(true);

    easyconnectRadioButton = new QRadioButton("EasyConnect", protocolGroup);
    auto *easyconnectDescription = new QLabel(
        "适用于传统 EasyConnect 服务器。",
        protocolGroup
    );
    easyconnectDescription->setWordWrap(true);

    protocolLayout->addWidget(atrustRadioButton);
    protocolLayout->addWidget(atrustDescription);
    protocolLayout->addSpacing(12);
    protocolLayout->addWidget(easyconnectRadioButton);
    protocolLayout->addWidget(easyconnectDescription);

    pageLayout->addWidget(protocolGroup);
    pageLayout->addStretch();

    connect(
        atrustRadioButton,
        &QRadioButton::toggled,
        this,
        &ConfigurationGuideDialog::updateProtocolPage
    );
    return page;
}

QWidget *ConfigurationGuideDialog::createAuthenticationPage()
{
    auto *page = new QWidget(this);
    auto *pageLayout = new QVBoxLayout(page);

    authenticationPages = new QStackedWidget(page);

    auto *atrustPage = new QWidget(authenticationPages);
    auto *atrustLayout = new QVBoxLayout(atrustPage);
    auto *atrustInfo = new QLabel(
        "从服务器读取可用的认证方式，然后选择与你的账号相符的一项。",
        atrustPage
    );
    atrustInfo->setWordWrap(true);
    selectedAuthenticationLabel = new QLabel(atrustPage);
    selectedAuthenticationLabel->setWordWrap(true);
    fetchAuthenticationButton = new QPushButton("获取认证方式", atrustPage);
    atrustLayout->addWidget(atrustInfo);
    atrustLayout->addWidget(selectedAuthenticationLabel);
    atrustLayout->addWidget(fetchAuthenticationButton, 0, Qt::AlignLeft);
    atrustLayout->addStretch();
    authenticationPages->addWidget(atrustPage);

    auto *easyconnectPage = new QWidget(authenticationPages);
    auto *easyconnectLayout = new QVBoxLayout(easyconnectPage);
    passwordAuthenticationRadioButton = new QRadioButton(
        "用户名和密码",
        easyconnectPage
    );
    certificateAuthenticationRadioButton = new QRadioButton(
        "证书",
        easyconnectPage
    );
    auto *certificateHint = new QLabel(
        "选择证书认证后，可在“文件 → 设置 → 认证”中配置证书文件。",
        easyconnectPage
    );
    certificateHint->setWordWrap(true);
    easyconnectLayout->addWidget(passwordAuthenticationRadioButton);
    easyconnectLayout->addWidget(certificateAuthenticationRadioButton);
    easyconnectLayout->addWidget(certificateHint);
    easyconnectLayout->addStretch();
    authenticationPages->addWidget(easyconnectPage);

    pageLayout->addWidget(authenticationPages);

    connect(
        fetchAuthenticationButton,
        &QPushButton::clicked,
        this,
        &ConfigurationGuideDialog::fetchAuthenticationMethods
    );
    return page;
}

void ConfigurationGuideDialog::goBack()
{
    if (pages->currentIndex() > 0)
    {
        pages->setCurrentIndex(pages->currentIndex() - 1);
    }
}

void ConfigurationGuideDialog::goNext()
{
    if (!validateCurrentPage())
    {
        return;
    }

    if (pages->currentIndex() == pages->count() - 1)
    {
        accept();
        return;
    }
    pages->setCurrentIndex(pages->currentIndex() + 1);
}

void ConfigurationGuideDialog::updateProtocolPage()
{
    if (authenticationPages != nullptr)
    {
        authenticationPages->setCurrentIndex(
            atrustRadioButton->isChecked() ? 0 : 1
        );
    }
}

void ConfigurationGuideDialog::fetchAuthenticationMethods()
{
    auto *authInfoWindow = new AuthInfoWindow(this);
    connect(
        authInfoWindow,
        &AuthInfoWindow::finishAuthInfo,
        this,
        &ConfigurationGuideDialog::selectAuthenticationMethod
    );
    authInfoWindow->fetchAuthInfo(
        serverAddressLineEdit->text().trimmed(),
        serverPortSpinBox->value()
    );
    authInfoWindow->exec();
}

bool ConfigurationGuideDialog::validateCurrentPage()
{
    if (pages->currentIndex() == 1)
    {
        if (requestProfileName)
        {
            const QString name = profileName();
            const QRegularExpression validName("^[a-zA-Z0-9_-]+$");
            if (!validName.match(name).hasMatch())
            {
                QMessageBox::warning(
                    this,
                    "配置名称无效",
                    "配置名称仅支持字母、数字、下划线和连字符。"
                );
                return false;
            }
        }

        if (serverAddressLineEdit->text().trimmed().isEmpty())
        {
            QMessageBox::warning(this, "服务器地址无效", "服务器地址不能为空。");
            return false;
        }
    }
    else if (pages->currentIndex() == 2)
    {
        if (atrustRadioButton->isChecked() && selectedAuthType.isEmpty())
        {
            QMessageBox::warning(
                this,
                "尚未选择认证方式",
                "请先获取并选择服务器支持的认证方式。"
            );
            return false;
        }
    }
    return true;
}

void ConfigurationGuideDialog::updateNavigation()
{
    static const QStringList titles{
        "选择协议",
        "配置服务器",
        "选择认证方式"
    };
    static const QStringList descriptions{
        "选择服务器实际使用的接入协议。",
        "填写 VPN 服务器提供方给出的地址和端口。",
        "只需确定认证方式；账号、密码和验证码将在连接时按需询问。"
    };

    const int pageIndex = pages->currentIndex();
    stepLabel->setText(
        QString("步骤 %1 / %2").arg(pageIndex + 1).arg(pages->count())
    );
    titleLabel->setText(titles.value(pageIndex));
    descriptionLabel->setText(descriptions.value(pageIndex));
    backButton->setEnabled(pageIndex > 0);
    nextButton->setText(
        pageIndex == pages->count() - 1 ? "保存" : "下一步"
    );
    updateProtocolPage();
}

void ConfigurationGuideDialog::selectAuthenticationMethod(
    const QString &authType,
    const QString &loginDomain,
    const QString &loginUrl
)
{
    selectedAuthType = normalizedAuthType(authType);
    selectedLoginDomain = loginDomain;
    selectedLoginUrl = loginUrl;

    QString details = "当前选择：" + authenticationMethodName(selectedAuthType);
    if (!selectedLoginDomain.isEmpty())
    {
        details += "\n登录域：" + selectedLoginDomain;
    }
    selectedAuthenticationLabel->setText(details);
    fetchAuthenticationButton->setText("重新获取认证方式");
}

QString ConfigurationGuideDialog::authenticationMethodName(
    const QString &authType
) const
{
    if (authType == "psw")
    {
        return "用户名和密码";
    }
    if (authType == "smsCheckCode")
    {
        return "短信验证码";
    }
    if (authType == "cas")
    {
        return "CAS";
    }
    if (authType == "httpsOauth2")
    {
        return "OAuth2";
    }
    return authType.isEmpty() ? "尚未选择" : authType;
}
