#include "EventMergeTB.hpp"
#include "Global.hpp"

EventMergeTB::EventMergeTB(TechnicalBulletin* tb, bool merge)
    : QEvent(EVENT_MERGE_TB)
    , TB(tb)
    , MergeKeywords(merge)
{}

TechnicalBulletin* EventMergeTB::tb() const
{
    return this->TB;
}

bool EventMergeTB::mergeKeywords() const
{
    return this->MergeKeywords;
}
