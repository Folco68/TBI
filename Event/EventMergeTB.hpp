#ifndef EVENTMERGETB_HPP
#define EVENTMERGETB_HPP

#include "Index/TechnicalBulletin.hpp"
#include <QEvent>

class EventMergeTB : public QEvent
{
  public:
    EventMergeTB(TechnicalBulletin* tb, bool merge);
    TechnicalBulletin* tb() const;
    bool               mergeKeywords() const;

  private:
    TechnicalBulletin* TB;
    bool               MergeKeywords;
};

#endif // EVENTMERGETB_HPP
