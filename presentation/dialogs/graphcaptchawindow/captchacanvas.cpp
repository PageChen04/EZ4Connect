#include "captchacanvas.h"

#include <QMouseEvent>
#include <QPainter>
#include <QSizePolicy>

CaptchaCanvas::CaptchaCanvas(QWidget *parent) : QWidget(parent)
{
    setMouseTracking(false);
    setAttribute(Qt::WA_OpaquePaintEvent, true);
    setAutoFillBackground(false);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
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
    p.fillRect(rect(), palette().window());
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::SmoothPixmapTransform, true);
    const QRectF target = imageRect();
    if (!m_img.isNull())
        p.drawPixmap(target, m_img, QRectF(m_img.rect()));

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
        const QPointF imagePoint = m_pointsPx[i];
        const QPointF pt(target.left() + imagePoint.x() * target.width() / m_img.width(),
                         target.top() + imagePoint.y() * target.height() / m_img.height());
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
        const QRectF target = imageRect();
        if (target.contains(pos))
        {
            if (m_maxPoints < 0 || m_pointsPx.size() < m_maxPoints)
            {
                m_pointsPx.push_back(QPointF((pos.x() - target.left()) * m_img.width() / target.width(),
                                             (pos.y() - target.top()) * m_img.height() / target.height()));
                update();
                emit pointCountChanged(m_pointsPx.size());
            }
        }
    }
}

QRectF CaptchaCanvas::imageRect() const
{
    if (m_img.isNull() || width() <= 0 || height() <= 0)
        return {};

    QSizeF scaledSize = m_img.size();
    scaledSize.scale(size(), Qt::KeepAspectRatio);
    return QRectF(QPointF((width() - scaledSize.width()) / 2.0,
                          (height() - scaledSize.height()) / 2.0),
                  scaledSize);
}
