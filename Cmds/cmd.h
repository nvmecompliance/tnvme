#ifndef _CMD_H_
#define _CMD_H_

#include "tnvme.h"
#include "trackable.h"
#include "prpData.h"
#include "metaData.h"
#include "globals.h"

class Cmd;    // forward definition
typedef boost::shared_ptr<Cmd>              SharedCmdPtr;
#define CAST_TO_Cmd(shared_trackable_ptr)   \
        boost::shared_polymorphic_downcast<Cmd>(shared_trackable_ptr);

typedef enum {
    DATADIR_NONE,
    DATADIR_TO_DEVICE,
    DATADIR_FROM_DEVICE
} DataDir;


/**
* This class is the base class to all other cmd classes. It is not meant to
* be instantiated. This class contains all things common to cmds at a high
* level. After instantiation by a child the Init() method must be called be
* to attain something useful.
*
* @note This class may throw exceptions.
*/
class Cmd : public Trackable, public PrpData, public MetaData
{
public:
    /**
     * @param fd Pass the opened file descriptor for the device under test
     * @param objBeingCreated Pass the type of object this child class is
     */
    Cmd(int fd, Trackable::ObjType objBeingCreated);
    virtual ~Cmd();

    /// Dump the entire contents of the cmd buffer to the logging endpoint
    void LogCmd();

    /// Access to the actual cmd bytes
    SharedMemBufferPtr GetCmd() { return mCmdBuf; }

    uint16_t  GetCmdSizeB() { return mCmdBuf->GetBufSize(); }
    uint8_t   GetCmdSizeDW() { return (mCmdBuf->GetBufSize() / 4); }
    nvme_cmds GetCmdSet() { return mCmdSet; }
    DataDir   GetDataDir() { return mDataDir; }
    uint8_t   GetOpcode() { return GetByte(0, 0); }

    static const uint8_t BITMASK_FUSE_B;
    static const uint32_t BITMASK_FUSE_DW;
    uint8_t  GetFUSE()   { return (GetByte(0, 1) & BITMASK_FUSE_B); }
    void     SetFUSE(uint8_t newVal);

    uint32_t  GetNSID()   { return GetDword(1); }
    void      SetNSID(uint32_t newVal);

    /**
     * This value cannot be set because dnvme overwrites any value we would
     * set. dnvme does this to guarantee the uniqueness of the ID. However,
     * dnvme copies the CID back to user space when a cmd is sent for
     * submission to a SQ. So the exact value is only valid after a cmd is
     * submitted to an NVME device.
     */
    static const uint32_t BITMASK_CID_DW;
    uint16_t GetCID() { return (uint16_t)(GetDword(0) & BITMASK_CID_DW) >> 16; }

    /**
     * @param newVal Pass the new DWORD value to set in the cmd buffer
     * @param whichDW Pass [0->(GetCmdSizeDW()-1)] which DWORD to set
     */
    void SetDword(uint32_t newVal, uint8_t whichDW);
    uint32_t GetDword(uint8_t whichDW);
    /**
     * @param newVal Pass the new DWORD value to set in the cmd buffer
     * @param whichDW Pass [0->(GetCmdSizeDW()-1)] which DWORD to set
     * @param dwOffset Pass [0->3] for which byte offset within the DW to set
     */
    void SetByte(uint8_t newVal, uint8_t whichDW, uint8_t dwOffset);
    uint8_t GetByte(uint8_t whichDW, uint8_t dwOffset);


protected:
    /// file descriptor to the device under test
    int mFd;

    /**
     * Initialize this object.
     * @param cmdSet Pass which cmd set this cmd belongs
     * @param opcode Pass the opcode defining this cmd, per NVME spec.
     * @param dataDir Pass the direction of data for this cmd
     * @param cmdSize Pass the number of bytes consisting of a single cmd.
     */
    void Init(nvme_cmds cmdSet, uint8_t opcode, DataDir dataDir,
        uint16_t cmdSize);


private:
    Cmd();

    nvme_cmds mCmdSet;
    SharedMemBufferPtr mCmdBuf;
    DataDir mDataDir;
};


#endif
