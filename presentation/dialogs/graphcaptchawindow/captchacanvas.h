#pragma once

#include <QWidget>

class CaptchaCanvas : public QWidget
{
    Q_OBJECT
public:
    explicit CaptchaCanvas(QWidget *parent = nullptr);

    void setImage(const QPixmap &pm);
    const QPixmap &image() const
    {
        return m_img;
    }

    const QVector<QPointF> &pointsPx() const
    {
        return m_pointsPx;
    }
    void clearAll();
    void undoLast();
    void setMaxPoints(int n)
    {
        m_maxPoints = n;
    }

    QSize sizeHint() const override
    {
        return m_img.size();
    }

signals:
    void pointCountChanged(int count);

protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *ev) override;

private:
    QPixmap m_img;
    QVector<QPointF> m_pointsPx;
    int m_maxPoints = -1;
};
