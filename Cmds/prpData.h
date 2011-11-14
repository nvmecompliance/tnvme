#ifndef _PRPDATA_H_
#define _PRPDATA_H_

#include "tnvme.h"
#include "dnvme.h"
#include "../Singletons/memBuffer.h"


/**
* This class is the interface for the user data buffer associated with the PRP
* entries of a cmd.
*
* @note This class may throw exceptions.
*/
class PrpData
{
public:
    PrpData();
    virtual ~PrpData();

    /**
     * Accept a previously created Read/Write (RW) user space buffer as the
     * user data buffer to be populated in the PRP fields of a cmd.
     * @param prpFields Pass the appropriate combination of bitfields to
     *      indicate to dnvme how to populate the PRP fields of a cmd with
     *      this the buffer.
     * @param memBuffer Hand off this cmds data buffer.
     */
    void SetPrpBuffer(send_64b_bitmask prpFields, SharedMemBufferPtr memBuffer);

    /**
     * Accept a previously created Read Only (RO) IOQ buffer as the user data to
     * be populated in the PRP fields of a cmd. This method is only intended to
     * be used with the creation of IOQ's because those memories are never
     * allowed to be modified by user space; kernel only access, but user space
     * has RO access to those memories for debug.
     * @param prpFields Pass the appropriate combination of bitfields to
     *      indicate to dnvme how to populate the PRP fields of a cmd with
     *      this the buffer.
     * @param bufSize Pass the number of bytes consisting of memBuffer
     * @param memBuffer Point to an IOQ's RO memory.
     */
    void SetPrpBuffer(send_64b_bitmask prpFields, uint8_t const *memBuffer,
        uint64_t bufSize);

    /**
     * This method will return a non MemBuffer::NullMemBufferPtr if and only if
     * the SetBuffer(SharedMemBufferPtr) version is used to set the buffer.
     * @return A pointer to RW memory, otherwise MemBuffer::NullMemBufferPtr.
     */
    SharedMemBufferPtr GetRWPrpBuffer() { return mBufRW; }

    /**
     * This method will always return a buffer if any of the SetBuffer() methods
     * have been used. This is always safe to return because the caller may
     * not change the associated user data buffer.
     * @return A pointer to the buffer, otherwise NULL indicates no buffer was
     *      setup, i.e. there is no user data at all for the PRP fields.
     */
    uint8_t const *GetROPrpBuffer();
    uint64_t       GetROPrpBufferSize() { return mBufSize; }

    /// Solely used to notify dnvme how to issue a PRP data buffer
    send_64b_bitmask GetPrpBitmask() { return mPrpFields; }


private:
    /// Used for RW memory for a cmd's user data
    SharedMemBufferPtr mBufRW;
    /// Used for RO memory assoc with a IOQ's data memory
    uint8_t const *mBufRO;
    /// Number of bytes consisting of either mBufRO or mBufRW
    uint64_t mBufSize;

    /// What fields in a cmd can we populate for the buffer?
    send_64b_bitmask mPrpFields;

    static const send_64b_bitmask ALLOWED_BITS;
};


#endif
