#include "asq.h"
#include "globals.h"


ASQ::ASQ() :
    SQ(0, Trackable::OBJTYPE_FENCE, Trackable::LIFETIME_FENCE, false)
{
    // This constructor will throw
}


ASQ::ASQ(int fd, Trackable::Lifetime life, bool ownByRsrcMngr) :
    SQ(fd, Trackable::OBJ_ASQ, life, ownByRsrcMngr)
{
}


ASQ::~ASQ()
{
}


void
ASQ::Init(uint16_t numEntries)
{
    SQ::Init(0, 16, numEntries, 0);
}


void
ASQ::Init(uint16_t numEntries, MemBuffer &memBuffer)
{
    SQ::Init(0, 16, numEntries, memBuffer, 0);
}
