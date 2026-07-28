#include "captchacanvas.h"

#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QVBoxLayout>

CaptchaCanvas::CaptchaCanvas(QWidget *parent) : QWidget(parent)
{
    setMouseTracking(false);
    setAttribute(Qt::WA_OpaquePaintEvent, true);
    setAutoFillBackground(false);
}

void CaptchaCanvas::setImage(const QPixmap &pm)
{
    m_img = pm;
    m_pointsPx.clear();
    updateGeometry();
    update();
    emit pointCountChanged(m_pointsPx.size());
}

void CaptchaCanvas::clearAll()
{
    if (m_pointsPx.isEmpty())
        return;
    m_pointsPx.clear();
    update();
    emit pointCountChanged(0);
}

void CaptchaCanvas::undoLast()
{
    if (m_pointsPx.isEmpty())
        return;
    m_pointsPx.removeLast();
    update();
    emit pointCountChanged(m_pointsPx.size());
}

void CaptchaCanvas::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    if (!m_img.isNull())
        p.drawPixmap(0, 0, m_img);

    QPen blackPen;
    blackPen.setWidth(2);
    blackPen.setColor(Qt::black);
    QBrush whiteBrush;
    whiteBrush.setStyle(Qt::SolidPattern);
    whiteBrush.setColor(Qt::white);
    QFont font;
    font.setPixelSize(14);
    p.setBrush(whiteBrush);
    p.setPen(blackPen);
    p.setFont(font);

    for (int i = 0; i < m_pointsPx.size(); ++i)
    {
        const QPointF pt = m_pointsPx[i];
        p.drawEllipse(pt, 8, 8);
        p.drawText(QRectF(pt - QPointF(6, 6), pt + QPointF(6, 6)), QString::number(i + 1),
                   QTextOption(Qt::AlignCenter));
    }
}

void CaptchaCanvas::mousePressEvent(QMouseEvent *ev)
{
    const QPointF pos = ev->position();
    if (ev->button() == Qt::LeftButton && !m_img.isNull())
    {
        if (QRectF(QPointF(0, 0), m_img.size()).contains(pos))
        {
            if (m_maxPoints < 0 || m_pointsPx.size() < m_maxPoints)
            {
                m_pointsPx.push_back(pos);
                update();
                emit pointCountChanged(m_pointsPx.size());
            }
        }
    }
}
