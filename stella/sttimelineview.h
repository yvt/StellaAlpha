#ifndef STTIMELINEVIEW_H
#define STTIMELINEVIEW_H

#include "stdynamiclistview.h"
#include "stfont.h"
#include "sttimeline.h"
#include <QTimer>
#include "stobjectmanager.h"

class STTimelineView : public STDynamicListView
{
    Q_OBJECT
    quint64 m_accountId;
    STTimeline *m_timeline;

    QTimer m_loadTimer;

    double heightForRow(quint64);
    STFont::Layout layoutRow(STTimeline::Row *);
public:
    explicit STTimelineView(QString timelineIdentifier, QGraphicsItem *parent = 0);
    virtual ~STTimelineView();

    QString timelineIdentifier() const;

    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);
protected:
    virtual QGraphicsItem *createView(quint64 id, double height, STClipView *parent);
signals:
    void linkActivated(STObjectManager::EntityRange *,
                       STObjectManager::LinkActivationInfo);
private slots:
    void timelineAddRow(quint64);
    void timelineRemoveRow(quint64);

    void loadTimeline();

    void linkActivatedSlot(STObjectManager::EntityRange *,
                           STObjectManager::LinkActivationInfo);
};

#endif // STTIMELINEVIEW_H
