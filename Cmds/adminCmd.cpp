#include "adminCmd.h"


AdminCmd::AdminCmd() :
    Cmd(0, Trackable::OBJTYPE_FENCE)
{
    // This constructor will throw
}


AdminCmd::AdminCmd(int fd, Trackable::ObjType objBeingCreated) :
    Cmd(fd, objBeingCreated)
{
}


AdminCmd::~AdminCmd()
{
}


void
AdminCmd::Init(uint8_t opcode, DataDir dataDir)
{
    Cmd::Init(CMD_ADMIN, opcode, dataDir, 64);
}
