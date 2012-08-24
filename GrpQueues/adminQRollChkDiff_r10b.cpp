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

#include "adminQRollChkDiff_r10b.h"
#include "globals.h"
#include "grpDefs.h"
#include "../Utils/kernelAPI.h"
#include "../Utils/io.h"

namespace GrpQueues {


AdminQRollChkDiff_r10b::AdminQRollChkDiff_r10b(
    string grpName, string testName) :
    Test(grpName, testName, SPECREV_10b)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 4");
    mTestDesc.SetShort(     "Validate admin Q doorbell rollover when Q's different size");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Create an ASQ/ACQ pair of size 2 and 4096; however the ASQ starts "
        "with max size while the ACQ starts with min size. Issue "
        "(max Q size plus 2) w/r to the largest queue, use generic identify "
        "cmds to fill and rollover the Q's, reaping each cmd as one is "
        "submitted, verify each CE.SQID is correct while filling. "
        "At the end of reaping all CE's verify ASQ tail_ptr = "
        "<calc_based_on_ASQ_size>, ACQ head_ptr = <calc_based_on_ACQ_size>, "
        "and CE.SQHD = <calc_based_on_ASQ_size>.");
}


AdminQRollChkDiff_r10b::~AdminQRollChkDiff_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


AdminQRollChkDiff_r10b::
AdminQRollChkDiff_r10b(const AdminQRollChkDiff_r10b &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


AdminQRollChkDiff_r10b &
AdminQRollChkDiff_r10b::operator=(const AdminQRollChkDiff_r10b &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


Test::RunType
AdminQRollChkDiff_r10b::RunnableCoreTest(bool preserve)
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
AdminQRollChkDiff_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) none.
     *  \endverbatim
     */
    uint16_t mACQSize = MIN_ADMIN_Q_SIZE; // Start ACQ with min size.
    uint16_t mASQSize = MAX_ADMIN_Q_SIZE;
    uint16_t loopCnt = 0;

    if (gCtrlrConfig->SetState(ST_DISABLE_COMPLETELY) == false)
        throw FrmwkEx(HERE);

    while (1) {
        LOG_NRM("(ASQSize, ACQSize, Loop Cnt) = (%d, %d, %d)",
            mASQSize, mACQSize, loopCnt++);
        // Issue cntl'r disable completely for every iteration.
        if (gCtrlrConfig->SetState(ST_DISABLE_COMPLETELY) == false)
            throw FrmwkEx(HERE);

        // Create ACQ and ASQ objects which have test life time
        SharedACQPtr acq = CAST_TO_ACQ(SharedACQPtr(new ACQ(gDutFd)))
        acq->Init(mACQSize);

        SharedASQPtr asq = CAST_TO_ASQ(SharedASQPtr(new ASQ(gDutFd)))
        asq->Init(mASQSize);

        gCtrlrConfig->SetCSS(CtrlrConfig::CSS_NVM_CMDSET);
        if (gCtrlrConfig->SetState(ST_ENABLE) == false)
            throw FrmwkEx(HERE);

        LOG_NRM("Create identify cmd and assoc some buffer memory");
        SharedIdentifyPtr idCmdCap = SharedIdentifyPtr(new Identify());
        LOG_NRM("Force identify to request ctrlr capabilities struct");
        idCmdCap->SetCNS(true);
        SharedMemBufferPtr idMemCap = SharedMemBufferPtr(new MemBuffer());
        idMemCap->InitAlignment(Identify::IDEAL_DATA_SIZE, PRP_BUFFER_ALIGNMENT,
            false, 0);
        send_64b_bitmask idPrpCap =
            (send_64b_bitmask)(MASK_PRP1_PAGE | MASK_PRP2_PAGE);
        idCmdCap->SetPrpBuffer(idPrpCap, idMemCap);

        LOG_NRM("Submit Idtfy cmds to fill & roll over the Q (Q_SIZE plus 2).");
        for (uint16_t nsubmitTimes = 0; nsubmitTimes <
            MAX(asq->GetNumEntries(), acq->GetNumEntries()) + 2;
            nsubmitTimes++) {
            LOG_NRM("Sending #%d times of %d", (nsubmitTimes + 1),
                (acq->GetNumEntries() + 2));
            IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1),
                asq, acq, idCmdCap, "AdminQRollChkDiff", false);
            VerifyCESQValues(acq, (nsubmitTimes + 1) % asq->GetNumEntries());
        }
        // Verify final Q pointers after all the cmds are submitted and reaped
        VerifyQPointers(acq, asq);

        if (mACQSize >= MAX_ADMIN_Q_SIZE) {
            break;
        }
        // If the test requires incremental sizes then edit here.
        mACQSize = MAX_ADMIN_Q_SIZE; // Assign MAX Q Size of ACQ.
        mASQSize = MIN_ADMIN_Q_SIZE;
    }
}


void
AdminQRollChkDiff_r10b::VerifyCESQValues(SharedACQPtr acq,
    uint16_t expectedVal)
{
    union CE ce;
    struct nvme_gen_cq acqMetrics = acq->GetQMetrics();

    // The CQ's metrics after reaping holds head_ptr plus 1 needed. Also Take
    // Q roll over into account
    if (acqMetrics.head_ptr == 0) {
        ce = acq->PeekCE(acq->GetNumEntries() - 1);
    } else {
        ce = acq->PeekCE(acqMetrics.head_ptr - 1);
    }

    if (ce.n.SQID != 0) {
        acq->Dump(
            FileSystem::PrepDumpFile(mGrpName, mTestName, "acq", "CE.SQID"),
            "CE SQ ID Inconsistent");
        throw FrmwkEx(HERE, "Expected CE.SQID = 0 in ACQ completion entry but actual "
            "CE.SQID  = 0x%04X", ce.n.SQID);
    }

    if (ce.n.SQHD != expectedVal) {
        acq->Dump(
            FileSystem::PrepDumpFile(mGrpName, mTestName, "acq", "CE.SQHD"),
            "CE SQ Head Pointer Inconsistent");
        throw FrmwkEx(HERE, 
            "Expected CE.SQHD = 0x%04X in ACQ completion entry but actual "
            "CE.SQHD  = 0x%04X", expectedVal, ce.n.SQHD);
    }
}


void
AdminQRollChkDiff_r10b::VerifyQPointers(SharedACQPtr acq, SharedASQPtr asq)
{
    union CE ce;
    struct nvme_gen_cq acqMetrics = acq->GetQMetrics();
    struct nvme_gen_sq asqMetrics = asq->GetQMetrics();

    // The CQ's metrics after reaping holds head_ptr plus 1 needed. Also Take
    // Q roll over into account.
    if (acqMetrics.head_ptr == 0) {
        ce = acq->PeekCE(acq->GetNumEntries() - 1);
    } else {
        ce = acq->PeekCE(acqMetrics.head_ptr - 1);
    }

    uint16_t expectedVal = (2 + MAX(acq->GetNumEntries(),
        asq->GetNumEntries())) % acq->GetNumEntries();
    if (acqMetrics.head_ptr != expectedVal) {
        acq->Dump(
            FileSystem::PrepDumpFile(mGrpName, mTestName, "acq", "head_ptr"),
            "CQ Metrics Head Pointer Inconsistent");
        throw FrmwkEx(HERE, "Expected ACQ.head_ptr = 0x%04X but actual "
            "ACQ.head_ptr = 0x%04X", expectedVal, acqMetrics.head_ptr);
    }

    expectedVal = (2 + MAX(acq->GetNumEntries(), asq->GetNumEntries())) %
        asq->GetNumEntries();
    if (asqMetrics.tail_ptr != expectedVal) {
        asq->Dump(
            FileSystem::PrepDumpFile(mGrpName, mTestName, "asq", "tail_ptr"),
            "SQ Metrics Tail Pointer Inconsistent");
        throw FrmwkEx(HERE, "Expected  ASQ.tail_ptr = 0x%04X but actual "
            "ASQ.tail_ptr  = 0x%04X", expectedVal, asqMetrics.tail_ptr);
    }

    if (ce.n.SQHD != expectedVal) {
        acq->Dump(
            FileSystem::PrepDumpFile(mGrpName, mTestName, "acq", "CE.SQHD"),
            "CE SQ Head Pointer Inconsistent");
        throw FrmwkEx(HERE, "Expected CE.SQHD = 0x%04X in ACQ completion entry but actual "
            "CE.SQHD  = 0x%04X", expectedVal, ce.n.SQHD);
    }
}

}   // namespaceS
