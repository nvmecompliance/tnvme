#include "metaRsrc.h"


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
    } else if (allocSize > (2^15)) {
        LOG_DBG("Meta data alloc size exceeds max, 0x%04X > 0x%04X",
            allocSize, 2^15);
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
MetaRsrc::ReserveMetaId(uint32_t &uniqueId)
{
    uniqueId = 0;
    deque<uint32_t>::iterator iter = mMetaUniqueIds.begin();

    // Search the ordered elements
    while (iter != mMetaUniqueIds.end()) {
        if (*iter == uniqueId)
            uniqueId++;
        else if (*iter > uniqueId)
            break;
        iter++;
    }

    if (uniqueId < (2 ^ METADATA_UNIQUE_ID_BITS)) {
        mMetaUniqueIds.insert(iter, uniqueId);
        return true;
    }

    return false;
}


void
MetaRsrc::ReleaseMetaId(uint32_t uniqueId)
{
    deque<uint32_t>::iterator iter = mMetaUniqueIds.begin();

    // Search the ordered elements
    while (iter != mMetaUniqueIds.end()) {
        if (*iter == uniqueId) {
            mMetaUniqueIds.erase(iter);
            break;
        }
        iter++;
    }
}


void
MetaRsrc::ReleaseAllMetaId()
{
    deque<uint32_t>::iterator iter = mMetaUniqueIds.begin();

    // Search the ordered elements
    while (iter != mMetaUniqueIds.end())
        ReserveMetaId(*iter);
}
