#include "cmd.h"

using namespace std;

const uint8_t  Cmd::BITMASK_FUSE_B = 0x03;
const uint32_t Cmd::BITMASK_FUSE_DW = (BITMASK_FUSE_B << 8);
const uint32_t Cmd::BITMASK_CID_DW = 0xffff0000;



Cmd::Cmd() :
    Trackable(Trackable::OBJTYPE_FENCE),
    mCmdBuf(new MemBuffer())
{
    // This constructor will throw
}


Cmd::Cmd(int fd, Trackable::ObjType objBeingCreated) :
    Trackable(objBeingCreated),
    mCmdBuf(new MemBuffer())
{
    mFd = fd;
    if (mFd < 0) {
        LOG_DBG("Object created with a bad FD=%d", fd);
        return;
    }

    mCmdSet = CMD_ADMIN;
    mDataDir = DATADIR_NONE;
}


Cmd::~Cmd()
{
}


void
Cmd::Init(nvme_cmds cmdSet, uint8_t opcode, DataDir dataDir, uint16_t cmdSize)
{
    switch (cmdSet) {
    case CMD_ADMIN:
    case CMD_NVM:
    case CMD_AON:
        mCmdSet = cmdSet;
        break;
    default:
        LOG_DBG("Illegal cmd set specified: %d", cmdSet);
        throw exception();
    }

    switch (dataDir) {
    case DATADIR_NONE:
    case DATADIR_TO_DEVICE:
    case DATADIR_FROM_DEVICE:
        mDataDir = dataDir;
        break;
    default:
        LOG_DBG("Illegal data direction specified: %d", dataDir);
        throw exception();
    }

    if (cmdSize % sizeof(uint32_t) != 0) {
        LOG_DBG("Illegal cmd size specified: %d", cmdSize);
        throw exception();
    }

    // Cmd buffers shall be DWORD aligned according to NVME spec., however
    // user space only has option to spec. QWORD alignment.
    mCmdBuf->InitAlignment(cmdSize, sizeof(void *), true, 0);
    SetByte(opcode, 0, 0);
}


uint32_t
Cmd::GetDword(uint8_t whichDW)
{
    if (whichDW >= GetCmdSizeDW()) {
        LOG_DBG("Cmd is not large enough to get requested value");
        throw exception();
    }
    return ((uint32_t *)mCmdBuf->GetBuffer())[whichDW];
}


uint8_t
Cmd::GetByte(uint8_t whichDW, uint8_t dwOffset)
{
    if (whichDW >= GetCmdSizeB()) {
        LOG_DBG("Cmd is not large enough to get requested value");
        throw exception();
    } else if (dwOffset > 4) {
        LOG_DBG("Illegal DW offset parameter passed: %d", dwOffset);
        throw exception();
    }
    return (uint8_t)(GetDword(whichDW) >> (dwOffset * 8));
}


void
Cmd::SetDword(uint32_t newVal, uint8_t whichDW)
{
    if (whichDW >= GetCmdSizeDW()) {
        LOG_DBG("Cmd is not large enough to set requested value");
        throw exception();
    }
    uint32_t *dw = (uint32_t *)mCmdBuf->GetBuffer();
    dw[whichDW] = newVal;
}


void
Cmd::SetByte(uint8_t newVal, uint8_t whichDW, uint8_t dwOffset)
{
    if (whichDW >= GetCmdSizeDW()) {
        LOG_DBG("Cmd is not large enough (%d DWORDS) to set req'd DWORD %d",
            GetCmdSizeDW(), whichDW);
        throw exception();
    } else if (dwOffset > 4) {
        LOG_DBG("Illegal DW offset parameter passed: %d", dwOffset);
        throw exception();
    }
    uint32_t dw = GetDword(whichDW);
    dw &= ~(0x000000ff << (dwOffset * 8));
    dw |= ((uint32_t)newVal << (dwOffset * 8));
    SetDword(dw, whichDW);
}


void
Cmd::SetFUSE(uint8_t newVal)
{
    uint8_t b1 = (GetByte(0, 1) & ~BITMASK_FUSE_B);
    b1 |= (newVal & BITMASK_FUSE_B);
    SetByte(b1, 0, 1);
}


void
Cmd::SetNSID(uint32_t newVal)
{
    SetDword(newVal, 1);
}


void
Cmd::LogCmd()
{
    LOG_NRM("Logging Cmd obj contents....");
    for (int i = 0; i < GetCmdSizeDW(); i++)
        LOG_NRM("Cmd DWORD%d: %s0x%08X", i, (i < 10) ? " " : "", GetDword(i));
}
