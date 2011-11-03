#include "nvmeCmd.h"


NVMECmd::NVMECmd() :
    Cmd(0, Trackable::OBJTYPE_FENCE)
{
    // This constructor will throw
}


NVMECmd::NVMECmd(int fd, Trackable::ObjType objBeingCreated) :
    Cmd(fd, objBeingCreated)
{
}


NVMECmd::~NVMECmd()
{
}


void
NVMECmd::Init(uint8_t opcode, DataDir dataDir)
{
    Cmd::Init(CMD_NVME, opcode, dataDir, 64);
}
