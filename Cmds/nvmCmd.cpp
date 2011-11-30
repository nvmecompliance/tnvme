#include "nvmCmd.h"


NVMCmd::NVMCmd() :
    Cmd(0, Trackable::OBJTYPE_FENCE)
{
    // This constructor will throw
}


NVMCmd::NVMCmd(int fd, Trackable::ObjType objBeingCreated) :
    Cmd(fd, objBeingCreated)
{
}


NVMCmd::~NVMCmd()
{
}


void
NVMCmd::Init(uint8_t opcode, DataDir dataDir)
{
    Cmd::Init(CMD_NVM, opcode, dataDir, 64);
}
