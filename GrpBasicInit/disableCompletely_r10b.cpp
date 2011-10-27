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


// todo remove after temporary unit tests are exhibited and accepted.
#include "../Queues/acq.h"
#include "../Queues/asq.h"
#include "../Queues/iocq.h"
#include "../Queues/iosq.h"

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
#if 0
// Nisheeth needs to fix bug in QEMU, NVME comes ready when it is not suppose to
        // This won't go ready because we didn't create any ASQ or ACQ
        if (gCtrlrConfig->SetStateEnabled(ST_ENABLE) == true) {
            LOG_DBG("Ctrlr became ready and wasn't suppose to");
            uint64_t someReg;
            gRegisters->Read(CTLSPC_CSTS, someReg);
            gRegisters->Read(CTLSPC_AQA, someReg);
            gRegisters->Read(CTLSPC_ASQ, someReg);
            gRegisters->Read(CTLSPC_ACQ, someReg);
            throw exception();
        }

        if (gCtrlrConfig->SetStateEnabled(ST_DISABLE_COMPLETELY) == false) {
            LOG_DBG("Failed");
            throw exception();
        } else if (gCtrlrConfig->GetStateEnabled() == true) {
            LOG_DBG("Should be disabled");
            throw exception();
        }
#endif
    }

    {
        // Create an object we expect to be freed after this test ends
        SharedTrackablePtr someACQ1;
        someACQ1 = gRsrcMngr->AllocObjTestLife(Trackable::OBJ_ACQ);
        if (someACQ1 == RsrcMngr::NullTrackablePtr) {
            LOG_DBG("Allocation of object failed");
            throw exception();
        }
        SharedTrackablePtr someASQ1;
        someASQ1 = gRsrcMngr->AllocObjTestLife(Trackable::OBJ_ASQ);
        if (someASQ1 == RsrcMngr::NullTrackablePtr) {
            LOG_DBG("Allocation of object failed");
            throw exception();
        }
        SharedTrackablePtr someIOCQ1;
        someIOCQ1 = gRsrcMngr->AllocObjTestLife(Trackable::OBJ_IOCQ);
        if (someIOCQ1 == RsrcMngr::NullTrackablePtr) {
            LOG_DBG("Allocation of object failed");
            throw exception();
        }
        SharedTrackablePtr someIOCS1;
        someIOCS1 = gRsrcMngr->AllocObjTestLife(Trackable::OBJ_IOSQ);
        if (someIOCS1 == RsrcMngr::NullTrackablePtr) {
            LOG_DBG("Allocation of object failed");
            throw exception();
        }

        // Create an object we expect to be freed after this group ends
        SharedTrackablePtr someACQ10;
        someACQ10 = gRsrcMngr->AllocObjGrpLife(Trackable::OBJ_ACQ, "MyACQ");
        if (someACQ10 == RsrcMngr::NullTrackablePtr) {
            LOG_DBG("Allocation of object failed");
            throw exception();
        }
        SharedTrackablePtr someASQ10;
        someASQ10 = gRsrcMngr->AllocObjGrpLife(Trackable::OBJ_ACQ, "MyASQ");
        if (someASQ10 == RsrcMngr::NullTrackablePtr) {
            LOG_DBG("Allocation of object failed");
            throw exception();
        }
        SharedTrackablePtr someIOCQ10;
        someIOCQ10 = gRsrcMngr->AllocObjGrpLife(Trackable::OBJ_IOCQ, "MyIOCQ");
        if (someIOCQ10 == RsrcMngr::NullTrackablePtr) {
            LOG_DBG("Allocation of object failed");
            throw exception();
        }
        SharedTrackablePtr someIOSQ10;
        someIOSQ10 = gRsrcMngr->AllocObjGrpLife(Trackable::OBJ_IOSQ, "MyIOSQ");
        if (someIOSQ10 == RsrcMngr::NullTrackablePtr) {
            LOG_DBG("Allocation of object failed");
            throw exception();
        }
    }

    return true;
}
