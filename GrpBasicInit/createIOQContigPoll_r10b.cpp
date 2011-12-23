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
#include "../Queues/iocq.h"
#include "../Queues/iosq.h"
#include "../Utils/kernelAPI.h"
#include "../Cmds/createIOCQ.h"
#include "../Cmds/createIOSQ.h"
#include "createACQASQ_r10b.h"

#define IOQ_ID                      1
#define DEFAULT_CMD_WAIT_ms         2000
#define IOQ_NUM_ENTRIES             5


CreateIOQContigPoll_r10b::CreateIOQContigPoll_r10b(int fd, string grpName,
    string testName) :
    Test(fd, grpName, testName, SPECREV_10b)
{
    // 66 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 7");
    mTestDesc.SetShort(     "Create contiguous IOCQ and IOSQ's");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Issue the admin commands Create contiguous I/O SQ and Create I/Q "
        "CQ to the ASQ and reap the resulting CE's from the ACQ to certify "
        "those Q's have been created.");
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


bool
CreateIOQContigPoll_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) The ASQ & ACQ's have been created by the RsrcMngr for group lifetime
     * 2) All interrupts are disabled.
     * 3) Empty ASQ & ACQ's
     * \endverbatim
     */

    KernelAPI::DumpKernelMetrics(mFd,
        FileSystem::PrepLogFile(mGrpName, mTestName, "kmetrics", "before"));

    // Lookup objs which were created in a prior test within group
    SharedASQPtr asq = CAST_TO_ASQ(gRsrcMngr->GetObj(ASQ_GROUP_ID))
    SharedACQPtr acq = CAST_TO_ACQ(gRsrcMngr->GetObj(ACQ_GROUP_ID))

    // Assuming a clean admin Q for our operations
    if (acq->ReapInquiry() != 0) {
        LOG_ERR("The ACQ should not have any CE's waiting before testing");
        throw exception();
    }

    CreateIOCQContigPoll(asq, acq);
    CreateIOSQContigPoll(asq, acq);

    KernelAPI::DumpKernelMetrics(mFd,
        FileSystem::PrepLogFile(mGrpName, mTestName, "kmetrics", "after"));
    return true;
}


void
CreateIOQContigPoll_r10b::CreateIOCQContigPoll(SharedASQPtr asq,
    SharedACQPtr acq)
{
    uint16_t numCE;


    gCtrlrConfig->SetIOCQES(IOCQ::COMMON_ELEMENT_SIZE_PWR_OF_2);
    LOG_NRM("Create an IOCQ object with group lifetime");
    SharedIOCQPtr iocq = CAST_TO_IOCQ(
        gRsrcMngr->AllocObj(Trackable::OBJ_IOCQ, IOCQ_CONTIG_POLL_GROUP_ID));
    LOG_NRM("Allocate contiguous memory, ID=%d for the IOCQ", IOQ_ID);
    iocq->Init(IOQ_ID, IOQ_NUM_ENTRIES, false, 0);


    LOG_NRM("Create a Create IOCQ cmd to perform the IOCQ creation");
    SharedCreateIOCQPtr createIOCQCmd =
        SharedCreateIOCQPtr(new CreateIOCQ(mFd));
    createIOCQCmd->Init(iocq);


    LOG_NRM("Send the Create IOCQ cmd to hdw");
    asq->Send(createIOCQCmd);
    asq->Dump(FileSystem::PrepLogFile(mGrpName, mTestName, "asq",
        "createIOCQCmd"),
        "Just B4 ringing SQ0 doorbell, dump entire SQ contents");
    asq->Ring();


    LOG_NRM("Wait for the CE to arrive in ACQ");
    if (acq->ReapInquiryWaitSpecify(DEFAULT_CMD_WAIT_ms, 1, numCE) == false) {
        LOG_ERR("Unable to see completion of Create IOCQ cmd");
        acq->Dump(
            FileSystem::PrepLogFile(mGrpName, mTestName, "acq","createIOCQCmd"),
            "Unable to see any CE's in CQ0, dump entire CQ contents");
        throw exception();
    } else if (numCE != 1) {
        LOG_ERR("The ACQ should only have 1 CE as a result of a cmd");
        throw exception();
    }
    acq->Dump(FileSystem::PrepLogFile(mGrpName, mTestName, "acq",
        "createIOCQCmd"), "Just B4 reaping CQ0, dump entire CQ contents");


    {
        uint16_t ceRemain;
        uint16_t numReaped;

        LOG_NRM("The CQ's metrics before reaping holds head_ptr needed");
        struct nvme_gen_cq acqMetrics = acq->GetQMetrics();
        KernelAPI::LogCQMetrics(acqMetrics);

        LOG_NRM("Reaping CE from ACQ, requires memory to hold reaped CE");
        SharedMemBufferPtr ceMemIOCQ = SharedMemBufferPtr(new MemBuffer());
        if ((numReaped = acq->Reap(ceRemain, ceMemIOCQ, numCE, true)) != 1) {
            LOG_ERR("Verified there was 1 CE, but reaping produced %d",
                numReaped);
            throw exception();
        }
        LOG_NRM("The reaped get features CE is...");
        acq->LogCE(acqMetrics.head_ptr);

        union CE ce = acq->PeekCE(acqMetrics.head_ptr);
        if (ce.n.status != 0) {
            LOG_ERR("CE shows cmd failed: status = 0x%02X", ce.n.status);
            throw exception();
        }
        LOG_NRM("The CE indicates a successful completion");

        // The PRP payload is in fact the memory backing the Q
        createIOCQCmd->Dump(
            FileSystem::PrepLogFile(mGrpName, mTestName, "IOCQ"),
            "The complete IOCQ contents, not yet used, but is created.");
    }
}


void
CreateIOQContigPoll_r10b::CreateIOSQContigPoll(SharedASQPtr asq,
    SharedACQPtr acq)
{
    uint16_t numCE;

    gCtrlrConfig->SetIOSQES(IOSQ::COMMON_ELEMENT_SIZE_PWR_OF_2);

    LOG_NRM("Create an IOSQ object with group lifetime");
    SharedIOSQPtr iosq = CAST_TO_IOSQ(
        gRsrcMngr->AllocObj(Trackable::OBJ_IOSQ, IOSQ_CONTIG_POLL_GROUP_ID));
    LOG_NRM("Allocate contiguous memory, ID=%d for the IOSQ", IOQ_ID);
    iosq->Init(IOQ_ID, IOQ_NUM_ENTRIES, false, 0);


    LOG_NRM("Create a Create IOSQ cmd to perform the IOSQ creation");
    SharedCreateIOSQPtr createIOSQCmd =
        SharedCreateIOSQPtr(new CreateIOSQ(mFd));
    createIOSQCmd->Init(iosq);


    LOG_NRM("Send the Create IOSQ cmd to hdw");
    asq->Send(createIOSQCmd);
    asq->Dump(FileSystem::PrepLogFile(mGrpName, mTestName, "asq",
        "createIOSQCmd"),
        "Just B4 ringing SQ0 doorbell, dump entire SQ contents");
    asq->Ring();


    LOG_NRM("Wait for the CE to arrive in ACQ");
    if (acq->ReapInquiryWaitSpecify(DEFAULT_CMD_WAIT_ms, 1, numCE) == false) {
        LOG_ERR("Unable to see completion of Create IOSQ cmd");
        acq->Dump(
            FileSystem::PrepLogFile(mGrpName, mTestName, "acq","createIOSQCmd"),
            "Unable to see any CE's in CQ0, dump entire CQ contents");
        throw exception();
    } else if (numCE != 1) {
        LOG_ERR("The ACQ should only have 1 CE as a result of a cmd");
        throw exception();
    }
    acq->Dump(FileSystem::PrepLogFile(mGrpName, mTestName, "acq",
        "createIOSQCmd"), "Just B4 reaping CQ0, dump entire CQ contents");


    {
        uint16_t ceRemain;
        uint16_t numReaped;

        LOG_NRM("The CQ's metrics before reaping holds head_ptr needed");
        struct nvme_gen_cq acqMetrics = acq->GetQMetrics();
        KernelAPI::LogCQMetrics(acqMetrics);

        LOG_NRM("Reaping CE from ACQ, requires memory to hold reaped CE");
        SharedMemBufferPtr ceMemIOSQ = SharedMemBufferPtr(new MemBuffer());
        if ((numReaped = acq->Reap(ceRemain, ceMemIOSQ, numCE, true)) != 1) {
            LOG_ERR("Verified there was 1 CE, but reaping produced %d",
                numReaped);
            throw exception();
        }
        LOG_NRM("The reaped get features CE is...");
        acq->LogCE(acqMetrics.head_ptr);

        union CE ce = acq->PeekCE(acqMetrics.head_ptr);
        if (ce.n.status != 0) {
            LOG_ERR("CE shows cmd failed: status = 0x%02X", ce.n.status);
            throw exception();
        }
        LOG_NRM("The CE indicates a successful completion");

        // The PRP payload is in fact the memory backing the Q
        createIOSQCmd->Dump(
            FileSystem::PrepLogFile(mGrpName, mTestName, "IOSQ"),
            "The complete IOSQ contents, not yet used, but is created.");
    }
}
