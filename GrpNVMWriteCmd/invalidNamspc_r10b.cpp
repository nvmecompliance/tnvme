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
#include "invalidNamspc_r10b.h"
#include "globals.h"
#include "grpDefs.h"
#include "../Queues/iocq.h"
#include "../Queues/iosq.h"
#include "../Queues/ce.h"
#include "../Utils/kernelAPI.h"
#include "../Cmds/write.h"

namespace GrpNVMWriteCmd {


InvalidNamspc_r10b::InvalidNamspc_r10b(
    string grpName, string testName) :
    Test(grpName, testName, SPECREV_10b)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 4,6");
    mTestDesc.SetShort(     "Issue write and cause SC=Invalid Namspc or Format");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Determine Identify.NN and issue write cmd requesting 1 block, to "
        "all namspcs not supported by DUT, expect failure.");
}


InvalidNamspc_r10b::~InvalidNamspc_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


InvalidNamspc_r10b::
InvalidNamspc_r10b(const InvalidNamspc_r10b &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


InvalidNamspc_r10b &
InvalidNamspc_r10b::operator=(const InvalidNamspc_r10b &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


Test::RunType
InvalidNamspc_r10b::RunnableCoreTest(bool preserve)
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
InvalidNamspc_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) Test CreateResources_r10b has run prior.
     * \endverbatim
     */
    uint64_t inc, i;
    string work;
    uint32_t numCE;
    uint32_t isrCountB4;

    LOG_NRM("Lookup objs which were created in a prior test within group");
    SharedIOSQPtr iosq = CAST_TO_IOSQ(gRsrcMngr->GetObj(IOSQ_GROUP_ID));
    SharedIOCQPtr iocq = CAST_TO_IOCQ(gRsrcMngr->GetObj(IOCQ_GROUP_ID));

    if ((numCE = iocq->ReapInquiry(isrCountB4, true)) != 0) {
        iocq->Dump(
            FileSystem::PrepDumpFile(mGrpName, mTestName, "iocq",
            "notEmpty"), "Test assumption have not been met");
        throw FrmwkEx(HERE, "Require 0 CE's within CQ %d, not upheld, found %d",
            iocq->GetQId(), numCE);
    }

    LOG_NRM("Setup write cmd's values that won't change per namspc");
    SharedMemBufferPtr writeMem = SharedMemBufferPtr(new MemBuffer());
    uint64_t lbaDataSize = 512;
    writeMem->Init(lbaDataSize);

    SharedWritePtr writeCmd = SharedWritePtr(new Write());
    send_64b_bitmask prpBitmask = (send_64b_bitmask)
        (MASK_PRP1_PAGE | MASK_PRP2_PAGE | MASK_PRP2_LIST);
    writeCmd->SetPrpBuffer(prpBitmask, writeMem);
    writeCmd->SetNLB(0);    // convert to 0-based value
    writeCmd->SetSLBA(0);

    LOG_NRM("For all namspc's issue cmd to an illegal namspc");
    ConstSharedIdentifyPtr idCtrlrStruct = gInformative->GetIdentifyCmdCtrlr();
    uint32_t nn = (uint32_t)idCtrlrStruct->GetValue(IDCTRLRCAP_NN);
    if (nn == 0 ) {
        throw FrmwkEx(HERE, "Required to support >= 1 namespace");
    }

    for (i = (nn + 1), inc = 1; i <= 0xffffffff; i += (2 * inc), inc += 1327) {

        LOG_NRM("Issue flush cmd with illegal namspc ID=%llu",
            (unsigned long long)i);
        writeCmd->SetNSID(i);
        work = str(boost::format("namspc%d") % i);
        SendCmdToHdw(iosq, iocq, writeCmd, work);
    }
}


void
InvalidNamspc_r10b::SendCmdToHdw(SharedSQPtr sq, SharedCQPtr cq,
    SharedCmdPtr cmd, string qualify)
{
    uint32_t isrCount;
    uint32_t numCE;
    uint32_t ceRemain;
    uint32_t numReaped;
    string work;
    uint16_t uniqueId;


    LOG_NRM("Send the cmd to hdw via SQ %d", sq->GetQId());
    sq->Send(cmd, uniqueId);
    sq->Ring();

    LOG_NRM("Wait for the CE to arrive in CQ %d", cq->GetQId());
    if (cq->ReapInquiryWaitSpecify(CALC_TIMEOUT_ms(1), 1, numCE, isrCount)
        == false) {

        work = str(boost::format(
            "Unable to see any CE's in CQ %d, dump entire CQ") % cq->GetQId());
        cq->Dump(
            FileSystem::PrepDumpFile(mGrpName, mTestName, "cq." +
            cmd->GetName(), qualify), work);
        throw FrmwkEx(HERE, "Unable to see CE for issued cmd");
    } else if (numCE != 1) {
        work = str(boost::format(
            "Unable to see any CE's in CQ %d, dump entire CQ") % cq->GetQId());
        cq->Dump(
            FileSystem::PrepDumpFile(mGrpName, mTestName, "cq." +
            cmd->GetName(), qualify), work);
        throw FrmwkEx(HERE, "1 cmd caused %d CE's to arrive in CQ %d",
            numCE, cq->GetQId());
    }

    LOG_NRM("The CQ's metrics before reaping holds head_ptr");
    struct nvme_gen_cq cqMetrics = cq->GetQMetrics();
    KernelAPI::LogCQMetrics(cqMetrics);

    LOG_NRM("Reaping CE from CQ %d, requires memory to hold CE", cq->GetQId());
    SharedMemBufferPtr ceMem = SharedMemBufferPtr(new MemBuffer());
    if ((numReaped = cq->Reap(ceRemain, ceMem, isrCount, numCE, true)) != 1) {
        work = str(boost::format("Verified CE's exist, desired %d, reaped %d")
            % numCE % numReaped);
        cq->Dump(
            FileSystem::PrepDumpFile(mGrpName, mTestName, "cq.error", qualify),
            work);
        throw FrmwkEx(HERE, work);
    }
    union CE ce = cq->PeekCE(cqMetrics.head_ptr);
    ProcessCE::Validate(ce, CESTAT_INVAL_NAMSPC);  // throws upon error
}


}   // namespace

