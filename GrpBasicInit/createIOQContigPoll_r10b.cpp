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

#include "createIOQContigPoll_r10b.h"
#include "globals.h"
#include "createACQASQ_r10b.h"
#include "grpDefs.h"
#include "../Queues/iocq.h"
#include "../Queues/iosq.h"
#include "../Utils/kernelAPI.h"
#include "../Utils/queues.h"

#define IOQ_ID                      1

namespace GrpBasicInit {

uint32_t CreateIOQContigPoll_r10b::NumEntriesIOQ =     5;


CreateIOQContigPoll_r10b::CreateIOQContigPoll_r10b(
    string grpName, string testName) :
    Test(grpName, testName, SPECREV_10b)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 7");
    mTestDesc.SetShort(     "Create contiguous IOCQ(poll) and IOSQ's");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Issue the admin commands Create contiguous I/O SQ and Create I/Q "
        "CQ(poll) to the ASQ and reap the resulting CE's from the ACQ to "
        "certify those Q's have been created.");
}


CreateIOQContigPoll_r10b::~CreateIOQContigPoll_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


CreateIOQContigPoll_r10b::
CreateIOQContigPoll_r10b(const CreateIOQContigPoll_r10b &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


CreateIOQContigPoll_r10b &
CreateIOQContigPoll_r10b::operator=(const CreateIOQContigPoll_r10b &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


Test::RunType
CreateIOQContigPoll_r10b::RunnableCoreTest(bool preserve)
{
    ///////////////////////////////////////////////////////////////////////////
    // All code contained herein must never permanently modify the state or
    // configuration of the DUT. Permanence is defined as state or configuration
    // changes that will not be restored after a cold hard reset.
    ///////////////////////////////////////////////////////////////////////////

    preserve = preserve;    // Suppress compiler error/warning
    return RUN_TRUE;        // This test is never destructive
}


void
CreateIOQContigPoll_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) Test CreateACQASQ_r10b has run prior.
     * 2) An individual test within this group cannot run, the entire group
     *    must be executed every time. Each subsequent test relies on the prior.
     * \endverbatim
     */
    uint64_t work;
    uint32_t isrCount;

    // Lookup objs which were created in a prior test within group
    SharedASQPtr asq = CAST_TO_ASQ(gRsrcMngr->GetObj(ASQ_GROUP_ID))
    SharedACQPtr acq = CAST_TO_ACQ(gRsrcMngr->GetObj(ACQ_GROUP_ID))

    // Verify assumptions are active/enabled/present/setup
    if (acq->ReapInquiry(isrCount, true) != 0) {
        acq->Dump(
            FileSystem::PrepDumpFile(mGrpName, mTestName, "acq",
            "notEmpty"), "Test assumption have not been met");
        throw FrmwkEx(HERE, 
            "The ACQ should not have any CE's waiting before testing");
    } else if (gRegisters->Read(CTLSPC_CAP, work) == false) {
        throw FrmwkEx(HERE, "Unable to determine MQES");
    }

    // Verify the min requirements for this test are supported by DUT
    work &= CAP_MQES;
    work += 1;      // convert to 1-based
    if (work < (uint64_t)NumEntriesIOQ) {
        LOG_NRM("Changing number of Q element from %d to %lld",
            NumEntriesIOQ, (unsigned long long)work);
        NumEntriesIOQ = work;
    } else if (gInformative->GetFeaturesNumOfIOCQs() < IOQ_ID) {
        throw FrmwkEx(HERE, "DUT doesn't support %d IOCQ's", IOQ_ID);
    } else if (gInformative->GetFeaturesNumOfIOSQs() < IOQ_ID) {
        LOG_ERR("DUT doesn't support %d IOSQ's", IOQ_ID);
        throw FrmwkEx(HERE);
    }


    gCtrlrConfig->SetIOCQES(gInformative->GetIdentifyCmdCtrlr()->
        GetValue(IDCTRLRCAP_CQES) & 0xf);
    Queues::CreateIOCQContigToHdw(mGrpName, mTestName, CALC_TIMEOUT_ms(1),
        asq, acq, IOQ_ID, NumEntriesIOQ, true, IOCQ_CONTIG_GROUP_ID, false, 0);


    gCtrlrConfig->SetIOSQES(gInformative->GetIdentifyCmdCtrlr()->
        GetValue(IDCTRLRCAP_SQES) & 0xf);
    Queues::CreateIOSQContigToHdw(mGrpName, mTestName, CALC_TIMEOUT_ms(1),
        asq, acq, IOQ_ID, NumEntriesIOQ, true, IOSQ_CONTIG_GROUP_ID, IOQ_ID, 0);
}

}   // namespace
