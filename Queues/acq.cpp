#include "acq.h"
#include "globals.h"


ACQ::ACQ() :
    CQ(0, Trackable::OBJTYPE_FENCE, Trackable::LIFETIME_FENCE, false)
{
    // This constructor will throw
}


ACQ::ACQ(int fd, Trackable::Lifetime life, bool ownByRsrcMngr) :
    CQ(fd, Trackable::OBJ_ACQ, life, ownByRsrcMngr)
{
}


ACQ::~ACQ()
{
}


void
ACQ::Init(uint16_t numEntries)
{
    CQ::Init(0, 16, numEntries, true, 0);
}


void
ACQ::Init(uint16_t numEntries, MemBuffer &memBuffer)
{
    CQ::Init(0, 16, numEntries, memBuffer, true, 0);
}
