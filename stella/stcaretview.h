#ifndef STCARETVIEW_H
#define STCARETVIEW_H

#include <QGraphicsObject>

class STCaretView : public QGraphicsObject
{
    Q_OBJECT

    QSizeF m_size;
    float m_oldFadeOpacity;
    float m_newFadeOpacity;
    unsigned int m_fadeDuration;
    quint64 m_fadeStartTime;
    int m_timer;

    quint64 m_blankStartTime;

    bool m_isCaretVisible;

    bool shouldAnimate();
    float fadeOpacity();
    float blankOpacity();
    void startAnimate();

public:
    explicit STCaretView(QGraphicsItem *parent = 0);
    
    void setSize(const QSizeF&);

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual QRectF boundingRect() const;
    virtual void timerEvent(QTimerEvent *);

    void blankCaret();
    void showCaret();
    void hideCaret();

    bool isCaretVisible() const{return m_isCaretVisible;}

signals:
    
public slots:
    
};

#endif // STCARETVIEW_H
