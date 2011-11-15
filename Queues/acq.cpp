#include "acq.h"
#include "globals.h"

SharedACQPtr ACQ::NullACQPtr;
const uint16_t ACQ::IDEAL_ELEMENT_SIZE = 16;


ACQ::ACQ() : CQ(0, Trackable::OBJTYPE_FENCE)
{
    // This constructor will throw
}


ACQ::ACQ(int fd) : CQ(fd, Trackable::OBJ_ACQ)
{
}


ACQ::~ACQ()
{
}


void
ACQ::Init(uint16_t numEntries)
{
    CQ::Init(0, IDEAL_ELEMENT_SIZE, numEntries, true, 0);
}
