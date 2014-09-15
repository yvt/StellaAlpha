#ifndef STSIMPLEANIMATOR_H
#define STSIMPLEANIMATOR_H

#include "ststdafx.h"
#include <QObject>

class STSimpleAnimator : public QObject
{
    Q_OBJECT
    float m_oldValue;
    float m_newValue;
    qint64 m_startTime;
    int m_duration;
    int m_animTimer;

    void startAnimation();

public:
    explicit STSimpleAnimator(QObject *parent = 0);
    
    bool isAnimating() const;
    float value() const;
    float finalValue() const{return m_newValue;}

protected:
    virtual void timerEvent(QTimerEvent *);

signals:
    void valueChanged(float);
public slots:
    
    void setValue(float);
    void setValueAnimated(float, int duration);

};

#endif // STSIMPLEANIMATOR_H
