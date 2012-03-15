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

#include "manySQtoCQAssoc_r10b.h"
#include "grpDefs.h"
#include "../Utils/io.h"
#include "../Utils/kernelAPI.h"

namespace GrpQueues {

static uint32_t NumEntriesIOQ =    5;

ManySQtoCQAssoc_r10b::ManySQtoCQAssoc_r10b(int fd, string grpName,
    string testName, ErrorRegs errRegs) :
    Test(fd, grpName, testName, SPECREV_10b, errRegs)
{
    // 66 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 4");
    mTestDesc.SetShort(     "Create many IOSQ to IOCQ associations");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Search for 1 of the following namspcs to run test. Find 1st bare "
        "namspc, or find 1st meta namspc, or find 1st E2E namspc. Loop and "
        "create IOSQ's  from 1 to GetFeatures.NumberOfQueues ID (NSQA) and "
        "assoc all with a single IOCQ for each iteration. Issue a single "
        "NVM write cmd sending 1 block and approp supporting meta/E2E if "
        "necessary to the selected namspc at LBA 0, to every IOSQ each "
        "iteration and verify CE success, CE.SQHD=<+1 than it was prior>, "
        "and CE.SQID refers back to the correct IOSQ.");
}


ManySQtoCQAssoc_r10b::~ManySQtoCQAssoc_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


ManySQtoCQAssoc_r10b::
ManySQtoCQAssoc_r10b(const ManySQtoCQAssoc_r10b &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


ManySQtoCQAssoc_r10b &
ManySQtoCQAssoc_r10b::operator=(const ManySQtoCQAssoc_r10b &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


void
ManySQtoCQAssoc_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) Test CreateResources_r10b has run prior.
     *  \endverbatim
     */

    // Lookup objs which were created in a prior test within group
    SharedASQPtr asq = CAST_TO_ASQ(gRsrcMngr->GetObj(ASQ_GROUP_ID))
    SharedACQPtr acq = CAST_TO_ACQ(gRsrcMngr->GetObj(ACQ_GROUP_ID))

    {
        uint64_t maxIOQEntries;
        // Determine the max IOQ entries supported
        if (gRegisters->Read(CTLSPC_CAP, maxIOQEntries) == false)
            throw FrmwkEx("Unable to determine MQES");
        maxIOQEntries &= CAP_MQES;
        maxIOQEntries += 1;      // convert to 1-based
        if (maxIOQEntries < (uint64_t)NumEntriesIOQ) {
            LOG_NRM("Changing number of Q elements from %d to %lld",
                NumEntriesIOQ, (unsigned long long)maxIOQEntries);
            NumEntriesIOQ = maxIOQEntries;
        }
    }

    // Set the controller to initial state.
    if (gCtrlrConfig->SetState(ST_DISABLE) == false)
        throw FrmwkEx();

    gCtrlrConfig->SetCSS(CtrlrConfig::CSS_NVM_CMDSET);
    if (gCtrlrConfig->SetState(ST_ENABLE) == false)
        throw FrmwkEx();

    SharedWritePtr writeCmd = SetWriteCmd();

    // Create one IOCQ for test lifetime.
    gCtrlrConfig->SetIOCQES(gInformative->GetIdentifyCmdCtrlr()->
        GetValue(IDCTRLRCAP_CQES) & 0xf);
    SharedIOCQPtr iocqContig = Queues::CreateIOCQContigToHdw(mGrpName,
        mTestName, DEFAULT_CMD_WAIT_ms, asq, acq, IOQ_ID, NumEntriesIOQ,
        false, IOCQ_CONTIG_GROUP_ID, false, 0, "iocq", true);

    vector<SharedIOSQPtr> iosqContigVector;
    vector<uint32_t> mSQIDToSQHDVector;
    mSQIDToSQHDVector.push_back(USHRT_MAX); // vector position 0 is not used.

    // Create Maximum allowed IOSQs and associate with same IOCQ.
    gCtrlrConfig->SetIOSQES(gInformative->GetIdentifyCmdCtrlr()->
        GetValue(IDCTRLRCAP_SQES) & 0xf);
    for (uint32_t j = 1; j <= gInformative->GetFeaturesNumOfIOSQs(); j++) {
        LOG_NRM("Creating contig IOSQ#%d", j);
        SharedIOSQPtr iosqContig = Queues::CreateIOSQContigToHdw(mGrpName,
            mTestName, DEFAULT_CMD_WAIT_ms, asq, acq, j, NumEntriesIOQ, false,
            IOSQ_CONTIG_GROUP_ID, IOQ_ID, 0, "iosq", true);

        iosqContigVector.push_back(iosqContig);
        mSQIDToSQHDVector.push_back(0);

        for (vector <SharedIOSQPtr>::iterator iosq = iosqContigVector.begin();
            iosq < iosqContigVector.end(); iosq++) {
            (*iosq)->Send(writeCmd);
            (*iosq)->Ring();
            mSQIDToSQHDVector[(*iosq)->GetQId()] =
                ++mSQIDToSQHDVector[(*iosq)->GetQId()] %
                    (*iosq)->GetNumEntries();
        }
        ReapIOCQAndVerifyCE(iocqContig, j, mSQIDToSQHDVector);
    }
}


SharedWritePtr
ManySQtoCQAssoc_r10b::SetWriteCmd()
{
    Informative::Namspc namspcData = gInformative->Get1stBareMetaE2E();
    LOG_NRM("Processing write cmd using namspc id %d", namspcData.id);
    if (namspcData.type != Informative::NS_BARE) {
        LBAFormat lbaFormat = namspcData.idCmdNamspc->GetLBAFormat();
        if (gRsrcMngr->SetMetaAllocSize(lbaFormat.MS) == false)
            throw FrmwkEx();
    }

    LOG_NRM("Create data pattern to write to media");
    SharedMemBufferPtr dataPat = SharedMemBufferPtr(new MemBuffer());
    uint64_t lbaDataSize = namspcData.idCmdNamspc->GetLBADataSize();
    dataPat->Init(lbaDataSize);

    SharedWritePtr writeCmd = SharedWritePtr(new Write());
    send_64b_bitmask prpBitmask = (send_64b_bitmask)(MASK_PRP1_PAGE
        | MASK_PRP2_PAGE | MASK_PRP2_LIST);

    if (namspcData.type == Informative::NS_META) {
        writeCmd->AllocMetaBuffer();
        prpBitmask = (send_64b_bitmask)(prpBitmask | MASK_MPTR);
    } else if (namspcData.type == Informative::NS_E2E) {
        writeCmd->AllocMetaBuffer();
        prpBitmask = (send_64b_bitmask)(prpBitmask | MASK_MPTR);
        LOG_ERR("Deferring E2E namspc work to the future");
        throw FrmwkEx("Need to add CRC's to correlate to buf pattern");
    }

    writeCmd->SetPrpBuffer(prpBitmask, dataPat);
    writeCmd->SetNSID(namspcData.id);
    writeCmd->SetNLB(0);

    return writeCmd;
}


void
ManySQtoCQAssoc_r10b::ReapIOCQAndVerifyCE(SharedIOCQPtr iocq, uint32_t numTil,
    vector<uint32_t> mSQIDToSQHDVector)
{
    uint32_t numCE;
    uint32_t ceRemain;
    uint32_t numReaped;
    uint32_t isrCount;

    // Reap one CE and verify and do for all the CE's in CQ.
    for (uint32_t i = 0; i < numTil; i++) {
        LOG_NRM("Wait for the CE to arrive in IOCQ");
        if (iocq->ReapInquiryWaitSpecify(DEFAULT_CMD_WAIT_ms, 1, numCE,
            isrCount) == false) {

            iocq->Dump(FileSystem::PrepLogFile(mGrpName, mTestName, "missing"),
                "Unable to see completion of cmd");
            throw FrmwkEx();
        } else if (numCE == 0) {
            iocq->Dump(FileSystem::PrepLogFile(mGrpName, mTestName, "missing"),
                "Unable to see completion of cmd");
            throw FrmwkEx("IOCQ should have one new CE for each IOSQ");
        }

        LOG_NRM("The CQ's metrics before reaping holds head_ptr");
        struct nvme_gen_cq iocqMetrics = iocq->GetQMetrics();
        KernelAPI::LogCQMetrics(iocqMetrics);

        LOG_NRM("Reaping CE from IOCQ, requires memory to hold reaped CE");
        SharedMemBufferPtr ceMemIOCQ = SharedMemBufferPtr(new MemBuffer());
        if ((numReaped = iocq->Reap(ceRemain, ceMemIOCQ, isrCount, 1, true))
            != 1) {

            throw FrmwkEx("Requested to reap 1 CE, but reaping produced %d",
                numReaped);
        }

        union CE ce = iocq->PeekCE(iocqMetrics.head_ptr);
        ProcessCE::Validate(ce, CESTAT_SUCCESS);  // throws upon error

        LOG_NRM("Validate CE of IOSQ ID=%d", ce.n.SQID);
        if (ce.n.SQHD != mSQIDToSQHDVector[ce.n.SQID]) {
            throw FrmwkEx("Expected CE.SQHD = 0x%04X in IOCQ CE but actual "
                "CE.SQHD  = 0x%04X", mSQIDToSQHDVector[ce.n.SQID], ce.n.SQHD);
        }

        mSQIDToSQHDVector[ce.n.SQID] = USHRT_MAX; // strike off this sq id
    }

    // Validate all SQIDs submitted have arrived.
    for (uint32_t it = 0; it < mSQIDToSQHDVector.size(); it++) {
        if (mSQIDToSQHDVector[it] != USHRT_MAX) {
            throw FrmwkEx("Never received CE for SQID %d", it);
        }
    }
}

}   // namespace
