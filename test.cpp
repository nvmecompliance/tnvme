#include "tnvme.h"
#include "test.h"
#include "globals.h"

#define STS_ERROR_BITS      (STS_DPE | STS_SSE | STS_RMA | STS_RTA |        \
                             STS_STA | STS_DPD)
#define CSTS_ERROR_BITS     (CSTS_CFS)


Test::Test(int fd)
{
    mFd = fd;
    if (mFd < 0)
        LOG_DBG("Object created with a bad FD=%d", fd);
}


Test::~Test()
{
}


bool
Test::Run()
{
    try {
        if (RunCoreTest()) {
            if (GetStatusRegErrors()) {
                LOG_NRM("SUCCESSFUL test case run");
                return true;
            }
        }
    } catch (...) {
        ;   // Don't let exceptions propagate, converting to boolean error
    }

    LOG_NRM("FAILED test case run");
    return false;
}


bool
Test::GetStatusRegErrors()
{
    ULONGLONG value;
    ULONGLONG expectedValue;
    const PciSpcType *pciMetrics = gRegisters->GetPciMetrics();
    const CtlSpcType *ctlMetrics = gRegisters->GetCtlMetrics();


    // PCI STS register may indicate some error
    if (gRegisters->Read(PCISPC_STS, value) == false)
        return false;

    expectedValue = (value & ~STS_ERROR_BITS);
    if (value != expectedValue) {
        LOG_ERR("%s error bit #%d indicates test failure",
            pciMetrics[PCISPC_STS].desc,
            ReportOffendingBitPos(value, expectedValue));
        return false;
    }


    // PCI STS register may indicate some error
    if (gRegisters->Read(CTLSPC_CSTS, value) == false)
        return false;

    expectedValue = (value & ~CSTS_ERROR_BITS);
    if (value != expectedValue) {
        LOG_ERR("%s error bit #%d indicates test failure",
            ctlMetrics[CTLSPC_CSTS].desc,
            ReportOffendingBitPos(value, expectedValue));
        return false;
    }

    return true;
}


int
Test::ReportOffendingBitPos(ULONGLONG val, ULONGLONG expectedVal)
{
    ULONGLONG bitMask;

    for (int i = 0; i < (int)(sizeof(ULONGLONG)*8); i++) {
        bitMask = (1 << i);
        if ((val & bitMask) != (expectedVal & bitMask))
            return i;
    }
    return INT_MAX; // there is no mismatch
}

