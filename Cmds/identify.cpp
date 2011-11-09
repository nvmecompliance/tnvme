#include "identify.h"

#define CNS_BITMASK         0x01

const uint16_t Identify::IDEAL_DATA_SIZE =  4096;


Identify::Identify() :
    Cmd(0, Trackable::OBJTYPE_FENCE)
{
    // This constructor will throw
}


Identify::Identify(int fd) :
    Cmd(fd, Trackable::OBJ_IDENTIFY)
{
    Init(CMD_ADMIN, 0x06, DATADIR_FROM_DEVICE, 64);
    SetCNS(true);
}


Identify::~Identify()
{
}


void
Identify::SetCNS(bool ctrlr)
{
    uint8_t curVal = GetByte(10, 0);
    if (ctrlr)
        curVal |= CNS_BITMASK;
    else
        curVal &= ~CNS_BITMASK;
    SetByte(curVal, 10, 0);
}


bool
Identify::GetCNS()
{
    uint8_t curVal = GetByte(10, 0);
    if (curVal & CNS_BITMASK)
        return true;
    return false;
}
