#ifndef STFILTEREDSTREAMER_H
#define STFILTEREDSTREAMER_H

#include "ststreamer.h"
#include <QMap>
#include "stobjectmanager.h"
#include <QSet>

class STFilteredStreamer : public STStreamer
{
    Q_OBJECT
public:
    explicit STFilteredStreamer(STAccount *account);
    enum FilterType{
        User,
        Keyword,
        Location
    };

    struct Filter{
        FilterType type;
        QString value;
        bool operator ==(const Filter& flt) const{
            return type==flt.type && value==flt.value;
        }
        bool operator <(const Filter& flt) const{
            if((int)type<(int)flt.type)
                return true;
            return value<flt.value;
        }
        bool matches(const STObjectManager::StatusRef&) const;
    };

    class FilterHandle{
        STFilteredStreamer *m_streamer;
        bool m_valid;
        Filter m_filter;
    public:
        FilterHandle();
        FilterHandle(STFilteredStreamer *streamer);
        ~FilterHandle();

        void setFilter(const Filter& flt);
        const Filter& filter() const{return m_filter;}
        bool isNull() const{return !m_valid;}
        bool isValid() const{return m_valid;}
        void clear();
    };

    typedef QMap<Filter, int> FilterMap;

public slots:
    void addFilter(STFilteredStreamer::Filter);
    void removeFilter(STFilteredStreamer::Filter);

private:
    FilterMap m_filters;
    QMap<Filter, int> m_newFilters;

    void updateParameter();
signals:
    void statusReceived(STObjectManager::StatusRef);
    void statusRemoved(quint64);
    void filterAdded(STFilteredStreamer::Filter);
protected slots:
    virtual void lineReceived(const QByteArray &);
    void reportNewFilters();
};

#endif // STFILTEREDSTREAMER_H
