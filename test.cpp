#include "tnvme.h"
#include "test.h"
#include "globals.h"

#define STS_ERRORS             (STS_DPE | STS_SSE | STS_RMA | STS_RTA | STS_DPD)
#define PXCAP_PXDS_ERRORS      (PXDS_URD | PXDS_FED | PXDS_NFED | PXDS_CED)
#define AERCAP_AERUCES_ERRORS    0xffff
#define AERCAP_AERUCESEV_ERRORS  0xffff
#define CSTS_ERRORS              CSTS_CFS


Test::Test(int fd, SpecRev specRev)
{
    mFd = fd;
    mSpecRev = specRev;
    if (mFd < 0)
        LOG_DBG("Object created with a bad fd=%d", fd);
}


Test::~Test()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


bool
Test::Run()
{
    try {
        ResetStatusRegErrors();
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


void
Test::ResetStatusRegErrors()
{
    const vector<PciCapabilities> *cap = gRegisters->GetPciCapabilities();

    LOG_NRM("Resetting sticky PCI errors");
    gRegisters->Write(PCISPC_STS, STS_ERRORS);

    for (uint16_t i = 0; i < cap->size(); i++) {
        if (cap->at(i) == PCICAP_PXCAP) {
            gRegisters->Write(PCISPC_PXDS, PXCAP_PXDS_ERRORS);
        } else if (cap->at(i) == PCICAP_AERCAP) {
            gRegisters->Write(PCISPC_AERUCES, AERCAP_AERUCES_ERRORS);
            gRegisters->Write(PCISPC_AERUCESEV, AERCAP_AERUCESEV_ERRORS);
        }
    }
}


bool
Test::GetStatusRegErrors()
{
    uint64_t value = 0;
    uint64_t expectedValue = 0;
    const PciSpcType *pciMetrics = gRegisters->GetPciMetrics();
    const CtlSpcType *ctlMetrics = gRegisters->GetCtlMetrics();
    const vector<PciCapabilities> *cap = gRegisters->GetPciCapabilities();


    // PCI STS register may indicate some error
    if (gRegisters->Read(PCISPC_STS, value) == false)
        return false;
    expectedValue = (value & ~STS_ERRORS);
    if (value != expectedValue) {
        LOG_ERR("%s error bit #%d indicates test failure",
            pciMetrics[PCISPC_STS].desc,
            ReportOffendingBitPos(value, expectedValue));
        return false;
    }


    // Other optional PCI errors
    for (uint16_t i = 0; i < cap->size(); i++) {
        if (cap->at(i) == PCICAP_PXCAP) {
            if (gRegisters->Read(PCISPC_PXDS, value) == false)
                return false;
            expectedValue = (value & ~PXCAP_PXDS_ERRORS);
            if (value != expectedValue) {
                LOG_ERR("%s error bit #%d indicates test failure",
                    pciMetrics[PCISPC_PXDS].desc,
                    ReportOffendingBitPos(value, expectedValue));
                return false;
            }
        } else if (cap->at(i) == PCICAP_AERCAP) {
            if (gRegisters->Read(PCISPC_AERUCES, value) == false)
                return false;
            expectedValue = (value & ~AERCAP_AERUCES_ERRORS);
            if (value != expectedValue) {
                LOG_ERR("%s error bit #%d indicates test failure",
                    pciMetrics[PCISPC_PXDS].desc,
                    ReportOffendingBitPos(value, expectedValue));
                return false;

            }


            if (gRegisters->Read(PCISPC_AERUCESEV, value) == false)
                return false;
            expectedValue = (value & ~AERCAP_AERUCESEV_ERRORS);
            if (value != expectedValue) {
                LOG_ERR("%s error bit #%d indicates test failure",
                    pciMetrics[PCISPC_PXDS].desc,
                    ReportOffendingBitPos(value, expectedValue));
                return false;

            }
        }
    }


    // PCI STS register may indicate some error
    if (gRegisters->Read(CTLSPC_CSTS, value) == false)
        return false;
    expectedValue = (value & ~CSTS_ERRORS);
    if (value != expectedValue) {
        LOG_ERR("%s error bit #%d indicates test failure",
            ctlMetrics[CTLSPC_CSTS].desc,
            ReportOffendingBitPos(value, expectedValue));
        return false;
    }

    return true;
}


int
Test::ReportOffendingBitPos(uint64_t val, uint64_t expectedVal)
{
    uint64_t bitMask;

    for (int i = 0; i < (int)(sizeof(uint64_t)*8); i++) {
        bitMask = (1 << i);
        if ((val & bitMask) != (expectedVal & bitMask))
            return i;
    }
    return INT_MAX; // there is no mismatch
}


bool
Test::RunCoreTest()
{
    LOG_ERR("Children must over ride to provide functionality");
    return false;
}

