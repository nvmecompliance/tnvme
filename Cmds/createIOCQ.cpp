#include "createIOCQ.h"


CreateIOCQ::CreateIOCQ() :
    Cmd(0, Trackable::OBJTYPE_FENCE)
{
    // This constructor will throw
}


CreateIOCQ::CreateIOCQ(int fd) :
    Cmd(fd, Trackable::OBJ_IOCQ)
{
    Init(CMD_ADMIN, 0x01, DATADIR_TO_DEVICE, 64);
}


CreateIOCQ::~CreateIOCQ()
{
}
