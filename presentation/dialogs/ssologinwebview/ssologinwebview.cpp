#include "ssologinwebview.h"
#include "ui_ssologinwebview.h"

#include <QDialogButtonBox>
#include <QLineEdit>
#include <QToolButton>
#include <QWebEngineHistory>
#include <QWebEngineView>
#include <QWebEnginePage>
#include <QtWebEngineCore>
#include <QUrl>

SsoLoginWebView::SsoLoginWebView(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SsoLoginWebView)
{
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);
    setModal(true);

    setupConnections();
}

SsoLoginWebView::~SsoLoginWebView()
{
    delete ui;
}

void SsoLoginWebView::setupConnections()
{
    connect(ui->backButton, &QToolButton::clicked, ui->webEngineView, &QWebEngineView::back);
    connect(ui->forwardButton, &QToolButton::clicked, ui->webEngineView, &QWebEngineView::forward);
    connect(ui->reloadButton, &QToolButton::clicked, ui->webEngineView, &QWebEngineView::reload);
    connect(ui->dialogButtonBox, &QDialogButtonBox::rejected, this, [&]()
    {
        if (!loginCompletedEmitted)
        {
            loginCompletedEmitted = true;
            loginCompleted(QString());
        }
    });

    connect(ui->addressLineEdit, &QLineEdit::returnPressed, this, [&]()
    {
        const QUrl target = QUrl::fromUserInput(ui->addressLineEdit->text());
        ui->webEngineView->load(target);
    });

    connect(ui->webEngineView, &QWebEngineView::urlChanged, this, [&](const QUrl &url)
    {
        ui->addressLineEdit->setText(url.toString());
    });

    connect(ui->webEngineView->page(), &QWebEnginePage::navigationRequested, this,
            [&](QWebEngineNavigationRequest &request) {
                if (request.navigationType() == QWebEngineNavigationRequest::NavigationType::RedirectNavigation &&
                    isCallbackUrl(request.url()))
                {
                    if (!loginCompletedEmitted)
                    {
                        loginCompletedEmitted = true;
                        loginCompleted(request.url().toString());
                    }
                    request.reject();
                    this->close();
                }
                else
                    request.accept();
            });
}

void SsoLoginWebView::setInitialUrl(const QUrl &url)
{
    if (!url.isEmpty())
    {
        ui->addressLineEdit->setText(url.toString());
        ui->webEngineView->load(url);
    }
}

void SsoLoginWebView::setCallbackServerUrl(const QUrl &url)
{
    callbackServerUrl = url;
}

bool SsoLoginWebView::isCallbackUrl(const QUrl &url) const
{
    const auto effectivePort = [](const QUrl &value)
    {
        const int defaultPort =
            value.scheme().compare("https", Qt::CaseInsensitive) == 0 ? 443 : 80;
        return value.port(defaultPort);
    };
    return url.scheme().compare(
               callbackServerUrl.scheme(),
               Qt::CaseInsensitive
           ) == 0
        && url.host().compare(
               callbackServerUrl.host(),
               Qt::CaseInsensitive
           ) == 0
        && effectivePort(url) == effectivePort(callbackServerUrl);
}

QUrl SsoLoginWebView::currentUrl() const
{
    return ui->webEngineView->url();
}

void SsoLoginWebView::closeEvent(QCloseEvent *event)
{
    if (!loginCompletedEmitted)
    {
        loginCompletedEmitted = true;
        loginCompleted(QString());
    }

    QDialog::closeEvent(event);
}
