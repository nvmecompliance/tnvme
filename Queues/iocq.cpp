#include <math.h>
#include "iocq.h"
#include "globals.h"

SharedIOCQPtr IOCQ::NullIOCQPtr;
const uint16_t IOCQ::COMMON_ELEMENT_SIZE = 16;
const uint8_t  IOCQ::COMMON_ELEMENT_SIZE_PWR_OF_2 = 4;


IOCQ::IOCQ() : CQ(0, Trackable::OBJTYPE_FENCE)
{
    // This constructor will throw
}


IOCQ::IOCQ(int fd) : CQ(fd, Trackable::OBJ_IOCQ)
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
    CQ::Init(qId, (uint16_t)pow(2, entrySize), numEntries, irqEnabled, irqVec);
}


void
IOCQ::Init(uint16_t qId, uint16_t numEntries,
    const SharedMemBufferPtr memBuffer, bool irqEnabled, uint16_t irqVec)
{
    uint8_t entrySize;

    if (gCtrlrConfig->GetIOCQES(entrySize) == false) {
        LOG_ERR("Unable to learn IOCQ entry size");
        throw exception();
    }
    CQ::Init(qId, (uint16_t)pow(2, entrySize), numEntries, memBuffer,
        irqEnabled, irqVec);
}
