#ifndef _METADATA_H_
#define _METADATA_H_

#include "tnvme.h"
#include "dnvme.h"
#include "../Singletons/memBuffer.h"


/**
* This class is the interface for the meta data buffer associated with the MPTR
* entry of a cmd. According to the NVME spec, meta data memory can only be
* contiguous as it is handed off to a NVME device. However, user space apps
* want access to this memory as RW, but user space allocated memory is not
* guaranteed to be contiguous if it crosses page boundaries. The maximum
* size of meta data is allowed to be 16KB, and thus could cross page boundaries.
* Therefore meta data memory is allocated contiguously in the kernel and mmap'd
* back into user space for RW accesses.
*
* Meta data buffers are separated into 2 components. The controlling component
* is located in RsrcMngr. Please see RsrcMngr class header for those details.
* The working component is this class. This class requests unique meta data ID's
* from the RsrcMngr because unique ID's are how user space tracks and
* identifies meta data buffers after they have been allocated by the kernel.
* After a buffer is allocated it will and must be inserted into the cmd. If
* no meta data buffers are desired, then don't allocate one. There is no way
* to deallocated a buffer once allocated, simply destroying the cmd and
* creating a new one should suffice.
*
* Allocations occur in the kernel and the kernel always enforces DWORD
* alignment. There is nothing to be gained by testing non properly aligned meta
* data buffers, but it will most certainly cause tnvme to eg fault or core dump.
*
* @note This class may throw exceptions.
*/
class MetaData
{
public:
    /**
     * @param fd Pass the opened file descriptor for the device under test
     */
    MetaData(int fd);
    virtual ~MetaData();

    /**
     * Request dnvme to allocate a contiguous meta data buffer which will be
     * mmap'd back into user space for RW access. Remember all meta data buffers
     * are forced to be of equal size until such time the ctrlr becomes
     * disabled and a new size can be set in method RsrcMngr::SetMetaAllocSize()
     */
    void AllocBuffer();

    /**
     * This method will return a previously allocated meta data buffer for RW
     * access. If no meta data buffer has been allocated, then no meta data
     * buffer will be associated with this cmd.
     * @return The pointer to the memory, otherwise NULL indicates that no meta
     *      data buffer is in use.
     */
    uint8_t *GetBuffer() { return mBuf; }
    uint16_t GetBufferSize() { return mBufSize; }


private:
    MetaData();

    /// file descriptor to the device under test
    int mFd;

    uint8_t *mBuf;
    uint32_t mBufId;
    uint16_t mBufSize;

    static const send_64b_bitmask ALLOWED_BITS;
};


#endif
