#include "createACQASQ_r10b.h"
#include "../globals.h"


CreateACQASQ_r10b::CreateACQASQ_r10b(int fd) : Test(fd, SPECREV_10b)
{
    // 66 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 7");
    mTestDesc.SetShort(     "Validate basic hardware initialization duties");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Create ACQ & ASQ kernel objects with group lifespan.");
}


CreateACQASQ_r10b::~CreateACQASQ_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


CreateACQASQ_r10b::
CreateACQASQ_r10b(const CreateACQASQ_r10b &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


CreateACQASQ_r10b &
CreateACQASQ_r10b::operator=(const CreateACQASQ_r10b &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


// todo remove after temporary unit tests are exhibited and accepted.
#include "../Queues/acq.h"
#include "../Queues/asq.h"
#include "../Queues/iocq.h"
#include "../Queues/iosq.h"

bool
CreateACQASQ_r10b::RunCoreTest()
{
    {
        LOG_NRM("Testing RsrcMngr: creating memBuffer obj for group life");
        LOG_NRM("Pause until keypress");
        getchar();

        // Create an object we expect to be freed after this test ends
        SharedTrackablePtr memBuf1;
        memBuf1 = gRsrcMngr->AllocObj(Trackable::OBJ_MEMBUFFER, "memBuf1");
        if (memBuf1 == Trackable::NullTrackablePtr) {
            LOG_DBG("Allocation of object failed");
            throw exception();
        }
        SharedMemBufferPtr memBuf2 = CAST_TO_SMBP(memBuf1);
    }

    {
        LOG_NRM("Testing creating memBuffer obj for test life");
        LOG_NRM("Pause until keypress");
        getchar();

        // Create an object we expect to be freed after this test ends
        SharedMemBufferPtr memBuf1 = SharedMemBufferPtr(new MemBuffer());
    }
    LOG_NRM("Pointer is now out of scope, did it die?");
    LOG_NRM("Pause until keypress");
    getchar();


    {
        uint8_t cmpVal;
        uint8_t regVal;

        LOG_NRM("Test CtrlrConfig: r/w to CC register, throws upon errors");
        LOG_NRM("Pause until keypress");
        getchar();

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

        if (gCtrlrConfig->SetState(ST_DISABLE) == false) {
            LOG_DBG("Failed");
            throw exception();
        } else if (gCtrlrConfig->IsStateEnabled() == true) {
            LOG_DBG("Should be disabled");
            throw exception();
        }

        // This won't go ready because we didn't create any ASQ or ACQ
        if (gCtrlrConfig->SetState(ST_ENABLE) == true) {
            LOG_DBG("Ctrlr became ready and wasn't suppose to");
            uint64_t someReg;
            gRegisters->Read(CTLSPC_CSTS, someReg);
            gRegisters->Read(CTLSPC_AQA, someReg);
            gRegisters->Read(CTLSPC_ASQ, someReg);
            gRegisters->Read(CTLSPC_ACQ, someReg);
            throw exception();
        }

        if (gCtrlrConfig->SetState(ST_DISABLE_COMPLETELY) == false) {
            LOG_DBG("Failed");
            throw exception();
        } else if (gCtrlrConfig->IsStateEnabled() == true) {
            LOG_DBG("Should be disabled");
            throw exception();
        }
    }
    LOG_NRM("If program still executing then test has passed");
    LOG_NRM("Pause until keypress");
    getchar();


    {
        LOG_NRM("Testing RsrcMngr: creating ACQ/ASQ/IOCQ/IOSQ obj's for group life");
        LOG_NRM("Pause until keypress");
        getchar();

        // Create an object we expect to be freed after this test ends
        SharedTrackablePtr ACQ1;
        ACQ1 = gRsrcMngr->AllocObj(Trackable::OBJ_ACQ, "ACQ1");
        if (ACQ1 == Trackable::NullTrackablePtr) {
            LOG_DBG("Allocation of object failed");
            throw exception();
        }
        SharedACQPtr ACQ2 = CAST_TO_ACQP(ACQ1);

        // Create an object we expect to be freed after this test ends
        SharedTrackablePtr ASQ1;
        ASQ1 = gRsrcMngr->AllocObj(Trackable::OBJ_ASQ, "ASQ1");
        if (ASQ1 == Trackable::NullTrackablePtr) {
            LOG_DBG("Allocation of object failed");
            throw exception();
        }
        SharedASQPtr ASQ2 = CAST_TO_ASQP(ASQ1);

        // Create an object we expect to be freed after this test ends
        SharedTrackablePtr IOSQ1;
        IOSQ1 = gRsrcMngr->AllocObj(Trackable::OBJ_IOSQ, "IOSQ1");
        if (IOSQ1 == Trackable::NullTrackablePtr) {
            LOG_DBG("Allocation of object failed");
            throw exception();
        }
        SharedIOSQPtr IOSQ2 = CAST_TO_IOSQP(IOSQ1);

        // Create an object we expect to be freed after this test ends
        SharedTrackablePtr IOCQ1;
        IOCQ1 = gRsrcMngr->AllocObj(Trackable::OBJ_IOCQ, "IOCQ1");
        if (IOCQ1 == Trackable::NullTrackablePtr) {
            LOG_DBG("Allocation of object failed");
            throw exception();
        }
        SharedIOCQPtr IOCQ2 = CAST_TO_IOCQP(IOCQ1);
    }


    {
        LOG_NRM("Testing creating ACQ/ASQ/IOCQ/IOSQ obj's for test life");
        LOG_NRM("Pause until keypress");
        getchar();

        // Create an object we expect to be freed after this test ends
        SharedACQPtr ACQ1 = SharedACQPtr(new ACQ(mFd));
        SharedASQPtr ASQ1 = SharedASQPtr(new ASQ(mFd));
        SharedIOCQPtr IOCQ1 = SharedIOCQPtr(new IOCQ(mFd));
        SharedIOSQPtr IOSQ1 = SharedIOSQPtr(new IOSQ(mFd));

        SharedMemBufferPtr memBuf1 = SharedMemBufferPtr(new MemBuffer());
        memBuf1->InitAlignment(4096, true, 0, 4096);
        IOCQ1->Init(1, 4, memBuf1, false, 0);
    }
    LOG_NRM("Pointers are now out of scope, did they die?");
    LOG_NRM("Pause until keypress");
    getchar();

    return true;
}
