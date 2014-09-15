#ifndef STTIMELINEPANE_H
#define STTIMELINEPANE_H

#include "stpane.h"
#include "stviewanimator.h"
#include "stheaderview.h"

class STTimelineView;
class STTimelinePane : public STPane
{
    Q_OBJECT

    STViewAnimator m_animator;
    STTimelineView *m_timelineView;
    QString m_title;

    STTimelineView *createView(QString);


public:
    explicit STTimelinePane(QGraphicsItem *parent = 0);
    virtual ~STTimelinePane();

    virtual void relayout();

    void setTimelineIdentifier(QString);
    void setTimelineIdentifierAnimated(QString);

    QString timelineIdentifier();
    virtual STHeaderView::Contents headerViewContents();

    void setTitle(QString t);

    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

signals:
    
public slots:
    
};

#endif // STTIMELINEPANE_H
