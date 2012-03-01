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

#include "adminQRollChkSame_r10b.h"
#include "globals.h"
#include "grpDefs.h"
#include "../Utils/kernelAPI.h"
#include "../Utils/io.h"

namespace GrpQueues {

#define MAX_ADMIN_Q_SIZE            4096


AdminQRollChkSame_r10b::AdminQRollChkSame_r10b(int fd, string grpName,
    string testName, ErrorRegs errRegs) :
    Test(fd, grpName, testName, SPECREV_10b, errRegs)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 4");
    mTestDesc.SetShort(     "Validate admin Q doorbell rollover when Q's same size");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Create an ASQ/ACQ pair of size 2 and 4096. Issue "
        "(max Q size plus 2) generic identify cmds to fill and "
        "rollover the Q's, reaping each cmd as one is submitted, "
        "verify each CE.SQID is correct while filling. Verify ASQ "
        "tail_ptr = 2, ACQ head_ptr = 2, and CE.SQHD = 2.");
}


AdminQRollChkSame_r10b::~AdminQRollChkSame_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


AdminQRollChkSame_r10b::
AdminQRollChkSame_r10b(const AdminQRollChkSame_r10b &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


AdminQRollChkSame_r10b &
AdminQRollChkSame_r10b::operator=(const AdminQRollChkSame_r10b &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


bool
AdminQRollChkSame_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) none.
     *  \endverbatim
     */
    uint16_t adminQSize = 2; // minimum Admin Q size.
    uint16_t loopCnt = 0;

    if (gCtrlrConfig->SetState(ST_DISABLE_COMPLETELY) == false)
        throw exception();

    while (1) {
        LOG_NRM("(ASQSize, ACQSize, Loop Cnt) = (%d, %d, %d)",
            adminQSize, adminQSize, loopCnt++);
        // Issue cntl'r disable completely for every iteration.
        if (gCtrlrConfig->SetState(ST_DISABLE_COMPLETELY) == false)
            throw exception();

        // Create ACQ and ASQ objects which have test life time
        SharedACQPtr acq = CAST_TO_ACQ(SharedACQPtr(new ACQ(mFd)))
        acq->Init(adminQSize);

        SharedASQPtr asq = CAST_TO_ASQ(SharedASQPtr(new ASQ(mFd)))
        asq->Init(adminQSize);

        gCtrlrConfig->SetCSS(CtrlrConfig::CSS_NVM_CMDSET);
        if (gCtrlrConfig->SetState(ST_ENABLE) == false)
            throw exception();

        LOG_NRM("Create identify cmd and assoc some buffer memory");
        SharedIdentifyPtr idCmdCap = SharedIdentifyPtr(new Identify(mFd));
        LOG_NRM("Force identify to request ctrlr capabilities struct");
        idCmdCap->SetCNS(true);
        SharedMemBufferPtr idMemCap = SharedMemBufferPtr(new MemBuffer());
        idMemCap->InitAlignment(Identify::IDEAL_DATA_SIZE, sizeof(uint64_t),
            true, 0);
        send_64b_bitmask idPrpCap =
            (send_64b_bitmask)(MASK_PRP1_PAGE | MASK_PRP2_PAGE);
        idCmdCap->SetPrpBuffer(idPrpCap, idMemCap);

        // Sumbit Identify cmds to fill and roll over the Q (Q_SIZE plus 2).
        for (uint16_t nsubmitTimes = 0;  nsubmitTimes < (asq->GetNumEntries()
            + 2); nsubmitTimes++) {
            IO::SendCmdToHdw(mGrpName, mTestName, DEFAULT_CMD_WAIT_ms,
                asq, acq, idCmdCap, "AdminQRollChkSame", false);
            VerifyCESQValues(acq, (nsubmitTimes + 1) % acq->GetNumEntries());
        }
        // Verify final Q pointers after all the cmds are submitted and reaped
        VerifyQPointers(acq, asq);

        if (adminQSize >= MAX_ADMIN_Q_SIZE) {
            break;
        }
        adminQSize = MAX_ADMIN_Q_SIZE;
    }
    return true;
}

void
AdminQRollChkSame_r10b::VerifyCESQValues(SharedACQPtr acq,
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
        LOG_ERR("Expected CE.SQID = 0 in ACQ completion entry but actual "
            "CE.SQID  = 0x%04X", ce.n.SQID);
        acq->Dump(
            FileSystem::PrepLogFile(mGrpName, mTestName, "acq", "CE.SQID"),
            "CE SQ ID Inconsistent");
        throw exception();
    }

    if (ce.n.SQHD != expectedVal) {
        LOG_ERR("Expected CE.SQHD = 0x%04X in ACQ completion entry but actual "
            "CE.SQHD  = 0x%04X", expectedVal, ce.n.SQHD);
        acq->Dump(
            FileSystem::PrepLogFile(mGrpName, mTestName, "acq", "CE.SQHD"),
            "CE SQ Head Pointer Inconsistent");
        throw exception();
    }
}

void
AdminQRollChkSame_r10b::VerifyQPointers(SharedACQPtr acq, SharedASQPtr asq)
{
    uint16_t expectedVal = 2;
    union CE ce;
    struct nvme_gen_cq acqMetrics = acq->GetQMetrics();
    struct nvme_gen_sq asqMetrics = asq->GetQMetrics();

    // The CQ's metrics after reaping holds head_ptr plus 1 needed. Also Take
    // Q roll over into account.
    if (acqMetrics.head_ptr == 0) {
        ce = acq->PeekCE(acq->GetNumEntries() - 1);
        expectedVal = 0;
    } else {
        ce = acq->PeekCE(acqMetrics.head_ptr - 1);
    }

    if (asqMetrics.tail_ptr != expectedVal) {
        LOG_ERR("Expected  ASQ.tail_ptr = 0x%04X but actual "
            "ASQ.tail_ptr  = 0x%04X", expectedVal, asqMetrics.tail_ptr);
        asq->Dump(
            FileSystem::PrepLogFile(mGrpName, mTestName, "asq", "tail_ptr"),
            "SQ Metrics Tail Pointer Inconsistent");
        throw exception();
    }

    if (acqMetrics.head_ptr != expectedVal) {
        LOG_ERR("Expected ACQ.head_ptr = 0x%04X but actual "
            "ACQ.head_ptr = 0x%04X", expectedVal, acqMetrics.head_ptr);
        acq->Dump(
            FileSystem::PrepLogFile(mGrpName, mTestName, "acq", "head_ptr"),
            "CQ Metrics Head Pointer Inconsistent");
        throw exception();
    }

    if (ce.n.SQHD != expectedVal) {
        LOG_ERR("Expected CE.SQHD = 0x%04X in ACQ completion entry but actual "
            "CE.SQHD  = 0x%04X", expectedVal, ce.n.SQHD);
        acq->Dump(
            FileSystem::PrepLogFile(mGrpName, mTestName, "acq", "CE.SQHD"),
            "CE SQ Head Pointer Inconsistent");
        throw exception();
    }
}

}   // namespace
