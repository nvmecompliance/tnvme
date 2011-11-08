#include "metaData.h"

using namespace std;

const send_64b_bitmask PrpData::ALLOWED_BITS = (send_64b_bitmask)MASK_MPTR;


MetaData::MetaData(int fd)
{
    mFd = 0;
    mBuf = NULL;
    mBufId = 0;
    mBufSize = 0;
}


MetaData::MetaData(int fd)
{
    mFd = fd;
    if (mFd < 0) {
        LOG_DBG("Object created with a bad FD=%d", fd);
        return;
    }

    mBuf = NULL;
    mBufId = 0;
    mBufSize = 0;
}


MetaData::~MetaData()
{
    int rc;

    if (mBuf != NULL)
        KernelAPI::munmap(mBuf, mBufSize);

    // Request dnvme to release our contiguous memory
    if ((rc = ioctl(mFd, NVME_IOCTL_METABUF_DELETE, mBufId)) < 0)
        LOG_ERR("Meta data free request denied with error: %d", rc);
}


void
MetaData::AllocBuffer()
{
    int rc;

    // Request current meta data metrics from the RsrcMngr
    mBufSize = gRsrcMngr->GetMetaAllocSize();
    if (gRsrcMngr->ReserveMetaId(mBufId)) {
        LOG_DBG("Unable to reserve a new meta data ID");
        throw exception();
    }
    LOG_NRM("Alloc meta data buf: size: 0x%08X, ID: 0x%06X", mBufSize, mBufId);

    if (mBuf != NULL) {
        LOG_DBG("Buffer already setup, resizing is not supported");
        throw exception();
    }

    // Request dnvme to reserve us some contiguous memory
    if ((rc = ioctl(mFd, NVME_IOCTL_METABUF_ALLOC, mBufId)) < 0) {
        LOG_ERR("Meta data alloc request denied with error: %d", rc);
        throw exception();
    }

    // Map that memory back to user space for RW access
    mBuf = KernelAPI::mmap(mFd, mBufSize, mBufId, KernelAPI::MMR_META);
    if (mBuf == NULL) {
        LOG_DBG("Unable to mmap contig memory to user space");
        throw exception();
    }
}
