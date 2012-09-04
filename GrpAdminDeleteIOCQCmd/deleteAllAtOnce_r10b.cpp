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

#include "deleteAllAtOnce_r10b.h"
#include "globals.h"
#include "grpDefs.h"
#include "../Utils/irq.h"
#include "../Cmds/deleteIOSQ.h"
#include "../Cmds/deleteIOCQ.h"

namespace GrpAdminDeleteIOCQCmd {


DeleteAllAtOnce_r10b::DeleteAllAtOnce_r10b(
    string grpName, string testName) :
    Test(grpName, testName, SPECREV_10b)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 5");
    mTestDesc.SetShort(     "Delete all IOCQ's at the same time");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Calc X, where X = the smaller of 1) max num cmds can fit within ASQ "
        "or 2) max num IOQ's DUT will support. Create X IOCQ/IOSQ pairs; "
        "QID's start with 1 and continue to X, all with num elements = 2. "
        "Issue X DeleteIOCQ cmds into the ASQ, thus deleting every IOSQ "
        "and reap all CE's, expect success. Then issue X DeleteIOCQ cmds "
        "and make sure the doorbell is rung for all cmds at once. Expect to "
        "reap all X cmds successfully. Verify CE.CID with expected value.");
}


DeleteAllAtOnce_r10b::~DeleteAllAtOnce_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


DeleteAllAtOnce_r10b::
DeleteAllAtOnce_r10b(const DeleteAllAtOnce_r10b &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


DeleteAllAtOnce_r10b &
DeleteAllAtOnce_r10b::operator=(const DeleteAllAtOnce_r10b &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


Test::RunType
DeleteAllAtOnce_r10b::RunnableCoreTest(bool preserve)
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
DeleteAllAtOnce_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * None.
     * \endverbatim
     */
    uint64_t maxIOQEntries = 2;
    uint16_t expectedCIDR1 = 0;

    if (gCtrlrConfig->SetState(ST_DISABLE_COMPLETELY) == false)
        throw FrmwkEx(HERE);

    LOG_NRM("Create admin queues ACQ and ASQ");
    SharedACQPtr acq = SharedACQPtr(new ACQ(gDutFd));
    acq->Init(MAX_ADMIN_Q_SIZE);

    SharedASQPtr asq = SharedASQPtr(new ASQ(gDutFd));
    asq->Init(MAX_ADMIN_Q_SIZE);

    // All queues will use identical IRQ vector
    IRQ::SetAnySchemeSpecifyNum(1);

    gCtrlrConfig->SetCSS(CtrlrConfig::CSS_NVM_CMDSET);
    if (gCtrlrConfig->SetState(ST_ENABLE) == false)
        throw FrmwkEx(HERE);

    LOG_NRM("Calculate the max IOQ's the DUT supports.");
    uint32_t maxIOQs = MIN(gInformative->GetFeaturesNumOfIOSQs(),
        gInformative->GetFeaturesNumOfIOCQs());

    LOG_NRM("Compute X, minimum of ASQ entries (%d) or max IOQ's (%d)",
        asq->GetNumEntries(), maxIOQs);
    uint32_t X = MIN(asq->GetNumEntries(), maxIOQs);

    LOG_NRM("Setup element sizes for the IOQ's");
    gCtrlrConfig->SetIOCQES(gInformative->GetIdentifyCmdCtrlr()->
        GetValue(IDCTRLRCAP_CQES) & 0xf);
    gCtrlrConfig->SetIOSQES(gInformative->GetIdentifyCmdCtrlr()->
        GetValue(IDCTRLRCAP_SQES) & 0xf);

    LOG_NRM("Create %d IOQSQ/IOCQ pairs", X);
    vector<SharedIOCQPtr> iocqs;
    vector<SharedIOSQPtr> iosqs;
    for (uint32_t ioqId = 1; ioqId <= X; ioqId++) {
        LOG_NRM("Create IOSQ and IOCQ with QID #%d", ioqId);
        SharedIOCQPtr iocq = Queues::CreateIOCQContigToHdw(mGrpName,
            mTestName, CALC_TIMEOUT_ms(1), asq, acq, ioqId, maxIOQEntries,
            false, IOCQ_GROUP_ID, true, 0);
        SharedIOSQPtr iosq = Queues::CreateIOSQContigToHdw(mGrpName,
            mTestName, CALC_TIMEOUT_ms(1), asq, acq, ioqId, maxIOQEntries,
            false, IOSQ_GROUP_ID, ioqId, 0);
        iocqs.push_back(iocq);
        iosqs.push_back(iosq);
        expectedCIDR1 += 2;
    }

    LOG_NRM("Issue %d simultaneous deleteIOSQ cmds.", X);
    DelAllIOSQsAndVerify(acq, asq, iosqs, expectedCIDR1);
    LOG_NRM("Issue %d simultaneous deleteIOCQ cmds.", X);
    DelAllIOCQsAndVerify(acq, asq, iocqs, expectedCIDR1);
}


void
DeleteAllAtOnce_r10b::DelAllIOSQsAndVerify(SharedACQPtr acq, SharedASQPtr asq,
    vector<SharedIOSQPtr> iosqs, uint16_t &expectedCIDR1)
{
    uint32_t isrCount;
    uint32_t ceRemain;
    uint32_t numReaped;
    uint32_t numCE;
    uint16_t uniqueId;

    uint16_t expectedCIDR2 = expectedCIDR1;
    uint32_t X = iosqs.size();
    LOG_NRM("Issue %d simultaneous deleteIOSQ cmds.", X);
    SharedDeleteIOSQPtr deleteIOSQCmd = SharedDeleteIOSQPtr(new DeleteIOSQ());
    for (uint32_t i = 0; i < X; i++) {
        deleteIOSQCmd->Init(iosqs[i]);
        asq->Send(deleteIOSQCmd, uniqueId);
        expectedCIDR2++;
    }
    asq->Ring();

    LOG_NRM("Perform reap inquiry on ACQ for %d elements", X);
    if (acq->ReapInquiryWaitSpecify(CALC_TIMEOUT_ms(X), X, numCE,
        isrCount) == false) {
        asq->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName, "asq.fail"),
            "Dump Entire ASQ");
        acq->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName, "acq.fail"),
            "Dump Entire ACQ");
        throw FrmwkEx(HERE, "Unable to see CEs for issued cmds #%d", X);
    } else if (numCE != X) {
        asq->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName, "asq.fail"),
            "Dump Entire ASQ");
        acq->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName, "acq.fail"),
            "Dump Entire ACQ");
        throw FrmwkEx(HERE, "The ACQ should only have #%d CE's as a result "
            "of #%d simultaneous cmds but found #%d", X, X, numCE);
    }

    LOG_NRM("Reap ACQ for all %d elements", X);
    SharedMemBufferPtr ceMem = SharedMemBufferPtr(new MemBuffer());
    if ((numReaped = acq->Reap(ceRemain, ceMem, isrCount, X, true)) != X) {
        asq->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName, "asq.fail"),
            "Dump Entire ASQ");
        acq->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName, "acq.fail"),
            "Dump Entire ACQ");
        throw FrmwkEx(HERE, "Unable to reap on ACQ. Reaped #%d of #%d",
            numReaped, X);
    }

    LOG_NRM("Validate all %d CEs.", X);
    vector<uint32_t> uniqueCIDs;
    union CE *ce = (union CE *)ceMem->GetBuffer();
    for (uint32_t i = 0; i < X; i++) {
        LOG_NRM("Validating for CE #%d", (i + 1));
        ProcessCE::Validate(*ce);  // throws upon error
        if ((find(uniqueCIDs.begin(), uniqueCIDs.end(), ce->n.CID) !=
            uniqueCIDs.end()) || (ce->n.CID < expectedCIDR1) ||
            (ce->n.CID >= expectedCIDR2)) {
            asq->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName, "asq.fail"),
                "Dump Entire ASQ");
            acq->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName, "acq.fail"),
                "Dump Entire ACQ");
            throw FrmwkEx(HERE, "Received invalid CID %d", ce->n.CID);
        } else {
            LOG_NRM("Verified CE.CID = %d", ce->n.CID);
            uniqueCIDs.push_back(ce->n.CID);
        }
        ce++;
    }
    expectedCIDR1 = expectedCIDR2; // update the expected CID
}


void
DeleteAllAtOnce_r10b::DelAllIOCQsAndVerify(SharedACQPtr acq, SharedASQPtr asq,
    vector<SharedIOCQPtr> iocqs, uint16_t &expectedCIDR1)
{
    uint32_t isrCount;
    uint32_t ceRemain;
    uint32_t numReaped;
    uint32_t numCE;
    uint16_t uniqueId;

    uint16_t expectedCIDR2 = expectedCIDR1;
    uint32_t X = iocqs.size();
    SharedDeleteIOCQPtr deleteIOCQCmd = SharedDeleteIOCQPtr(new DeleteIOCQ());
    for (uint32_t i = 0; i < X; i++) {
        deleteIOCQCmd->Init(iocqs[i]);
        asq->Send(deleteIOCQCmd, uniqueId);
        expectedCIDR2++;
    }
    asq->Ring();

    LOG_NRM("Perform reap inquiry on ACQ for %d elements", X);
    if (acq->ReapInquiryWaitSpecify(CALC_TIMEOUT_ms(X), X, numCE,
        isrCount) == false) {
        asq->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName, "asq.fail"),
            "Dump Entire ASQ");
        acq->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName, "acq.fail"),
            "Dump Entire ACQ");
        throw FrmwkEx(HERE, "Unable to see CEs for issued cmds #%d", X);
    } else if (numCE != X) {
        asq->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName, "asq.fail"),
            "Dump Entire ASQ");
        acq->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName, "acq.fail"),
            "Dump Entire ACQ");
        throw FrmwkEx(HERE, "The ACQ should only have #%d CE's as a result "
            "of #%d simultaneous cmds but found #%d", X, X, numCE);
    }

    LOG_NRM("Reap ACQ for all %d elements", X);
    SharedMemBufferPtr ceMem = SharedMemBufferPtr(new MemBuffer());
    if ((numReaped = acq->Reap(ceRemain, ceMem, isrCount, X, true)) != X) {
        asq->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName, "asq.fail"),
            "Dump Entire ASQ");
        acq->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName, "acq.fail"),
            "Dump Entire ACQ");
        throw FrmwkEx(HERE, "Unable to reap on ACQ. Reaped #%d of #%d",
            numReaped, X);
    }

    LOG_NRM("Validate all %d CEs.", X);
    vector<uint32_t> uniqueCIDs;
    union CE *ce = (union CE *)ceMem->GetBuffer();
    for (uint32_t i = 0; i < X; i++) {
        LOG_NRM("Validating for CE #%d", (i + 1));
        ProcessCE::Validate(*ce);  // throws upon error
        if ((find(uniqueCIDs.begin(), uniqueCIDs.end(), ce->n.CID) !=
            uniqueCIDs.end()) || (ce->n.CID < expectedCIDR1) ||
            (ce->n.CID >= expectedCIDR2)) {
            asq->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName, "asq.fail"),
                "Dump Entire ASQ");
            acq->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName, "acq.fail"),
                "Dump Entire ACQ");
            throw FrmwkEx(HERE, "Received invalid CID %d", ce->n.CID);
        } else {
            LOG_NRM("Verified CE.CID = %d", ce->n.CID);
            uniqueCIDs.push_back(ce->n.CID);
        }
        ce++;
    }
    expectedCIDR1 = expectedCIDR2; // update the expected CID
}


}   // namespace

