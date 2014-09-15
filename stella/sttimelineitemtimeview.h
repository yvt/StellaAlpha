#ifndef STTIMELINEITEMTIMEVIEW_H
#define STTIMELINEITEMTIMEVIEW_H

#include <QGraphicsObject>
#include <QDateTime>

class STOneSecTimer;
class STButtonView;

class STTimelineItemTimeView : public QGraphicsObject
{
    Q_OBJECT
    STOneSecTimer *m_timer;
    QDateTime m_dateTime;
    QString m_text;
    bool m_absView;
    int m_width;

    QColor m_backgroundColor;

public:
    explicit STTimelineItemTimeView(QDateTime dt, QGraphicsItem *parent = 0);
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual QRectF boundingRect() const;

    void setBackgroundColor(const QColor&);

    void updateText();
signals:
    
private slots:
    void timerFired();
};

#endif // STTIMELINEITEMTIMEVIEW_H
