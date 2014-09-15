#ifndef STSCROLLBARVIEW_H
#define STSCROLLBARVIEW_H

#include <QGraphicsObject>

class STScrollTrackBar;
class STScrollBarView : public QGraphicsObject
{
    friend class STScrollTrackBar;
    Q_OBJECT
public:
    explicit STScrollBarView(QGraphicsItem *parent = 0);
    
    enum Style{
        NoStyle,
        StellaLight,
        ClearWhite,
        ClearBlack,
        StellaBright
    };

    void setStyle(const Style&);
    void setSize(const QSize&);

    Style style() const{return m_style;}
    QSize size() const{return m_size;}

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual QRectF boundingRect() const;

    void setRange(double minValue, double maxValue);
    void setRange(double minValue, double maxValue, double value);
    void setValue(double);
    double value() const{return m_value;}
    void setLargeIncrement(double);
    double minValue() const{return m_minValue;}
    double maxValue() const{return m_maxValue;}

private:
    Style m_style;
    QSize m_size;

    STScrollTrackBar *m_trackBar;

    double m_minValue;
    double m_maxValue;
    double m_value;
    double m_largeIncrement;

    bool m_scrolling;
    double m_scrollOverflow;
    double m_coordRange;

    void relayout();
    double actualLargeIncrement() const;
    bool isScrollable() const;

protected:
    void trackBarDraggedBy(const QPointF& delta);

signals:
    void scrolled(); // only by user
    void valueChanged();
public slots:
    
};

#endif // STSCROLLBARVIEW_H
