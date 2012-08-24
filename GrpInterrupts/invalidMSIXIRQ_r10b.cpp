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

#include <boost/format.hpp>
#include "invalidMSIXIRQ_r10b.h"
#include "globals.h"
#include "grpDefs.h"
#include "createResources_r10b.h"
#include "../Queues/iocq.h"
#include "../Cmds/createIOCQ.h"
#include "../Utils/io.h"

#define IOQ_ID                      1
#define NUM_IOQ_ENTRY               2

namespace GrpInterrupts {


InvalidMSIXIRQ_r10b::InvalidMSIXIRQ_r10b(
    string grpName, string testName) :
    Test(grpName, testName, SPECREV_10b)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 4,7");
    mTestDesc.SetShort(     "Assoc illegal MSI-X IRQ to cause SC=Invalid interrupt vector");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Only allowed to execute test if the DUT supports MSI-X IRQ's by "
        "reporting the MSIXCAP PCI structures. Determine X, the maximum IRQ "
        "vector supported by the DUT by referencing MSIXCAP.MXC.TS. Issue "
        "multiple CreateIOCQ cmds, num elements=2, QID=1, enable IRQ's, and "
        "set DW11.IV by looping from (X+1) to 2047, expect failure.");
}


InvalidMSIXIRQ_r10b::~InvalidMSIXIRQ_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


InvalidMSIXIRQ_r10b::
InvalidMSIXIRQ_r10b(const InvalidMSIXIRQ_r10b &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


InvalidMSIXIRQ_r10b &
InvalidMSIXIRQ_r10b::operator=(const InvalidMSIXIRQ_r10b &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


Test::RunType
InvalidMSIXIRQ_r10b::RunnableCoreTest(bool preserve)
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
InvalidMSIXIRQ_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) Requires test createResources_r10b to execute 1st
     * \endverbatim
     */
    bool capable;
    uint16_t numIrqSupport;


    // Only allowed to execute if DUT supports MSI-X IRQ's
    if (gCtrlrConfig->IsMSIXCapable(capable, numIrqSupport) == false)
        throw FrmwkEx(HERE);
    else if (capable == false) {
        LOG_NRM("DUT does not support MSI-X IRQ's; unable to execute test");
        return;
    }

    if (gCtrlrConfig->SetState(ST_DISABLE) == false)
        throw FrmwkEx(HERE);

    LOG_NRM("Notify DUT we plan on using all IRQ's that it supports");
    if (gCtrlrConfig->SetIrqScheme(INT_MSIX, numIrqSupport) == false) {
        throw FrmwkEx(HERE,
            "Unable to use %d IRQ's, but DUT reports it supports",
            numIrqSupport);
    }

    gCtrlrConfig->SetCSS(CtrlrConfig::CSS_NVM_CMDSET);
    if (gCtrlrConfig->SetState(ST_ENABLE) == false)
        throw FrmwkEx(HERE);

    // Lookup objs which were created in a prior test within group
    SharedASQPtr asq = CAST_TO_ASQ(gRsrcMngr->GetObj(ASQ_GROUP_ID))
    SharedACQPtr acq = CAST_TO_ACQ(gRsrcMngr->GetObj(ACQ_GROUP_ID))

    LOG_NRM("Set legal IOCQ element size");
    gCtrlrConfig->SetIOCQES(gInformative->GetIdentifyCmdCtrlr()->
        GetValue(IDCTRLRCAP_CQES) & 0xf);

    // This test expects the DUT to reject the usage of illegal/unsupported
    // MSIX IRQ vectors, thus they should never be used. In order to safely fool
    // dnvme's safeguards, thus preventing a kernel crash, we need to issue a
    // legal cmd and allow dnvme to do it job. Then just before ringing the
    // doorbell inject a toxic illegal MSIX IRQ vector, the guts of this test.
    // So lets prepare that "legal" cmd here and then corrupt/toxify it later.
    LOG_NRM("Last supported MSIX IRQ vec = %d", (numIrqSupport - 1));
    for (uint32_t i = numIrqSupport; i <= CtrlrConfig::MAX_MSIX_IRQ_VEC; i++) {
        // We must re-init the objects because a failed attempt at creating an
        // IOCQ forces dnvme to deconstruct the entire thing when it is reaped.
        LOG_NRM("Create the IOCQ and the cmd to issue to the DUT");
        SharedIOCQPtr iocq = SharedIOCQPtr(new IOCQ(gDutFd));
        LOG_NRM("Allocate contiguous memory; IOCQ has ID=%d", IOQ_ID);
        iocq->Init(IOQ_ID, NUM_IOQ_ENTRY, true, (numIrqSupport - 1));
        SharedCreateIOCQPtr createIOCQCmd =
            SharedCreateIOCQPtr(new CreateIOCQ());
        createIOCQCmd->Init(iocq);
        SendToxicCmd(asq, acq, createIOCQCmd, i);
    }
}


void
InvalidMSIXIRQ_r10b::SendToxicCmd(SharedASQPtr asq, SharedACQPtr acq,
    SharedCmdPtr cmd, uint16_t illegalIrqVec)
{
    string work;
    uint16_t uniqueId;
    uint32_t isrCnt;
    uint32_t numCE;

    LOG_NRM("Send the cmd to hdw via ASQ with illegal IRQ vec %d",
        illegalIrqVec);
    asq->Send(cmd, uniqueId);
    work = str(boost::format("pure.%d") % illegalIrqVec);
    asq->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName, "asq", work),
        "Just B4 modifying, dump ASQ");

    ASQCmdToxify(asq, illegalIrqVec);
    work = str(boost::format("toxic.%d") % illegalIrqVec);
    asq->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName, "asq", work),
        "Just B4 ringing doorbell, dump ASQ");

    asq->Ring();

    LOG_NRM("Wait for the CE to arrive in CQ %d", acq->GetQId());
    if (acq->ReapInquiryWaitSpecify(CALC_TIMEOUT_ms(1), 1, numCE, isrCnt)
        == false) {
        acq->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName, "acq.fail"),
            "Dump Entire ACQ");
        asq->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName, "asq.fail"),
            "Dump Entire ASQ");
        throw FrmwkEx(HERE, "Unable to see CEs for issued cmd");
    }

    IO::ReapCE(acq, 1, isrCnt, mGrpName, mTestName, "acq",
        CESTAT_INVAL_INT_VEC);
}


void
InvalidMSIXIRQ_r10b::ASQCmdToxify(SharedASQPtr asq, uint16_t illegalIrqVec)
{
    struct backdoor_inject inject;
    struct nvme_gen_sq asqMetrics = asq->GetQMetrics();


    LOG_NRM("Attempt to utilize illegal IRQ vec %d", illegalIrqVec);
    if (asqMetrics.tail_ptr_virt)
        inject.cmd_ptr = (asqMetrics.tail_ptr_virt - 1);
    else
        inject.cmd_ptr = (asq->GetNumEntries() - 1);

    inject.q_id = asq->GetQId();
    inject.dword = 11;
    inject.value_mask = 0xFFFF0000;
    inject.value = (illegalIrqVec << 16);

    LOG_NRM("Inject toxic parameters: (qId, cmd_ptr, dword, mask, val) = "
        "(%d, %d, %d, %d, %d)", inject.q_id, inject.cmd_ptr, inject.dword,
        inject.value_mask, inject.value);

    asq->SetToxicCmdValue(inject);
}


}   // namespace
