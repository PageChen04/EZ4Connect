#include "graphcaptchawindow.h"

#include "captchacanvas.h"

#include <QKeyEvent>
#include <QMessageBox>
#include <QProcess>
#include <QPushButton>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

GraphCaptchaWindow::GraphCaptchaWindow(QWidget *parent) : QDialog(parent), ui(new Ui::GraphCaptchaWindow)
{
    ui->setupUi(this);

    setWindowModality(Qt::WindowModal);
    setAttribute(Qt::WA_DeleteOnClose);

    connect(this, &QDialog::accepted, [&]() {
        QJsonObject obj;
        QJsonArray points;
        for (auto &point : ui->canvas->pointsPx())
        {
            QJsonArray pointArray;
            pointArray.append(int(point.x()));
            pointArray.append(int(point.y()));
            points.append(pointArray);
        }
        obj.insert("coordinates", points);
        obj.insert("width", ui->canvas->image().width());
        obj.insert("height", ui->canvas->image().height());
        QByteArray docJson = QJsonDocument(obj).toJson(QJsonDocument::Compact);
        emit finishCaptcha(docJson);
    });

    connect(this, &QDialog::rejected, [&]() {
        emit finishCaptcha(QByteArray("{}"));
    });

    connect(ui->buttonBox->button(QDialogButtonBox::Reset), &QAbstractButton::clicked, [&]() {
        ui->canvas->clearAll();
    });
}

GraphCaptchaWindow::~GraphCaptchaWindow()
{
    delete ui;
}

void GraphCaptchaWindow::setGraph(const QString &graphFile)
{
    QPixmap graph(graphFile);
    ui->canvas->setImage(graph);
    ui->canvas->setFixedSize(graph.size());
}
