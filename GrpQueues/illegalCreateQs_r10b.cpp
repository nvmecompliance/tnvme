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
#include "illegalCreateQs_r10b.h"
#include "globals.h"
#include "grpDefs.h"

namespace GrpQueues {


IllegalCreateQs_r10b::IllegalCreateQs_r10b(
    string grpName, string testName) :
    Test(grpName, testName, SPECREV_10b)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 5");
    mTestDesc.SetShort(     "Create admin Q's with CreateIOQ cmds.");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Issue CreateIOSQ and CreateIOCQ cmds, specify QID = 0 trying to "
        "create all admin Q's, expect status code = \"Invalid Q ID\". Also "
        "issue a legal Create IOSQ but specify CQID = 0 for the illegal "
        "association, also expect status code = \"Completion Q Invalid\".");
}


IllegalCreateQs_r10b::~IllegalCreateQs_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


IllegalCreateQs_r10b::
IllegalCreateQs_r10b(const IllegalCreateQs_r10b &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


IllegalCreateQs_r10b &
IllegalCreateQs_r10b::operator=(const IllegalCreateQs_r10b &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


Test::RunType
IllegalCreateQs_r10b::RunnableCoreTest(bool preserve)
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
IllegalCreateQs_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) Test CreateResources_r10b has run prior.
     *  \endverbatim
     */

    // Lookup objs which were created in a prior test within group
    SharedACQPtr acq = CAST_TO_ACQ(gRsrcMngr->GetObj(ACQ_GROUP_ID))
    SharedASQPtr asq = CAST_TO_ASQ(gRsrcMngr->GetObj(ASQ_GROUP_ID))

    uint32_t numEntries = 2;
    uint8_t dword;
    uint32_t mask;
    uint32_t value;
    {
        LOG_NRM("Create IOCQ ID #%d but toxify its QID to 0", IOQ_ID);
        SharedIOCQPtr iocq = SharedIOCQPtr(new IOCQ(gDutFd));
        iocq->Init(IOQ_ID, numEntries, true, 0);

        LOG_NRM("Form a Create IOCQ cmd");
        SharedCreateIOCQPtr iocqCmd = SharedCreateIOCQPtr(new CreateIOCQ());
        iocqCmd->Init(iocq);

        dword = 10;
        mask = 0xFFFF;
        value = 0;
        SendToxicCmd(asq, acq, iocqCmd, dword, mask, value, CESTAT_INVALID_QID);
    }
    {
        LOG_NRM("Create IOSQ ID #%d but toxify its QID to 0", IOQ_ID);
        SharedIOSQPtr iosq = SharedIOSQPtr(new IOSQ(gDutFd));
        iosq->Init(IOQ_ID, numEntries, IOQ_ID, 0);

        LOG_NRM("Form a Create IOSQ cmd");
        SharedCreateIOSQPtr iosqCmd = SharedCreateIOSQPtr(new CreateIOSQ());
        iosqCmd->Init(iosq);

        dword = 10;
        mask = 0xFFFF;
        value = 0;
        SendToxicCmd(asq, acq, iosqCmd, dword, mask, value, CESTAT_INVALID_QID);
    }
    {
        LOG_NRM("Create IOSQ ID #%d but wrongly associate to ACQ", IOQ_ID);
        SharedIOSQPtr iosq = SharedIOSQPtr(new IOSQ(gDutFd));
        iosq->Init(IOQ_ID, numEntries, IOQ_ID, 0);

        LOG_NRM("Form a Create IOSQ cmd");
        SharedCreateIOSQPtr iosqCmd = SharedCreateIOSQPtr(new CreateIOSQ());
        iosqCmd->Init(iosq);

        dword = 11;
        mask = 0xFFFF0000;
        value = 0;
        SendToxicCmd(asq, acq, iosqCmd, dword, mask, value, CESTAT_CQ_INVALID);
    }
}


void
IllegalCreateQs_r10b::SendToxicCmd(SharedASQPtr asq, SharedACQPtr acq,
    SharedCmdPtr cmd, uint8_t dw, uint32_t mask, uint32_t val, CEStat status)
{
    uint16_t uniqueId;
    uint32_t isrCnt;
    uint32_t numCE;
    string work;

    LOG_NRM("Send the cmd to hdw via ASQ");
    asq->Send(cmd, uniqueId);

    work = str(boost::format("%s.pure.%d") % cmd->GetName().c_str() % uniqueId);
    asq->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName, "asq"
        + cmd->GetName(), work), "Just B4 modifying, dump ASQ");

    ASQCmdToxify(asq, dw, mask, val);
    work = str(boost::format("%s.toxic.%d") % cmd->GetName().c_str() % uniqueId);
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

    work = str(boost::format("acq.%d") % uniqueId);
    IO::ReapCE(acq, 1, isrCnt, mGrpName, mTestName, work, status);
}


void
IllegalCreateQs_r10b::ASQCmdToxify(SharedASQPtr asq, uint8_t dw, uint32_t mask,
    uint32_t val)
{
    struct nvme_gen_sq asqMetrics = asq->GetQMetrics();
    struct backdoor_inject inject;

    inject.q_id = asq->GetQId();

    if (asqMetrics.tail_ptr_virt)
        inject.cmd_ptr = asqMetrics.tail_ptr_virt - 1;
    else
        inject.cmd_ptr = asq->GetNumEntries() - 1;

    inject.dword = dw;
    inject.value_mask = mask;
    inject.value = val;

    LOG_NRM("Inject toxic parameters: (qId, cmd_ptr, dword, mask, val) = "
        "(%d, %d, %d, %d, %d)", inject.q_id, inject.cmd_ptr, inject.dword,
        inject.value_mask, inject.value);

    asq->SetToxicCmdValue(inject);
}


}   // namespace
