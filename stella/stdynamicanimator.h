#ifndef STDYNAMICANIMATOR_H
#define STDYNAMICANIMATOR_H

#include <QObject>

class STDynamicAnimator
{
    double m_oldPos;
    double m_newPos;
    quint64 m_startTime;
    unsigned int m_duration:16;
    unsigned int m_futureDuration:15;
    bool m_hasFuturePos:1;
    double m_startVel;
    double m_futurePos;

public:
    void setPos(double newPos);
    void setPosAnimated(double newPos, unsigned int duration,
                        quint64 currentTime);
    double currentPos(quint64 currentTime);
    double currentVelocity(quint64 currentTime); // pixels/ms
    double finalPos() const{
        if(m_hasFuturePos)return m_futurePos;
        return m_newPos;
    }

    bool isAnimating(quint64 currentTime) const;
};

#endif // STDYNAMICANIMATOR_H
