#ifndef _METADATA_H_
#define _METADATA_H_

#include "tnvme.h"
#include "dnvme.h"
#include "../Singletons/memBuffer.h"


/**
* This class is the interface for the meta data buffer associated with the MPTR
* entry of a cmd. According to the NVME spec. meta data memory can only be
* contiguous as it is handed off to the NVME device. However, user space apps
* want access to this memory as RW also, but user space allocated memory is not
* guaranteed to be contiguous if it crosses page boundaries. The maximum
* size of meta data is allowed to be 16KB, and thus could cross page boundaries.
* Thus meta data memory is allocated contiguously in the kernel and mmap'd back
* into user space for RW accesses.
*
* The allocation technique has some guidelines to follow in order to allow
* the allocation of the correct sizes. The allocation are always DWORD aligned
* per the NVME spec, otherwise we would only succeed in crashing the kernel or
* seg faulting tnvme. There is nothing to gain by supplying a NVME device non
* properly aligned meta data buffers. Thus this is enforced always.
*
* There are 3 basic steps to force dnvme to provide a properly sized and aligned
* contiguous meta data buffer. 1st we have to notify dnvme which size of
* allocations we would like. 2nd we have to ask dvnme to allocate and track
* the memory for us, upon success tnvme must mmap that memory into user space.
* 3rd we must delete the allocation when we are done using it. It should be
* deleted when this object goes out of scope.
*
* @note This class may throw exceptions.
*/
class MetaData
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
    void SetBuffer(send_64b_bitmask prpFields, SharedMemBufferPtr memBuffer);

    /**
     * Accept a previously created Read Only (RO) IOQ buffer as the user data to
     * be populated in the PRP fields of a cmd. This method is only intended to
     * be used with the creation of IOQ's because those memories are never
     * allowed to be modified by user space; kernel only access, but user space
     * has RO access to those memories for debug.
     * @param prpFields Pass the appropriate combination of bitfields to
     *      indicate to dnvme how to populate the PRP fields of a cmd with
     *      this the buffer.
     * @param memBuffer Point to an IOQ's RO memory.
     */
    void SetBuffer(send_64b_bitmask prpFields, uint8_t const *memBuffer);

    /**
     * This method will return a non MemBuffer::NullMemBufferPtr if and only if
     * the SetBuffer(SharedMemBufferPtr) version is used to set the buffer.
     * @return A pointer to RW memory, otherwise MemBuffer::NullMemBufferPtr.
     */
    SharedMemBufferPtr GetRWBuffer() { return mBufRW; }

    /**
     * This method will always return a buffer if any of the SetBuffer() methods
     * have been used. This is always safe to return because the caller may
     * not change the associated user data buffer.
     * @return A pointer to the buffer, otherwise NULL indicates no buffer was
     *      setup, i.e. there is no user data at all for the PRP fields.
     */
    uint8_t const *GetROBuffer();

    /**
     * The PRP fields are dnvme specific, they notify dnvme which PRP fields
     * of this cmd may be touched by the dnvme to populate those respective
     * fields with physical pointers. The physical pointers point to either
     * a PRP list or the actual physical memory as set by SetBuffer().
     * @return The allowed PRP fields which can touched when sent to dnvme
     */
    send_64b_bitmask GetPrpFields() { return mPrpFields; }


private:
    /// Used for RW memory for a cmd's user data
    SharedMemBufferPtr mBufRW;
    /// Ussed for RO memory assoc with a IOQ's data memory
    uint8_t const *mBufRO;

    /// What fields in a cmd can we populate for the buffer?
    send_64b_bitmask mPrpFields;

    static const send_64b_bitmask ALLOWED_BITS;
};


#endif
