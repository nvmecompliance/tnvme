#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "kernelAPI.h"
#include "globals.h"
#include "../Singletons/rsrcMngr.h"


KernelAPI::KernelAPI()
{
}


KernelAPI::~KernelAPI()
{
}


bool
KernelAPI::SoftReset()
{
    bool retVal;

    // In user space, in kernel space and in hardware, nothing remains.
    LOG_NRM("Performing soft reset");
    gRsrcMngr->FreeAllObj();
    gCtrlrConfig->SetState(ST_DISABLE_COMPLETELY);
    if ((retVal = gCtrlrConfig->SetIrqScheme(INT_NONE)) == false)
        LOG_ERR("Setting IRQ scheme failed");
    return retVal;
}


uint8_t *
KernelAPI::mmap(int fd, size_t bufLength, uint16_t bufID, MmapRegion region)
{
    int prot = PROT_READ;

    if (region >= MMAPREGION_FENCE) {
        LOG_DBG("Detected illegal region = %d", region);
        throw exception();
    }

    if (region == MMR_META)
        prot |= PROT_WRITE;

    off_t encodeOffset = bufID;
    encodeOffset |= ((off_t)region << METADATA_UNIQUE_ID_BITS);
    encodeOffset *= sysconf(_SC_PAGESIZE);
    return (uint8_t *)::mmap(0, bufLength, prot, MAP_SHARED, fd, encodeOffset);
}


void
KernelAPI::munmap(uint8_t *memPtr, size_t bufLength)
{
    ::munmap(memPtr, bufLength);
}


void
KernelAPI::DumpKernelMetrics(int fd, string filename)
{
    int rc;
    struct nvme_file dumpMe = { filename.length(), filename.c_str() };


    LOG_NRM("Dump dnvme metrics to filename: %s", filename.c_str());
    if ((rc = ioctl(fd, NVME_IOCTL_DUMP_METRICS, &dumpMe)) < 0) {
        LOG_DBG("Unable to dump dnvme metrics, error code = %d", rc);
        throw exception();
    }
}

