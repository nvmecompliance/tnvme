#include <sys/ioctl.h>
#include "registers.h"
#include "tnvme.h"
#include "dnvme/dnvme_ioctls.h"

const unsigned long long Registers::RegMasking[MAX_SUPPORTED_REG_SIZE + 1] = {
    0x0000000000000000,
    0x00000000000000ff,
    0x000000000000ffff,
    0x0000000000ffffff,
    0x00000000ffffffff,
    0x000000ffffffffff,
    0x0000ffffffffffff,
    0x00ffffffffffffff,
    0xffffffffffffffff
};


// Register metrics (meta data) to aid interfacing with the kernel driver
#define ZZ(a, b, c, d, e)       { b, c, d, e },
PciSpcType Registers::mPciSpcMetrics[] =
{
    PCISPC_TABLE
};
#undef ZZ

#define ZZ(a, b, c, d)          { b, c, d },
CtlSpcType Registers::mCtlSpcMetrics[] =
{
    CTLSPC_TABLE
};
#undef ZZ



Registers::Registers(int fd)
{
    LOG_NRM("Constructing register access");

    mFd = fd;
    if (mFd < 0) {
        LOG_DBG("Object created with a bad FD=%d", fd);
        return;
    }

    DiscoverPciCapabilities();
}


Registers::~Registers()
{
}


bool
Registers::Read(PciSpc reg, unsigned long long &value)
{
    int rc;
    int rsize = mPciSpcMetrics[reg].size;
    int roffset = mPciSpcMetrics[reg].offset;
    const char *rdesc = mPciSpcMetrics[reg].desc;
    struct nvme_read_generic io = { NVMEIO_PCI_HDR,
                                    roffset, rsize,
                                    (unsigned char *)&value };

    // Verify we discovered the true offset of requested register
    if (roffset == INT_MAX) {
        LOG_ERR("Offset of %s could not be discovered", rdesc);
        return false;
    } else if (rsize > MAX_SUPPORTED_REG_SIZE) {
        LOG_ERR("Size of %s is larger than supplied buffer", rdesc);
        return false;
    } else if ((rc = ioctl(mFd, NVME_IOCTL_READ_GENERIC, &io)) < 0) {
        LOG_ERR("Error reading %s: %d returned", rdesc, rc);
        return false;
    }

    value = value & RegMasking[rsize];
    LOG_NRM("%s", FormatRegister(rsize, rdesc, value).c_str());
    return true;
}


bool
Registers::Read(CtlSpc reg, unsigned long long &value)
{
    int rc;
    int rsize = mCtlSpcMetrics[reg].size;
    int roffset = mCtlSpcMetrics[reg].offset;
    const char *rdesc = mCtlSpcMetrics[reg].desc;
    struct nvme_read_generic io = { NVMEIO_BAR01,
                                    roffset, rsize,
                                    (unsigned char *)&value };

    // Verify we discovered the true offset of requested register
    if (roffset == INT_MAX) {
        LOG_ERR("Offset of %s could not be discovered", rdesc);
        return false;
    } else if (rsize > MAX_SUPPORTED_REG_SIZE) {
        LOG_ERR("Size of %s is larger than supplied buffer", rdesc);
        return false;
    } else if ((rc = ioctl(mFd, NVME_IOCTL_READ_GENERIC, &io)) < 0) {
        LOG_ERR("Error reading %s: %d returned", rdesc, rc);
        return false;
    }

    value = value & RegMasking[rsize];
    LOG_NRM("%s", FormatRegister(rsize, rdesc, value).c_str());
    return true;
}


bool
Registers::Read(nvme_io_space regSpc, int rsize, int roffset,
    unsigned char *value)
{
    int rc;
    struct nvme_read_generic io = { regSpc, roffset, rsize, value };

    if ((rc = ioctl(mFd, NVME_IOCTL_READ_GENERIC, &io)) < 0) {
        LOG_ERR("Error reading reg offset 0x%08X: %d returned", roffset, rc);
        return false;
    }

    LOG_NRM("%s", FormatRegister(regSpc, rsize, roffset, value).c_str());
    return true;
}


string
Registers::FormatRegister(int regSize, const char *regDesc,
    unsigned long long regValue)
{
    string result;
    char buffer[80];
    bool truncated = false;
    const char *regFmt[MAX_SUPPORTED_REG_SIZE + 1] = {
        "%s = 0x",      // not intending in over using this, just a place holder
        "%s = 0x%02llX",
        "%s = 0x%04llX",
        "%s = 0x%06llX",
        "%s = 0x%08llX",
        "%s = 0x%010llX",
        "%s = 0x%012llX",
        "%s = 0x%014llX",
        "%s = 0x%016llX"
    };

    if (regSize > MAX_SUPPORTED_REG_SIZE) {
        regSize = MAX_SUPPORTED_REG_SIZE;
        truncated = true;
    }

    snprintf(buffer, sizeof(buffer), regFmt[regSize], regDesc,
        (regValue & RegMasking[regSize]));
    result = buffer;
    if (truncated)
        result += "(TRUNCATED VALUE)";
    return result;
}


string
Registers::FormatRegister(nvme_io_space regSpc, int rsize, int roffset,
    unsigned char *value)
{
    unsigned char *tmp = value;
    char buffer[80];
    string result;
    int i,j;

    switch (regSpc) {

    case NVMEIO_PCI_HDR:
        snprintf(buffer, sizeof(buffer), "PCI space register...");
        break;
    case NVMEIO_BAR01:
        snprintf(buffer, sizeof(buffer), "Ctrl'r space register...");
        break;
    default:
        snprintf(buffer, sizeof(buffer), "Unknown space register");
        break;
    }
    result = buffer;

    for (i = 0, j = 0; j < rsize; i++, j++) {
        if (i == 0) {
            snprintf(buffer, sizeof(buffer), "\n    0x%08X: 0x", roffset+j);
            result += buffer;
        }
        snprintf(buffer, sizeof(buffer), "%02X ", *tmp++);
        result += buffer;
        if (i >= 15)
            i = -1;
    }

    return result;
}


void
Registers::DiscoverPciCapabilities()
{
    int rc;
    unsigned int capId;
    unsigned int capOffset;
    unsigned long long work;
    unsigned long long nextCap;
    struct nvme_read_generic io = { NVMEIO_PCI_HDR, 0, 4,
                                    (unsigned char *)&nextCap };


    // NOTE: We cannot report errors/violations of the spec as we parse PCI
    //      space because any non-conformance we may find could be changed
    //      in later releases of the NVME spec. Being that this is a
    //      non-versioned utility class we have no ability to note changes in
    //      the spec. The test architecture does handle specification mods but
    //      that is handled in the versioning of the test cases themselves.
    //      This is not a test case, thus we can't flag spec violations, this is
    //      a helper class for the test cases only.
    LOG_NRM("Discovering PCI capabilities");
    mPciCap.clear();
    if (Read(PCISPC_STS, work) == false)
        return;
    if ((work & STS_CL) == 0) {
        LOG_NRM("%s states there are no capabilities",
            mPciSpcMetrics[PCISPC_STS].desc);
        return;
    }

    // Find the index/offset to the 1st capability
    if (Read(PCISPC_CAP, nextCap) == false)
        return;
    nextCap <<= 8;   // small trick to make the while() work for 1st CAP reg

    // Traverse the link list of capabilities, PCI spec states when next ptr
    // becomes 0, then that is the capabilities among many.
    while (((nextCap >> 8) & RegMasking[1]) != 0) {
        io.offset = (unsigned int)((nextCap >> 8) & RegMasking[1]);
        if ((rc = ioctl(mFd, NVME_IOCTL_READ_GENERIC, &io)) < 0) {
            LOG_ERR("Error reading offset 0x%08X from PCI space: %d returned",
                io.offset, rc);
            return;
        }
        LOG_NRM("PCI space offset 0x%04X=0x%04X", io.offset,
            (unsigned int)(RegMasking[2] & nextCap));

        // For each capability we find, log the order in which it was found
        capId = (unsigned char)(RegMasking[1] & nextCap);
        capOffset = (unsigned int)(RegMasking[1] & (nextCap >> 8));
        switch (capId) {

        case 0x01:  // PCICAP_PMCAP
            LOG_NRM("Decoding PMCAP capabilities");
            mPciCap.push_back(PCICAP_PMCAP);
            mPciSpcMetrics[PCISPC_PID].offset = capOffset;
            break;

        case 0x05:  // PCICAP_MSICAP
            LOG_NRM("Decoding MSICAP capabilities");
            mPciCap.push_back(PCICAP_MSICAP);
            mPciSpcMetrics[PCISPC_MID].offset = capOffset;
            break;

        case 0x10:  // PCICAP_PXCAP
            LOG_NRM("Decoding PXCAP capabilities");
            mPciCap.push_back(PCICAP_PXCAP);
            mPciSpcMetrics[PCISPC_PXID].offset = capOffset;
            break;

        case 0x11:  // PCICAP_MSIXCAP
            LOG_NRM("Decoding MSIXCAP capabilities");
            mPciCap.push_back(PCICAP_MSIXCAP);
            mPciSpcMetrics[PCISPC_MXID].offset = capOffset;
            break;

        default:
            LOG_ERR("Decoded an unknown capability ID: 0x%02X", capId);
            return;
        }

        // For each capability we find update our knowledge of each reg's
        // offset within that capability.
        for (int i = 0; i < PCISPC_FENCE; i++) {
            if (mPciSpcMetrics[i].cap == mPciCap.back()) {
                mPciSpcMetrics[i].offset =
                    mPciSpcMetrics[i-1].offset + mPciSpcMetrics[i-1].size;
            }
        }
    }

    // Handle PCI extended capabilities which must start at offset 0x100.
    // Only one of these is possible, i.e. the AERCAP capabilities.
    io.offset = 0x100;
    io.nBytes = 4;
    if ((rc = ioctl(mFd, NVME_IOCTL_READ_GENERIC, &io)) < 0) {
        LOG_ERR("Error reading offset 0x%08X from PCI space: %d returned",
            io.offset, rc);
        return;
    }
    LOG_NRM("Extended PCI space offset 0x%04X=0x%08X", io.offset,
        (unsigned int)(RegMasking[4] & nextCap));
    capId = (unsigned int)(RegMasking[2] & nextCap);
    capOffset = (unsigned int)(RegMasking[2] & (nextCap >> 20));
    if (capId == 0x0001) {
        LOG_NRM("Decoding AERCAP capabilities");
        mPciCap.push_back(PCICAP_AERCAP);
        mPciSpcMetrics[PCISPC_AERID].offset = capOffset;
        for (int i = PCISPC_AERUCES; i <= PCISPC_AERTLP; i++) {
            mPciSpcMetrics[i].offset =
                mPciSpcMetrics[i-1].offset + mPciSpcMetrics[i-1].size;
        }
    } else {
        LOG_ERR("Decoded an unknown extended capability ID: 0x%04X", capId);
        return;
    }
}


