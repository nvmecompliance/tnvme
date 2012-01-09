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

#include "deleteIOQDiscontig_r10b.h"
#include "globals.h"
#include "../Queues/iocq.h"
#include "../Queues/iosq.h"
#include "../Utils/kernelAPI.h"
#include "../Cmds/deleteIOCQ.h"
#include "../Cmds/deleteIOSQ.h"
#include "createACQASQ_r10b.h"
#include "createIOQDiscontigPoll_r10b.h"

#define DEFAULT_CMD_WAIT_ms         2000


DeleteIOQDiscontig_r10b::DeleteIOQDiscontig_r10b(int fd, string grpName,
    string testName) :
    Test(fd, grpName, testName, SPECREV_10b)
{
    // 66 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 7");
    mTestDesc.SetShort(     "Delete discontiguous IOCQ and IOSQ's");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Issue the admin commands Delete I/O SQ and Delete I/Q CQ"
        "to the ASQ and reap the resulting CE's from the ACQ to certify "
        "those the discontiguous IOQ's have been deleted. Dumping driver "
        "metrics before and after the deletion will prove the dnvme/hdw has "
        "removed those Q's");
}


DeleteIOQDiscontig_r10b::~DeleteIOQDiscontig_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


DeleteIOQDiscontig_r10b::
DeleteIOQDiscontig_r10b(const DeleteIOQDiscontig_r10b &other) :
    Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


DeleteIOQDiscontig_r10b &
DeleteIOQDiscontig_r10b::operator=(const DeleteIOQDiscontig_r10b &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


bool
DeleteIOQDiscontig_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) The ASQ & ACQ's have been created by the RsrcMngr for group lifetime
     * 2) All interrupts are disabled.
     * 3) CreateIOQDiscontigPoll_r10b test case has setup the Q's to delete
     * 4) CC.IOCQES and CC.IOSQES are already setup with correct values.
     * \endverbatim
     */

    uint64_t regVal;
    if (gRegisters->Read(CTLSPC_CAP, regVal) == false) {
        LOG_ERR("Unable to determine Q memory requirements");
        throw exception();
    } else if (regVal & CAP_CQR) {
        LOG_NRM("Unable to utilize discontig Q's, DUT requires contig");
        return true;
    }

    KernelAPI::DumpKernelMetrics(mFd,
        FileSystem::PrepLogFile(mGrpName, mTestName, "kmetrics", "before"));

    // Lookup objs which were created in a prior test within group
    SharedASQPtr asq = CAST_TO_ASQ(gRsrcMngr->GetObj(ASQ_GROUP_ID))
    SharedACQPtr acq = CAST_TO_ACQ(gRsrcMngr->GetObj(ACQ_GROUP_ID))

    DeleteIOCQDiscontig(asq, acq);
    DeleteIOSQDiscontig(asq, acq);

    KernelAPI::DumpKernelMetrics(mFd,
        FileSystem::PrepLogFile(mGrpName, mTestName, "kmetrics", "after"));
    return true;
}


void
DeleteIOQDiscontig_r10b::DeleteIOCQDiscontig(SharedASQPtr asq,
    SharedACQPtr acq)
{
    uint16_t numCE;

    LOG_NRM("Lookup IOCQ which was created in a prior test within group");
    SharedIOCQPtr iocq = CAST_TO_IOCQ(
        gRsrcMngr->GetObj(IOCQ_DISCONTIG_POLL_GROUP_ID))

    LOG_NRM("Create a Delete IOCQ cmd to perform the IOCQ deletion");
    SharedDeleteIOCQPtr deleteIOCQCmd =
        SharedDeleteIOCQPtr(new DeleteIOCQ(mFd));
    deleteIOCQCmd->Init(iocq);


    LOG_NRM("Send the Delete IOCQ cmd to hdw");
    asq->Send(deleteIOCQCmd);
    asq->Dump(FileSystem::PrepLogFile(mGrpName, mTestName, "asq",
        "deleteIOCQCmd"),
        "Just B4 ringing SQ0 doorbell, dump entire SQ contents");
    asq->Ring();


    LOG_NRM("Wait for the CE to arrive in ACQ");
    if (acq->ReapInquiryWaitSpecify(DEFAULT_CMD_WAIT_ms, 1, numCE) == false) {
        LOG_ERR("Unable to see completion of Delete IOCQ cmd");
        acq->Dump(
            FileSystem::PrepLogFile(mGrpName, mTestName, "acq","deleteIOCQCmd"),
            "Unable to see any CE's in CQ0, dump entire CQ contents");
        throw exception();
    }
    acq->Dump(FileSystem::PrepLogFile(mGrpName, mTestName, "acq",
        "deleteIOCQCmd"), "Just B4 reaping CQ0, dump entire CQ contents");


    {
        uint16_t ceRemain;
        uint16_t numReaped;

        LOG_NRM("Reaping CE from ACQ, requires memory to hold reaped CE");
        SharedMemBufferPtr ceMemIOCQ = SharedMemBufferPtr(new MemBuffer());
        if ((numReaped = acq->Reap(ceRemain, ceMemIOCQ, numCE, true)) != 1) {
            LOG_ERR("Verified there was 1 CE, but reaping produced %d",
                numReaped);
            throw exception();
        }
        LOG_NRM("The reaped identify CE is...");
        ceMemIOCQ->Log();
    }

    // Not explicitly necessary, but is more clean to free what is not needed
    gRsrcMngr->FreeObj(IOCQ_DISCONTIG_POLL_GROUP_ID);
}


void
DeleteIOQDiscontig_r10b::DeleteIOSQDiscontig(SharedASQPtr asq,
    SharedACQPtr acq)
{
    uint16_t numCE;

    LOG_NRM("Lookup IOSQ which was created in a prior test within group");
    SharedIOSQPtr iosq = CAST_TO_IOSQ(
        gRsrcMngr->GetObj(IOSQ_DISCONTIG_POLL_GROUP_ID))

    LOG_NRM("Create a Delete IOSQ cmd to perform the IOSQ deletion");
    SharedDeleteIOSQPtr deleteIOSQCmd =
        SharedDeleteIOSQPtr(new DeleteIOSQ(mFd));
    deleteIOSQCmd->Init(iosq);


    LOG_NRM("Send the Delete IOSQ cmd to hdw");
    asq->Send(deleteIOSQCmd);
    asq->Dump(FileSystem::PrepLogFile(mGrpName, mTestName, "asq",
        "deleteIOSQCmd"),
        "Just B4 ringing SQ0 doorbell, dump entire SQ contents");
    asq->Ring();


    LOG_NRM("Wait for the CE to arrive in ACQ");
    if (acq->ReapInquiryWaitSpecify(DEFAULT_CMD_WAIT_ms, 1, numCE) == false) {
        LOG_ERR("Unable to see completion of Delete IOSQ cmd");
        acq->Dump(
            FileSystem::PrepLogFile(mGrpName, mTestName, "acq","deleteIOSQCmd"),
            "Unable to see any CE's in CQ0, dump entire CQ contents");
        throw exception();
    }
    acq->Dump(FileSystem::PrepLogFile(mGrpName, mTestName, "acq",
        "deleteIOSQCmd"), "Just B4 reaping CQ0, dump entire CQ contents");


    {
        uint16_t ceRemain;
        uint16_t numReaped;

        LOG_NRM("Reaping CE from ACQ, requires memory to hold reaped CE");
        SharedMemBufferPtr ceMemIOSQ = SharedMemBufferPtr(new MemBuffer());
        if ((numReaped = acq->Reap(ceRemain, ceMemIOSQ, numCE, true)) != 1) {
            LOG_ERR("Verified there was 1 CE, but reaping produced %d",
                numReaped);
            throw exception();
        }
        LOG_NRM("The reaped identify CE is...");
        ceMemIOSQ->Log();
    }

    // Not explicitly necessary, but is more clean to free what is not needed
    gRsrcMngr->FreeObj(IOSQ_DISCONTIG_POLL_GROUP_ID);
}
