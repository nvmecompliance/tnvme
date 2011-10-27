#include "iocq.h"
#include "globals.h"


IOCQ::IOCQ() :
    CQ(0, Trackable::OBJTYPE_FENCE, Trackable::LIFETIME_FENCE, false)
{
    // This constructor will throw
}


IOCQ::IOCQ(int fd, Trackable::Lifetime life, bool ownByRsrcMngr) :
    CQ(fd, Trackable::OBJ_IOCQ, life, ownByRsrcMngr)
{
}


IOCQ::~IOCQ()
{
}


void
IOCQ::Init(uint16_t qId, uint16_t numEntries, bool irqEnabled,
    uint16_t irqVec)
{
    uint8_t entrySize;

    if (gCtrlrConfig->GetIOCQES(entrySize) == false) {
        LOG_ERR("Unable to learn IOCQ entry size");
        throw exception();
    }
    CQ::Init(qId, entrySize, numEntries, irqEnabled, irqVec);
}


void
IOCQ::Init(uint16_t qId, uint16_t numEntries, MemBuffer &memBuffer,
    bool irqEnabled, uint16_t irqVec)
{
    uint8_t entrySize;

    if (gCtrlrConfig->GetIOCQES(entrySize) == false) {
        LOG_ERR("Unable to learn IOCQ entry size");
        throw exception();
    }
    CQ::Init(qId, entrySize, numEntries, memBuffer, irqEnabled, irqVec);
}
