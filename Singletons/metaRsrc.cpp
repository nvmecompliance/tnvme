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

#include "metaRsrc.h"
#include "../Utils/kernelAPI.h"
#include "../Exception/frmwkEx.h"


MetaRsrc::MetaRsrc()
{
    mFd = 0;
    mMetaAllocSize = 0;
}


MetaRsrc::MetaRsrc(int fd)
{
    mFd = fd;
    if (mFd < 0)
        throw FrmwkEx(HERE, "Object created with a bad FD=%d", fd);

    mMetaAllocSize = 0;
}


MetaRsrc::~MetaRsrc()
{
    FreeAllMetaBuf();
}


bool
MetaRsrc::SetMetaAllocSize(uint32_t allocSize)
{
    int rc;

    if (mMetaAllocSize == allocSize) {
        ;    // Already setup, no need to resend
    } else if (mMetaAllocSize != 0) {
        LOG_ERR("Attempt to reset meta data alloc size w/o 1st disabling");
        return false;
    } else if (allocSize % sizeof(uint32_t)) {
        LOG_ERR("Requested meta data alloc size is not modulo %ld",
            sizeof(uint32_t));
        return false;
    } else if ((rc = ioctl(mFd, NVME_IOCTL_METABUF_CREATE, allocSize)) < 0) {
        LOG_ERR("Meta data size request denied with error: %d", rc);
        return false;
    }

    LOG_NRM("Meta data alloc size set to: 0x%08X", allocSize);
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

        // Is the found ID within dnvme's max allowed range?
        if (metaBuf.ID < (1 << METADATA_UNIQUE_ID_BITS)) {
            LOG_NRM("Alloc meta data buf: size: 0x%08X, ID: 0x%06X",
                metaBuf.size, metaBuf.ID);

            // Request dnvme to reserve us some contiguous memory
            if ((rc = ioctl(mFd, NVME_IOCTL_METABUF_ALLOC, metaBuf.ID)) < 0) {
                throw FrmwkEx(HERE,
                    "Meta data alloc request denied with error: %d", rc);
            }

            // Map that memory back to user space for RW access
            metaBuf.buf = KernelAPI::mmap(metaBuf.size, metaBuf.ID,
                KernelAPI::MMR_META);
            if (metaBuf.buf == NULL) {
                LOG_ERR("Unable to mmap contig memory to user space");
                // Have to free the memory, not useful if we can't access it
                if ((rc =ioctl(mFd, NVME_IOCTL_METABUF_DELETE, metaBuf.ID)) < 0)
                    LOG_ERR("Meta data free request denied with error: %d", rc);
                throw FrmwkEx(HERE);
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
    while (resIter != mMetaReserved.end()) {
        ReleaseMetaBuf(*resIter);
        resIter = mMetaReserved.begin();
    }

    // Now we have to free all the elements in the released list
    while (mMetaReleased.size()) {
        MetaDataBuf tmp = mMetaReleased.back();
        mMetaReleased.pop_back();

        // Undo all which was done to create/reserve kernel meta data buffers
        KernelAPI::munmap(tmp.buf, tmp.size);

        // Specifically ignoring the error code because the memory may have
        // been deleted by a prior NVME_IOCTL_DEVICE_STATE call to dnvme. The
        // act of not freeing causes memory leak, the act of freeing to many
        // times is of no harm.
        rc = ioctl(mFd, NVME_IOCTL_METABUF_DELETE, tmp.ID);
    }

    mMetaAllocSize = 0;

    if ((mMetaReserved.empty() == false) || (mMetaReleased.empty() == false))
        throw FrmwkEx(HERE, "Internal program error: meta list not empty");
}
