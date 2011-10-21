#include "disableCompletely_r10b.h"
#include "../globals.h"


DisableCompletely_r10b::DisableCompletely_r10b(int fd) : Test(fd, SPECREV_10b)
{
    // 66 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 7");
    mTestDesc.SetShort(     "Validate basic hardware initialization duties");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Setup for forthcoming tests. It will completely disabled the "
        "controller. All driver and NVME device memory is freed to the system, "
        "nothing will be operational, not even IRQ's");
}


DisableCompletely_r10b::~DisableCompletely_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


bool
DisableCompletely_r10b::RunCoreTest()
{
    {
        LOG_NRM("Test RsrcMngr");

        // Create an object we expect to be freed after this test ends
        SharedTrackablePtr someMemory1;
        someMemory1 = gRsrcMngr->AllocObjTestLife(Trackable::OBJ_MEMBUFFER);
        if (someMemory1 == RsrcMngr::NullTrackablePtr) {
            LOG_DBG("Allocation of object failed");
            throw exception();
        }

        // Create an object we expect to be freed after this group ends
        SharedTrackablePtr someMemory2;
        someMemory2 = gRsrcMngr->AllocObjGrpLife(Trackable::OBJ_MEMBUFFER,
            "MyObj");
        if (someMemory2 == RsrcMngr::NullTrackablePtr) {
            LOG_DBG("Allocation of object failed");
            throw exception();
        }
    }

    {
        uint8_t cmpVal;
        uint8_t regVal;

        LOG_NRM("Test CtrlrConfig");
        regVal = 0x0e;
        gCtrlrConfig->SetIOCQES(regVal);
        if (gCtrlrConfig->GetIOCQES(cmpVal) == false) {
            LOG_DBG("Failed reading register");
            throw exception();
        } else if (cmpVal != regVal) {
            LOG_DBG("Compare failed: 0x%02X != 0x%02X", cmpVal, regVal);
            throw exception();
        }

        regVal = 0x0a;
        gCtrlrConfig->SetIOSQES(regVal);
        if (gCtrlrConfig->GetIOSQES(cmpVal) == false) {
            LOG_DBG("Failed reading register");
            throw exception();
        } else if (cmpVal != regVal) {
            LOG_DBG("Compare failed: 0x%02X != 0x%02X", cmpVal, regVal);
            throw exception();
        }

        regVal = 0x01;
        gCtrlrConfig->SetSHN(regVal);
        if (gCtrlrConfig->GetSHN(cmpVal) == false) {
            LOG_DBG("Failed reading register");
            throw exception();
        } else if (cmpVal != regVal) {
            LOG_DBG("Compare failed: 0x%02X != 0x%02X", cmpVal, regVal);
            throw exception();
        }

        regVal = 0x03;
        gCtrlrConfig->SetAMS(regVal);
        if (gCtrlrConfig->GetAMS(cmpVal) == false) {
            LOG_DBG("Failed reading register");
            throw exception();
        } else if (cmpVal != regVal) {
            LOG_DBG("Compare failed: 0x%02X != 0x%02X", cmpVal, regVal);
            throw exception();
        }

        regVal = 0x0c;
        gCtrlrConfig->SetMPS(regVal);
        if (gCtrlrConfig->GetMPS(cmpVal) == false) {
            LOG_DBG("Failed reading register");
            throw exception();
        } else if (cmpVal != regVal) {
            LOG_DBG("Compare failed: 0x%02X != 0x%02X", cmpVal, regVal);
            throw exception();
        }

        regVal = 0x04;
        gCtrlrConfig->SetCSS(regVal);
        if (gCtrlrConfig->GetCSS(cmpVal) == false) {
            LOG_DBG("Failed reading register");
            throw exception();
        } else if (cmpVal != regVal) {
            LOG_DBG("Compare failed: 0x%02X != 0x%02X", cmpVal, regVal);
            throw exception();
        }

        if (gCtrlrConfig->SetStateEnabled(ST_DISABLE) == false) {
            LOG_DBG("Failed");
            throw exception();
        } else if (gCtrlrConfig->GetStateEnabled() == true) {
            LOG_DBG("Should be disabled");
            throw exception();
        }

        // This won't go ready because we didn't create any ASQ or ACQ
        if (gCtrlrConfig->SetStateEnabled(ST_ENABLE) == true) {
            LOG_DBG("Failed");
            throw exception();
        }

        if (gCtrlrConfig->SetStateEnabled(ST_DISABLE_COMPLETELY) == false) {
            LOG_DBG("Failed");
            throw exception();
        } else if (gCtrlrConfig->GetStateEnabled() == true) {
            LOG_DBG("Should be disabled");
            throw exception();
        }
    }

    return true;
}
