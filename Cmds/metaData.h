/*
 * Copyright (c) 2011, Intel Corporation.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef _METADATA_H_
#define _METADATA_H_

#include "tnvme.h"
#include "dnvme.h"
#include "globals.h"


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
* The working component is this class. This class requests meta data buffers
* from the RsrcMngr. After a buffer is allocated it will and must be inserted
* into the cmd. If no meta data buffers are desired, then don't allocate one.
* Meta data buffers are reserved and solely associated with the cmd for the
* entire lifetime of that cmd.
*
* @note This class may throw exceptions.
*/
class MetaData
{
public:
    MetaData();
    virtual ~MetaData();

    /**
     * Request dnvme to allocate a contiguous meta data buffer which will be
     * mmap'd back into user space for RW access. Remember all meta data buffers
     * are forced to be of equal size until such time the ctrlr becomes
     * disabled and a new size can be set in method RsrcMngr::SetMetaAllocSize()
     */
    void AllocMetaBuffer();

    /**
     * This method will return a previously allocated meta data buffer from a
     * call to AllocBuffer() for RW user space access. If no meta data buffer
     * has been allocated, then no meta data buffer will be associated with
     * this cmd.
     * @return The pointer to the memory, otherwise NULL indicates that no meta
     *      data buffer is in use/allocated.
     */
    uint8_t *GetMetaBuffer() { return mMetaData.buf; }
    uint16_t GetMetaBufferSize() { return mMetaData.size; }
    uint32_t GetMetaBufferID() { return mMetaData.ID; }

    /// Solely used to notify dnvme how to issue a PRP data buffer
    send_64b_bitmask GetMetaBitmask();


private:
    MetaDataBuf mMetaData;
};


#endif
