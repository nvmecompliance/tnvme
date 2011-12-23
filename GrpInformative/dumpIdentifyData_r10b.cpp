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

#include <unistd.h>
#include "dumpIdentifyData_r10b.h"
#include "globals.h"
#include "createACQASQ_r10b.h"
#include "../Cmds/identify.h"
#include "../Utils/kernelAPI.h"

#define DEFAULT_CMD_WAIT_ms         2000


DumpIdentifyData_r10b::DumpIdentifyData_r10b(int fd, string grpName,
    string testName) :
    Test(fd, grpName, testName, SPECREV_10b)
{
    // 66 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 7");
    mTestDesc.SetShort(     "Issue the identify cmd");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Issue the identify cmd to the ASQ. Request both controller and all "
        "namespace pages. Dump this data to the log directory");
}


DumpIdentifyData_r10b::~DumpIdentifyData_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


DumpIdentifyData_r10b::
DumpIdentifyData_r10b(const DumpIdentifyData_r10b &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


DumpIdentifyData_r10b &
DumpIdentifyData_r10b::operator=(const DumpIdentifyData_r10b &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


bool
DumpIdentifyData_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) The ASQ & ACQ's have been created by the RsrcMngr for group lifetime
     * 2) All interrupts are disabled.
     *  \endverbatim
     */

    KernelAPI::DumpKernelMetrics(mFd,
        FileSystem::PrepLogFile(mGrpName, mTestName, "kmetrics", "before"));

    // Lookup objs which were created in a prior test within group
    SharedASQPtr asq = CAST_TO_ASQ(gRsrcMngr->GetObj(ASQ_GROUP_ID))
    SharedACQPtr acq = CAST_TO_ACQ(gRsrcMngr->GetObj(ACQ_GROUP_ID))

    // Assuming the cmd we issue will result in only a single CE
    if (acq->ReapInquiry() != 0) {
        LOG_ERR("The ACQ should not have any CE's waiting before testing");
        throw exception();
    }

    SendIdentifyCtrlrStruct(asq, acq);
    SendIdentifyNamespaceStruct(asq, acq);

    KernelAPI::DumpKernelMetrics(mFd,
        FileSystem::PrepLogFile(mGrpName, mTestName, "kmetrics", "after"));
    return true;
}


void
DumpIdentifyData_r10b::SendIdentifyCtrlrStruct(SharedASQPtr asq,
    SharedACQPtr acq)
{
    uint16_t numCE;


    LOG_NRM("Create 1st identify cmd and assoc some buffer memory");
    SharedIdentifyPtr idCmdCap = SharedIdentifyPtr(new Identify(mFd));
    LOG_NRM("Force identify to request ctrlr capabilities struct");
    idCmdCap->SetCNS(true);
    SharedMemBufferPtr idMemCap = SharedMemBufferPtr(new MemBuffer());
    idMemCap->InitAlignment(Identify::IDEAL_DATA_SIZE, sizeof(uint64_t),
        true, 0);
    send_64b_bitmask idPrpCap =
        (send_64b_bitmask)(MASK_PRP1_PAGE | MASK_PRP2_PAGE);
    idCmdCap->SetPrpBuffer(idPrpCap, idMemCap);


    LOG_NRM("Send the 1st identify cmd to hdw");
    asq->Send(idCmdCap);
    asq->Dump(FileSystem::PrepLogFile(mGrpName, mTestName, "asq", "idCmdCap"),
        "Just B4 ringing SQ0 doorbell, dump entire SQ contents");
    asq->Ring();


    LOG_NRM("Wait for the CE to arrive in ACQ");
    if (acq->ReapInquiryWaitSpecify(DEFAULT_CMD_WAIT_ms, 1, numCE) == false) {
        LOG_ERR("Unable to see completion of identify cmd");
        acq->Dump(
            FileSystem::PrepLogFile(mGrpName, mTestName, "acq", "idCmdCap"),
            "Unable to see any CE's in CQ0, dump entire CQ contents");
        throw exception();
    } else if (numCE != 1) {
        LOG_ERR("The ACQ should only have 1 CE as a result of a cmd");
        throw exception();
    }
    acq->Dump(FileSystem::PrepLogFile(mGrpName, mTestName, "acq", "idCmdCap"),
        "Just B4 reaping CQ0, dump entire CQ contents");

    {
        uint16_t ceRemain;
        uint16_t numReaped;

        LOG_NRM("The CQ's metrics before reaping holds head_ptr needed");
        struct nvme_gen_cq acqMetrics = acq->GetQMetrics();
        KernelAPI::LogCQMetrics(acqMetrics);

        LOG_NRM("Reaping CE from ACQ, requires memory to hold reaped CE");
        SharedMemBufferPtr ceMemCap = SharedMemBufferPtr(new MemBuffer());
        if ((numReaped = acq->Reap(ceRemain, ceMemCap, numCE, true)) != 1) {
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

        idCmdCap->Dump(FileSystem::PrepLogFile(mGrpName, mTestName, "idCmdCap"),
            "The complete admin cmd identify ctgrlr data structure decoded:");

        // Update the Informative singleton for all tests to see and use
        gInformative->SetIdentifyCmdCapabilities(idCmdCap);
    }
}


void
DumpIdentifyData_r10b::SendIdentifyNamespaceStruct(SharedASQPtr asq,
    SharedACQPtr acq)
{
    uint16_t numCE;
    uint64_t numNamSpc;
    char qualifier[20];


    ConstSharedIdentifyPtr idCmdCap =
        gInformative->GetIdentifyCmdCapabilities();
    if ((numNamSpc = idCmdCap->GetValue(IDCTRLRCAP_NN)) == 0) {
        LOG_ERR("Required to support >= 1 namespace");
        throw exception();
    }

    for (uint64_t namSpc = 1; namSpc <= numNamSpc; namSpc++) {
        snprintf(qualifier, sizeof(qualifier), "idCmdNamSpc-%llu",
            (long long unsigned int)namSpc);

        LOG_NRM("Create identify cmd #%llu & assoc some buffer memory",
            (long long unsigned int)(namSpc+1));
        SharedIdentifyPtr idCmdNamSpc = SharedIdentifyPtr(new Identify(mFd));
        LOG_NRM("Force identify to request namespace struct #%llu",
            (long long unsigned int)namSpc);
        idCmdNamSpc->SetCNS(false);
        idCmdNamSpc->SetNSID(namSpc);
        SharedMemBufferPtr idMemNamSpc = SharedMemBufferPtr(new MemBuffer());
        idMemNamSpc->InitAlignment(Identify::IDEAL_DATA_SIZE, sizeof(uint64_t),
            true, 0);
        send_64b_bitmask idPrpNamSpc =
            (send_64b_bitmask)(MASK_PRP1_PAGE | MASK_PRP2_PAGE);
        idCmdNamSpc->SetPrpBuffer(idPrpNamSpc, idMemNamSpc);


        LOG_NRM("Send the identify cmd to hdw");
        asq->Send(idCmdNamSpc);
        asq->Dump(
            FileSystem::PrepLogFile(mGrpName, mTestName, "asq", qualifier),
            "Just B4 ringing SQ0 doorbell, dump entire SQ contents");
        asq->Ring();


        LOG_NRM("Wait for the CE to arrive in ACQ");
        if (acq->ReapInquiryWaitSpecify(DEFAULT_CMD_WAIT_ms, 1, numCE)
            == false) {

            LOG_ERR("Unable to see completion of identify cmd");
            acq->Dump(
                FileSystem::PrepLogFile(mGrpName, mTestName, "acq", qualifier),
                "Unable to see any CE's in CQ0, dump entire CQ contents");
            throw exception();
        } else if (numCE != 1) {
            LOG_ERR("The ACQ should only have 1 CE as a result of a cmd");
            throw exception();
        }
        acq->Dump(
            FileSystem::PrepLogFile(mGrpName, mTestName, "acq", qualifier),
            "Just B4 reaping CQ0, dump entire CQ contents");


        {
            uint16_t ceRemain;
            uint16_t numReaped;

            LOG_NRM("The CQ's metrics before reaping holds head_ptr needed");
            struct nvme_gen_cq acqMetrics = acq->GetQMetrics();
            KernelAPI::LogCQMetrics(acqMetrics);

            LOG_NRM("Reaping CE from ACQ, requires memory to hold reaped CE");
            SharedMemBufferPtr ceMemNamSpc =
                SharedMemBufferPtr(new MemBuffer());
            if ((numReaped =
                acq->Reap(ceRemain, ceMemNamSpc, numCE, true)) != 1) {

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

            idCmdNamSpc->Dump(
                FileSystem::PrepLogFile(mGrpName, mTestName, qualifier),
                "The complete admin cmd identify namespace structure decoded:");

            gInformative->SetIdentifyCmdNamespace(idCmdNamSpc);
        }
    }
}
