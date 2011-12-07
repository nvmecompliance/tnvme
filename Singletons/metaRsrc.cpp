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

#include <math.h>
#include "metaRsrc.h"
#include "../Utils/kernelAPI.h"


MetaRsrc::MetaRsrc()
{
    mFd = 0;
    mMetaAllocSize = 0;
}


MetaRsrc::MetaRsrc(int fd)
{
    mFd = fd;
    if (mFd < 0) {
        LOG_DBG("Object created with a bad FD=%d", fd);
        return;
    }

    mMetaAllocSize = 0;
}


MetaRsrc::~MetaRsrc()
{
    FreeAllMetaBuf();
}


bool
MetaRsrc::SetMetaAllocSize(uint16_t allocSize)
{
    int rc;

    if (mMetaAllocSize == allocSize) {
        ;    // Already setup, no need to resend
    } else if (mMetaAllocSize != 0) {
        LOG_DBG("Attempt to reset meta data alloc size w/o 1st disabling");
        return false;
    } else if (allocSize % sizeof(uint32_t)) {
        LOG_DBG("Requested meta data alloc size is not modulo %ld",
            sizeof(uint32_t));
        return false;
    } else if (allocSize > (uint16_t)pow(2, 15)) {
        LOG_DBG("Meta data alloc size exceeds max, 0x%04X > 0x%04X",
            allocSize, (uint16_t)pow(2, 15));
        return false;
    } else if ((rc = ioctl(mFd, NVME_IOCTL_METABUF_CREATE, allocSize)) < 0) {
        LOG_ERR("Meta data size request denied with error: %d", rc);
        return false;
    }

    LOG_NRM("Meta data alloc size set to: 0x%04X", allocSize);
    mMetaAllocSize = allocSize;
    return true;
}


bool
MetaRsrc::ReserveMetaBuf(MetaDataBuf &metaBuf)
{
    int rc;
    metaBuf.ID = 0;
    metaBuf.size = mMetaAllocSize;
    deque<MetaDataBuf>::iterator resIter = mMetaReserved.begin();
    deque<MetaDataBuf>::iterator relIter = mMetaReleased.begin();


    // If meta buffers were previously alloc'd and then subsequently released,
    // we can quickly turn them back into usability again; saves calls to dnvme
    if (relIter != mMetaReleased.end()) {
        // Search the ordered elements for a place to insert into Reserved
        while (resIter != mMetaReserved.end()) {
            if ((*resIter).ID > (*relIter).ID)
                break;
            resIter++;
        }

        metaBuf = *relIter;
        LOG_NRM("Alloc meta data buf: size: 0x%08X, ID: 0x%06X",
            metaBuf.size, metaBuf.ID);

        mMetaReserved.insert(resIter, metaBuf);
        mMetaReleased.erase(relIter);
        return true;

    } else {
        // Search the ordered elements, need to find a unique ID
        while (resIter != mMetaReserved.end()) {
            if ((*resIter).ID == metaBuf.ID)
                metaBuf.ID++;
            else if ((*resIter).ID > metaBuf.ID)
                break;
            resIter++;
        }

        if (metaBuf.ID < (uint32_t)pow(2, METADATA_UNIQUE_ID_BITS)) {
            LOG_NRM("Alloc meta data buf: size: 0x%08X, ID: 0x%06X",
                metaBuf.size, metaBuf.ID);

            // Request dnvme to reserve us some contiguous memory
            if ((rc = ioctl(mFd, NVME_IOCTL_METABUF_ALLOC, metaBuf.ID)) < 0) {
                LOG_ERR("Meta data alloc request denied with error: %d", rc);
                throw exception();
            }

            // Map that memory back to user space for RW access
            metaBuf.buf = KernelAPI::mmap(mFd, metaBuf.size, metaBuf.ID,
                KernelAPI::MMR_META);
            if (metaBuf.buf == NULL) {
                LOG_DBG("Unable to mmap contig memory to user space");
                // Have to free the memory, not useful if we can't access it
                if ((rc =ioctl(mFd, NVME_IOCTL_METABUF_DELETE, metaBuf.ID)) < 0)
                    LOG_ERR("Meta data free request denied with error: %d", rc);
                throw exception();
            }

            mMetaReserved.insert(resIter, metaBuf);
            return true;
        }
    }
    return false;
}


void
MetaRsrc::ReleaseMetaBuf(MetaDataBuf metaBuf)
{
    deque<MetaDataBuf>::iterator resIter = mMetaReserved.begin();

    // Are we trying to release a default constructed MetaDataBuf, or illegal 1
    if (metaBuf == MetaDataBuf())
        return;

    // Search the ordered elements
    while (resIter != mMetaReserved.end()) {
        if ((*resIter).ID == metaBuf.ID) {
            mMetaReleased.push_back(*resIter);
            mMetaReserved.erase(resIter);
            break;
        }
        resIter++;
    }
}


void
MetaRsrc::FreeAllMetaBuf()
{
    int rc;
    deque<MetaDataBuf>::iterator resIter = mMetaReserved.begin();

    // Search the ordered elements, moves every reserved item into released
    while (resIter != mMetaReserved.end())
        ReleaseMetaBuf(*resIter);

    // Now we have to free all the elements in the reserved list
    while (mMetaReleased.size()) {
        MetaDataBuf tmp = mMetaReleased.back();
        mMetaReleased.pop_back();

        // Undo all which was done to create/reserve kernel meta data buffers
        KernelAPI::munmap(tmp.buf, tmp.size);
        if ((rc = ioctl(mFd, NVME_IOCTL_METABUF_DELETE, tmp.ID)) < 0)
            LOG_ERR("Meta data free request denied with error: %d", rc);
    }
}
