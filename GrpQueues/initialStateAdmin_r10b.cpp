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

#include "initialStateAdmin_r10b.h"
#include "globals.h"
#include "grpDefs.h"
#include "../Utils/kernelAPI.h"
#include "../Utils/io.h"

namespace GrpQueues {


InitialStateAdmin_r10b::InitialStateAdmin_r10b(
    string grpName, string testName) :
    Test(grpName, testName, SPECREV_10b)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 4");
    mTestDesc.SetShort(     "Validate new ASQ/ACQ pointer initial states");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Create an ASQ/ACQ pair; issue identify cmd reap it successfully, "
        "disable the DUT, but not completely, allow the ASQ/ACQ to propagate "
        "through reset. Re-enable the DUT, and re-issue the same identify cmd "
        "and reap it successfully, then validate ASQ tail_ptr = 1, "
        "ACQ head_ptr = 1, and CE.SQHD = 1.");
}


InitialStateAdmin_r10b::~InitialStateAdmin_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


InitialStateAdmin_r10b::
InitialStateAdmin_r10b(const InitialStateAdmin_r10b &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


InitialStateAdmin_r10b &
InitialStateAdmin_r10b::operator=(const InitialStateAdmin_r10b &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


Test::RunType
InitialStateAdmin_r10b::RunnableCoreTest(bool preserve)
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
InitialStateAdmin_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) none
     *  \endverbatim
     */
    if (gCtrlrConfig->SetState(ST_DISABLE_COMPLETELY) == false)
        throw FrmwkEx(HERE);

    LOG_NRM("Create ACQ and ASQ objects which have test life time");
    SharedACQPtr acq = CAST_TO_ACQ(SharedACQPtr(new ACQ(gDutFd)))
    acq->Init(5);

    SharedASQPtr asq = CAST_TO_ASQ(SharedASQPtr(new ASQ(gDutFd)))
    asq->Init(5);

    ValidateInitialStateAdmin(acq, asq);
}

void
InitialStateAdmin_r10b::ValidateInitialStateAdmin(SharedACQPtr acq,
    SharedASQPtr asq)
{
    // Enabled controller and submit identify cmd and reap it, validate head,
    // tail pointers and verify SQ head pointer in completion entry of ACQ.
    // Repeat procedure after disabling and re-enabling ctlr.
    for (uint16_t i = 0; i < 2; i++) {
        LOG_NRM("Validateing initial state admin for #%d times", i);
        gCtrlrConfig->SetCSS(CtrlrConfig::CSS_NVM_CMDSET);
        if (gCtrlrConfig->SetState(ST_ENABLE) == false)
            throw FrmwkEx(HERE);

        SubmitIdentifyCmd(acq, asq);
        VerifyHeadAndTailDoorBells(acq, asq);

        if (gCtrlrConfig->SetState(ST_DISABLE) == false)
            throw FrmwkEx(HERE);
    }
}

void
InitialStateAdmin_r10b::SubmitIdentifyCmd(SharedACQPtr acq, SharedASQPtr asq)
{
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

    LOG_NRM("Send identify cmds to hdw");
    IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1),
        asq, acq, idCmdCap, "InitStateAdmin", true);
}

void
InitialStateAdmin_r10b::VerifyHeadAndTailDoorBells(SharedACQPtr acq,
    SharedASQPtr asq)
{
    struct nvme_gen_cq acqMetrics = acq->GetQMetrics();
    KernelAPI::LogCQMetrics(acqMetrics);
    struct nvme_gen_sq asqMetrics = asq->GetQMetrics();
    KernelAPI::LogSQMetrics(asqMetrics);

    LOG_NRM("Verify ASQ tail_ptr, head_ptr & CE.SQHD position vals equal to 1");
    if (asqMetrics.tail_ptr != 1) {
        asq->Dump(
            FileSystem::PrepDumpFile(mGrpName, mTestName, "asq", "tail_ptr"),
            "SQ Metrics Tail Pointer Inconsistent");
        throw FrmwkEx(HERE, "Expected  ASQ.tail_ptr = 0x0001 but actual "
            "ASQ.tail_ptr  = 0x%04X", asqMetrics.tail_ptr);
    }
    if (acqMetrics.head_ptr != 1) {
        acq->Dump(
            FileSystem::PrepDumpFile(mGrpName, mTestName, "acq", "head_ptr"),
            "CQ Metrics Head Pointer Inconsistent");
        throw FrmwkEx(HERE, "Expected ACQ.head_ptr = 0x0001 but actual "
            "ACQ.head_ptr = 0x%04X", acqMetrics.head_ptr);
    }
    // The CQ's metrics after reaping holds head_ptr plus 1 needed
    union CE ce = acq->PeekCE(acqMetrics.head_ptr - 1);
    if (ce.n.SQHD != 1) {
        acq->Dump(
            FileSystem::PrepDumpFile(mGrpName, mTestName, "acq", "CE.SQHD"),
            "CE SQ Head Pointer Inconsistent");
        throw FrmwkEx(HERE, 
            "Expected CE.SQHD = 0x0001 in ACQ completion entry but actual "
            "CE.SQHD  = 0x%04X", ce.n.SQHD);
    }
}

}   // namespace
