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

#include "dumpGetFeatures_r10b.h"
#include "globals.h"
#include "grpDefs.h"
#include "../Cmds/getFeatures.h"
#include "../Utils/kernelAPI.h"
#include "../Queues/ce.h"

namespace GrpInformative {


DumpGetFeatures_r10b::DumpGetFeatures_r10b(int fd, string grpName,
    string testName, ErrorRegs errRegs) :
    Test(fd, grpName, testName, SPECREV_10b, errRegs)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 7");
    mTestDesc.SetShort(     "Issue the get features cmd");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Issue the get features cmd to the ASQ. Request various feature "
        "identifiers which are deemed important enough to retrieve for all "
        "tests to view easily");
}


DumpGetFeatures_r10b::~DumpGetFeatures_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


DumpGetFeatures_r10b::
DumpGetFeatures_r10b(const DumpGetFeatures_r10b &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


DumpGetFeatures_r10b &
DumpGetFeatures_r10b::operator=(const DumpGetFeatures_r10b &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


void
DumpGetFeatures_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) Test CreateResources_r10b has run prior.
     *  \endverbatim
     */
    uint32_t isrCount;

    // Lookup objs which were created in a prior test within group
    SharedASQPtr asq = CAST_TO_ASQ(gRsrcMngr->GetObj(ASQ_GROUP_ID))
    SharedACQPtr acq = CAST_TO_ACQ(gRsrcMngr->GetObj(ACQ_GROUP_ID))

    // Assuming the cmd we issue will result in only a single CE
    if (acq->ReapInquiry(isrCount, true) != 0) {
        acq->Dump(
            FileSystem::PrepDumpFile(mGrpName, mTestName, "acq",
            "notEmpty"), "Test assumption have not been met");
        throw FrmwkEx(HERE, 
            "The ACQ should not have any CE's waiting before testing");
    }

    SendGetFeaturesNumOfQueues(asq, acq);
}


void
DumpGetFeatures_r10b::SendGetFeaturesNumOfQueues(SharedASQPtr asq,
    SharedACQPtr acq)
{
    uint32_t numCE;
    uint32_t isrCount;
    uint16_t uniqueId;


    LOG_NRM("Create get features");
    SharedGetFeaturesPtr gfNumQ = SharedGetFeaturesPtr(new GetFeatures());
    LOG_NRM("Force get features to request number of queues");
    gfNumQ->SetFID(GetFeatures::FID_NUM_QUEUES);
    gfNumQ->Dump(
        FileSystem::PrepDumpFile(mGrpName, mTestName, "GetFeat", "NumOfQueue"),
        "The get features number of queues cmd");


    LOG_NRM("Send the get features cmd to hdw");
    asq->Send(gfNumQ, uniqueId);
    asq->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName, "asq",
        "GetFeat.NumOfQueue"),
        "Just B4 ringing SQ0 doorbell, dump entire SQ contents");
    asq->Ring();


    LOG_NRM("Wait for the CE to arrive in ACQ");
    if (acq->ReapInquiryWaitSpecify(DEFAULT_CMD_WAIT_ms, 1, numCE, isrCount)
        == false) {

        acq->Dump(
            FileSystem::PrepDumpFile(mGrpName, mTestName, "acq",
            "GetFeat.NumOfQueue"),
            "Unable to see any CE's in CQ0, dump entire CQ contents");
        throw FrmwkEx(HERE, "Unable to see completion of get features cmd");
    } else if (numCE != 1) {
        acq->Dump(
            FileSystem::PrepDumpFile(mGrpName, mTestName, "acq",
            "GetFeat.NumOfQueue"),
            "Unable to see any CE's in CQ0, dump entire CQ contents");
        LOG_ERR("The ACQ should only have 1 CE as a result of a cmd");
        throw FrmwkEx(HERE);
    }
    acq->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName, "acq",
        "GetFeat.NumOfQueue"),
        "Just B4 reaping CQ0, dump entire CQ contents");

    {
        uint32_t ceRemain;
        uint32_t numReaped;


        LOG_NRM("The CQ's metrics before reaping holds head_ptr needed");
        struct nvme_gen_cq acqMetrics = acq->GetQMetrics();
        KernelAPI::LogCQMetrics(acqMetrics);

        LOG_NRM("Reaping CE from ACQ, requires memory to hold reaped CE");
        SharedMemBufferPtr ceMemCap = SharedMemBufferPtr(new MemBuffer());
        if ((numReaped = acq->Reap(ceRemain, ceMemCap, isrCount, numCE, true))
            != 1) {

            throw FrmwkEx(HERE,
                "Verified there was 1 CE, but reaping produced %d", numReaped);
        }
        LOG_NRM("The reaped CE is...");
        acq->LogCE(acqMetrics.head_ptr);
        acq->DumpCE(acqMetrics.head_ptr, FileSystem::PrepDumpFile
            (mGrpName, mTestName, "CE", "GetFeat.NumOfQueue"),
            "The CE of the Get Features cmd; Number of Q's feature ID:");

        union CE ce = acq->PeekCE(acqMetrics.head_ptr);
        ProcessCE::Validate(ce);  // throws upon error

        // Update the Informative singleton for all tests to see and use
        gInformative->SetGetFeaturesNumberOfQueues(ce.t.dw0);
    }
}

}   // namespace
