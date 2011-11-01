#include "acq.h"
#include "globals.h"

SharedACQPtr ACQ::NullACQPtr;


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
    CQ::Init(0, 16, numEntries, true, 0);
}


void
ACQ::Init(uint16_t numEntries, SharedMemBufferPtr memBuffer)
{
    CQ::Init(0, 16, numEntries, memBuffer, true, 0);
}
