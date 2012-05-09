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

#ifndef _METARSRC_H_
#define _METARSRC_H_

#include <deque>
#include "tnvme.h"

#define METADATA_UNIQUE_ID_BITS         18      // 18 bits to rep a unique ID

struct MetaDataBuf {
    uint8_t *buf;
    uint32_t size;
    uint32_t ID;

    MetaDataBuf() : buf(NULL), size(0), ID(0) {}
    MetaDataBuf(uint8_t *b, uint32_t s, uint32_t i) : buf(b), size(s), ID(i) {}
    bool operator==(const MetaDataBuf &other) const {
        if ((other.buf == buf) && (other.size == size) && (other.ID == ID))
            return true;
        return false;
    }
    bool operator!=(const MetaDataBuf &other) const { return !(*this == other);
 }
};


/**
* This base class will handle the controlling aspect of meta data management in
* user space. Meta data buffers are separated into 2 components. The controlling
* component and the working component. Please see class header MetaData for
* details concerning the working component.
*
* Meta data buffers are allocated in dnvme. Each new allocation is the same
* size as the prior. The allocation size is controlled by this class. Once set
* the allocation size is not allowed to be changed until the ctrlr is disabled
* by CtrlrConfig::SetState(DISABLE_XXXX).
*
* This class also tracks in use meta data IDs. These IDs are used to reference
* allocated meta data buffers. The disabling of the controller releases all
* meta data memory and meta data IDs. If a new meta data buffer is desired this
* class will yield a non-reserved unique meta data ID and attach kernel
* memory to it. Remember to release previously reserved ID's/buffers because
* they are limited in number.
*
* Allocations occur in the kernel and the kernel always enforces DWORD
* alignment. There is nothing to be gained by testing non properly aligned meta
* data buffers, but it will most certainly cause tnvme to eg fault or core dump.
*
* @note This class does not throw exceptions.
*/
class MetaRsrc
{
public:
    /**
     * @param fd Pass the opened file descriptor for the device under test
     */
    MetaRsrc(int fd);
    ~MetaRsrc();

    /**
     * Sets the size of meta data buffer allocation within the kernel. Each
     * allocation requested by MetaData::AllocBuffer() must and will be of the
     * same size, as will be set by this method only. This allocation size
     * will persist, unchangeable until such time a
     * gCtrlrConfig->SetState(DISABLE_XXX) occurs, at which time this method
     * can be called again to setup a new allocation size.
     * @param allocSize Pass the requested size of meta data buffers; The
     *      value must be modulo sizeof(uint32_t).
     * @return true upon success, otherwise false
     */
    bool     SetMetaAllocSize(uint32_t allocSize);
    uint32_t GetMetaAllocSize() { return mMetaAllocSize; }

    /**
     * Meta data buffer's are tracked within the kernel by a unique reference
     * number. The burden of managing these ID's is left to user space, and this
     * object tracks all meta data unique ID's/buffers. Thus, if a meta data
     * buffer is needed an object must reserve a unique ID to which an
     * associated meta data buffer will be attached. Henceforth that kernel
     * memory is referenced by its ID.
     * @param metaBuf Returns the description of contiguous kernel meta data buf
     * @return true when a buffer has been allocated, otherwise false
     */
    bool ReserveMetaBuf(MetaDataBuf &metaBuf);
    void ReleaseMetaBuf(MetaDataBuf metaBuf);


protected:
    /// Releases all kernel meta data memory back to the system
    void FreeAllMetaBuf();


private:
    MetaRsrc();

    /// file descriptor to the device under test
    int mFd;

    /// Stores the size of each meta data allocation
    uint32_t mMetaAllocSize;

    /// Track all outstanding/reserved meta data unique IDs
    deque<MetaDataBuf> mMetaReserved;
    /// Previously reserved, but no longer in use, can be easily reserved again
    deque<MetaDataBuf> mMetaReleased;
};


#endif
