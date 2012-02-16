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

#include "ctrlrResetIOQDeleted_r10b.h"
#include "globals.h"
#include "grpDefs.h"
#include "../Queues/iocq.h"
#include "../Queues/iosq.h"
#include "../Utils/kernelAPI.h"
#include "../Utils/queues.h"

#define IOCQ_ID                     1
#define IOSQ_ID                     2


CtrlrResetIOQDeleted_r10b::CtrlrResetIOQDeleted_r10b(int fd, string grpName,
    string testName, ErrorRegs errRegs) :
    Test(fd, grpName, testName, SPECREV_10b, errRegs)
{
    // 66 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 7");
    mTestDesc.SetShort(     "Ctrlr resets causes IOSQ & IOCQ's to be deleted");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Create a ratio of 2 IOSQ's associated with a single IOCQ, cause "
        "CC.EN=0, then verify all Q's were deleted by attempting to recreate "
        "those same Q's with identical parameters and verifying the successful "
        "CE within the ACQ. It succeeds because they are not duplicates and "
        "thus are allowed to be created again.");
}


CtrlrResetIOQDeleted_r10b::~CtrlrResetIOQDeleted_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


CtrlrResetIOQDeleted_r10b::
CtrlrResetIOQDeleted_r10b(const CtrlrResetIOQDeleted_r10b &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


CtrlrResetIOQDeleted_r10b &
CtrlrResetIOQDeleted_r10b::operator=(const CtrlrResetIOQDeleted_r10b &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


bool
CtrlrResetIOQDeleted_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) This is the 1st within GrpResets.
     * 2) The NVME device is disabled completely.
     * 3) All interrupts are disabled.
     *  \endverbatim
     */
    uint64_t work;
    uint16_t numEntriesIOQ = 10;

    // Verify the min requirements for this test are supported by DUT
    if (gRegisters->Read(CTLSPC_CAP, work) == false) {
        LOG_ERR("Unable to determine MQES");
        throw exception();
    }

    work &= CAP_MQES;
    if (work < (uint64_t)numEntriesIOQ) {
        LOG_NRM("Changing number of Q element from %d to %d",
            numEntriesIOQ, (uint16_t)work);
        numEntriesIOQ = work;
    } else if (gInformative->GetFeaturesNumOfIOCQs() < IOCQ_ID) {
        LOG_ERR("DUT doesn't support %d IOCQ's", IOCQ_ID);
        throw exception();
    } else if (gInformative->GetFeaturesNumOfIOSQs() < IOSQ_ID) {
        LOG_ERR("DUT doesn't support %d IOSQ's", IOSQ_ID);
        throw exception();
    }

    // Create Admin Q Objects for Group lifetime
    SharedACQPtr acq = CAST_TO_ACQ(
        gRsrcMngr->AllocObj(Trackable::OBJ_ACQ, ACQ_GROUP_ID))
    acq->Init(15);
    SharedASQPtr asq = CAST_TO_ASQ(
        gRsrcMngr->AllocObj(Trackable::OBJ_ASQ, ASQ_GROUP_ID))
    asq->Init(15);

    VerifyCtrlrResetDeletesIOQs(acq, asq, numEntriesIOQ);

    return true;
}

void
CtrlrResetIOQDeleted_r10b::VerifyCtrlrResetDeletesIOQs(SharedACQPtr acq,
    SharedASQPtr asq, uint16_t numEntriesIOQ)
{
    char work[20];

    // Set ctrl'r to enable state, create IOQ's and then disable the ctrl'r.
    // Re-enable the ctrl'r and try to create the same IOQ's, verify that
    // these IOQ's are created successfully.
    for (uint16_t i = 0; i < 2; i++) {
        snprintf(work, sizeof(work), "iter%d", i);

        gCtrlrConfig->SetCSS(CtrlrConfig::CSS_NVM_CMDSET);
        if (gCtrlrConfig->SetState(ST_ENABLE) == false)
            throw exception();

        gCtrlrConfig->SetIOCQES(IOCQ::COMMON_ELEMENT_SIZE_PWR_OF_2);
        Queues::CreateIOCQContigToHdw(mFd, mGrpName, mTestName,
            DEFAULT_CMD_WAIT_ms, asq, acq, IOCQ_ID, numEntriesIOQ, false,
            IOCQ_CONTIG_GROUP_ID, false, 0, work);

        // Create 2 IO SQ's, start with SQ ID 1 and associate all SQ's to one
        // CQ with IOCQ_ID
        gCtrlrConfig->SetIOSQES(IOSQ::COMMON_ELEMENT_SIZE_PWR_OF_2);
        for (uint16_t j = 1; j <= IOSQ_ID; j++) {
            Queues::CreateIOSQContigToHdw(mFd, mGrpName, mTestName,
                DEFAULT_CMD_WAIT_ms, asq, acq, j, numEntriesIOQ, false,
                IOSQ_CONTIG_GROUP_ID, IOCQ_ID, 0, work);
        }

        if (gCtrlrConfig->SetState(ST_DISABLE) == false)
            throw exception();
    }
}
