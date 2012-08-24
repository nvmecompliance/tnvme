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

static uint32_t NumEntriesIOQ = 5;


ManySQtoCQAssoc_r10b::ManySQtoCQAssoc_r10b(
    string grpName, string testName) :
    Test(grpName, testName, SPECREV_10b)
{
    // 66 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 4");
    mTestDesc.SetShort(     "Create many IOSQ to IOCQ associations");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Search for 1 of the following namspcs to run test. Find 1st bare "
        "namspc, or find 1st meta namspc, or find 1st E2E namspc. Loop and "
        "create IOSQ's from 1 to GetFeatures.NumberOfQueues ID (NSQA) and "
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


Test::RunType
ManySQtoCQAssoc_r10b::RunnableCoreTest(bool preserve)
{
    ///////////////////////////////////////////////////////////////////////////
    // All code contained herein must never permanently modify the state or
    // configuration of the DUT. Permanence is defined as state or configuration
    // changes that will not be restored after a cold hard reset.
    ///////////////////////////////////////////////////////////////////////////

    return ((preserve == true) ? RUN_FALSE : RUN_TRUE);   // Test is destructive
}


void
ManySQtoCQAssoc_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) Test CreateResources_r10b has run prior.
     *  \endverbatim
     */
    uint16_t uniqueId;

    // Lookup objs which were created in a prior test within group
    SharedASQPtr asq = CAST_TO_ASQ(gRsrcMngr->GetObj(ASQ_GROUP_ID))
    SharedACQPtr acq = CAST_TO_ACQ(gRsrcMngr->GetObj(ACQ_GROUP_ID))

    {
        uint64_t maxIOQEntries;
        // Determine the max IOQ entries supported
        if (gRegisters->Read(CTLSPC_CAP, maxIOQEntries) == false)
            throw FrmwkEx(HERE, "Unable to determine MQES");
        maxIOQEntries &= CAP_MQES;
        maxIOQEntries += 1;      // convert to 1-based
        if (maxIOQEntries < (uint64_t)NumEntriesIOQ) {
            LOG_NRM("Changing number of Q elements from %d to %lld",
                NumEntriesIOQ, (unsigned long long)maxIOQEntries);
            NumEntriesIOQ = maxIOQEntries;
        }
    }

    SharedWritePtr writeCmd = SetWriteCmd();

    LOG_NRM("Create one IOCQ for test lifetime.");
    SharedIOCQPtr iocq = Queues::CreateIOCQContigToHdw(mGrpName,
        mTestName, CALC_TIMEOUT_ms(1), asq, acq, IOQ_ID, NumEntriesIOQ,
        false, IOCQ_CONTIG_GROUP_ID, false, 0, "iocq", true);

    vector<SharedIOSQPtr> iosqVector;
    vector<uint32_t> mSQIDToSQHDVector;
    mSQIDToSQHDVector.push_back(USHRT_MAX); // vector position 0 is not used.

    LOG_NRM("Create Maximum allowed IOSQs and associate with same IOCQ.");
    for (uint32_t j = 1; j <= gInformative->GetFeaturesNumOfIOSQs(); j++) {
        LOG_NRM("Creating contig IOSQ #%d", j);
        SharedIOSQPtr iosq = Queues::CreateIOSQContigToHdw(mGrpName,
            mTestName, CALC_TIMEOUT_ms(1), asq, acq, j, NumEntriesIOQ, false,
            IOSQ_CONTIG_GROUP_ID, IOQ_ID, 0, "iosq", true);

        iosqVector.push_back(iosq);
        mSQIDToSQHDVector.push_back(0);

        for (vector <SharedIOSQPtr>::iterator iosq = iosqVector.begin();
            iosq != iosqVector.end(); iosq++) {
            (*iosq)->Send(writeCmd, uniqueId);
            (*iosq)->Ring();
            mSQIDToSQHDVector[(*iosq)->GetQId()] =
                ++mSQIDToSQHDVector[(*iosq)->GetQId()] %
                (*iosq)->GetNumEntries();
        }
        ReapIOCQAndVerifyCE(iocq, j, mSQIDToSQHDVector);
    }

    LOG_NRM("Delete all IOSQs before the IOCQ to comply with spec.");
    for (vector <SharedIOSQPtr>::iterator iosq = iosqVector.begin();
        iosq != iosqVector.end(); iosq++) {
        Queues::DeleteIOSQToHdw(mGrpName, mTestName, CALC_TIMEOUT_ms(1),
            *iosq, asq, acq);
    }
    Queues::DeleteIOCQToHdw(mGrpName, mTestName, CALC_TIMEOUT_ms(1),
        iocq, asq, acq);
}


SharedWritePtr
ManySQtoCQAssoc_r10b::SetWriteCmd()
{
    Informative::Namspc namspcData = gInformative->Get1stBareMetaE2E();
    LOG_NRM("Processing write cmd using namspc id %d", namspcData.id);
    LBAFormat lbaFormat = namspcData.idCmdNamspc->GetLBAFormat();

    LOG_NRM("Create data pattern to write to media");
    SharedMemBufferPtr dataPat = SharedMemBufferPtr(new MemBuffer());
    uint64_t lbaDataSize = namspcData.idCmdNamspc->GetLBADataSize();

    SharedWritePtr writeCmd = SharedWritePtr(new Write());
    send_64b_bitmask prpBitmask = (send_64b_bitmask)(MASK_PRP1_PAGE
        | MASK_PRP2_PAGE | MASK_PRP2_LIST);

    switch (namspcData.type) {
    case Informative::NS_BARE:
        dataPat->Init(lbaDataSize);
        break;
    case Informative::NS_METAS:
        dataPat->Init(lbaDataSize);
        if (gRsrcMngr->SetMetaAllocSize(lbaFormat.MS) == false)
            throw FrmwkEx(HERE);
        writeCmd->AllocMetaBuffer();
        break;
    case Informative::NS_METAI:
        dataPat->Init(lbaDataSize + lbaFormat.MS);
        break;
    case Informative::NS_E2ES:
    case Informative::NS_E2EI:
        throw FrmwkEx(HERE, "Deferring work to handle this case in future");
        break;
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

    LOG_NRM("Reap one CE and verify and do for all the CE's in CQ.");
    for (uint32_t i = 0; i < numTil; i++) {
        LOG_NRM("Wait for the CE to arrive in IOCQ");
        if (iocq->ReapInquiryWaitSpecify(CALC_TIMEOUT_ms(1), 1, numCE,
            isrCount) == false) {

            iocq->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName, "missing"),
                "Unable to see completion of cmd");
            throw FrmwkEx(HERE);
        } else if (numCE == 0) {
            iocq->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName, "missing"),
                "Unable to see completion of cmd");
            throw FrmwkEx(HERE, "IOCQ should have one new CE for each IOSQ");
        }

        LOG_NRM("The CQ's metrics before reaping holds head_ptr");
        struct nvme_gen_cq iocqMetrics = iocq->GetQMetrics();
        KernelAPI::LogCQMetrics(iocqMetrics);

        LOG_NRM("Reaping CE from IOCQ, requires memory to hold reaped CE");
        SharedMemBufferPtr ceMemIOCQ = SharedMemBufferPtr(new MemBuffer());
        if ((numReaped = iocq->Reap(ceRemain, ceMemIOCQ, isrCount, 1, true))
            != 1) {

            throw FrmwkEx(HERE, "Requested to reap 1 CE, but reaping produced %d",
                numReaped);
        }

        union CE ce = iocq->PeekCE(iocqMetrics.head_ptr);
        ProcessCE::Validate(ce, CESTAT_SUCCESS);  // throws upon error

        LOG_NRM("Validate CE of IOSQ ID=%d", ce.n.SQID);
        if (ce.n.SQHD != mSQIDToSQHDVector[ce.n.SQID]) {
            throw FrmwkEx(HERE, "Expected CE.SQHD = 0x%04X in IOCQ CE but actual "
                "CE.SQHD  = 0x%04X", mSQIDToSQHDVector[ce.n.SQID], ce.n.SQHD);
        }

        mSQIDToSQHDVector[ce.n.SQID] = USHRT_MAX; // strike off this sq id
    }

    LOG_NRM("Validate all SQIDs submitted have arrived.");
    for (uint32_t it = 0; it < mSQIDToSQHDVector.size(); it++) {
        if (mSQIDToSQHDVector[it] != USHRT_MAX) {
            throw FrmwkEx(HERE, "Never received CE for SQID %d", it);
        }
    }
}

}   // namespace
