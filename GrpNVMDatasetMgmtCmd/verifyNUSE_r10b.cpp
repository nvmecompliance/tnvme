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
#include "verifyNUSE_r10b.h"
#include "globals.h"
#include "grpDefs.h"
#include "Singletons/informative.h"
#include "../Queues/iocq.h"
#include "../Queues/iosq.h"
#include "../Utils/io.h"
#include "../Cmds/write.h"
#include "../Cmds/datasetMgmt.h"

#define NUM_RANGES      1

namespace GrpNVMDatasetMgmtCmd {


VerifyNUSE_r10b::VerifyNUSE_r10b(
    string grpName, string testName) :
    Test(grpName, testName, SPECREV_10b)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 6");
    mTestDesc.SetShort(     "Issue dataset mgmt single and verify NUSE");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Search for 1 of the following namspcs to run test. Find 1st bare "
        "namspc, or find 1st meta namspc, or find 1st E2E namspc. "
        "1) Decalloc every LBA in selected namspc. 2) Write data "
        "pattern dword++, and approp metadata/E2E if necessary, and "
        "issue a single block to LBA 0. 3) Validate (Identify.NUSE == 1).");
}


VerifyNUSE_r10b::~VerifyNUSE_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


VerifyNUSE_r10b::
VerifyNUSE_r10b(const VerifyNUSE_r10b &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


VerifyNUSE_r10b &
VerifyNUSE_r10b::operator=(const VerifyNUSE_r10b &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


Test::RunType
VerifyNUSE_r10b::RunnableCoreTest(bool preserve)
{
    ///////////////////////////////////////////////////////////////////////////
    // All code contained herein must never permanently modify the state or
    // configuration of the DUT. Permanence is defined as state or configuration
    // changes that will not be restored after a cold hard reset.
    ///////////////////////////////////////////////////////////////////////////

    ConstSharedIdentifyPtr idCtrlrCap = gInformative->GetIdentifyCmdCtrlr();
    uint64_t oncs = idCtrlrCap->GetValue(IDCTRLRCAP_ONCS);
    if ((oncs & ONCS_SUP_DSM_CMD) == 0)
        return RUN_FALSE;

    return ((preserve == true) ? RUN_FALSE : RUN_TRUE);   // Test is destructive
}


void
VerifyNUSE_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) Test CreateResources_r10b has run prior.
     * \endverbatim
     */
    string work;

    LOG_NRM("Lookup Q'S which were created in a prior test within group");
    SharedASQPtr asq = CAST_TO_ASQ(gRsrcMngr->GetObj(ASQ_GROUP_ID))
    SharedACQPtr acq = CAST_TO_ACQ(gRsrcMngr->GetObj(ACQ_GROUP_ID))
    SharedIOSQPtr iosq = CAST_TO_IOSQ(gRsrcMngr->GetObj(IOSQ_GROUP_ID));
    SharedIOCQPtr iocq = CAST_TO_IOCQ(gRsrcMngr->GetObj(IOCQ_GROUP_ID));

    LOG_NRM("Search for 1st bare/meta or e2e namespace.");
    Informative::Namspc namspcData = gInformative->Get1stBareMetaE2E();
    uint64_t ncap = namspcData.idCmdNamspc->GetValue(IDNAMESPC_NCAP);
    LBAFormat lbaFormat = namspcData.idCmdNamspc->GetLBAFormat();
    uint64_t lbaDataSize = namspcData.idCmdNamspc->GetLBADataSize();
    LOG_NRM("For namespace ID #%d; NCAP = 0x%08lX", namspcData.id, ncap);

    LOG_NRM("Create dataset mgmt cmd to be used subsequently");
    SharedDatasetMgmtPtr datasetMgmtCmd =
        SharedDatasetMgmtPtr(new DatasetMgmt());
    SharedMemBufferPtr rangeMem = SharedMemBufferPtr(new MemBuffer());
    rangeMem->Init((NUM_RANGES * sizeof(RangeDef)), true);

    send_64b_bitmask prpBitmask =
            (send_64b_bitmask)(MASK_PRP1_PAGE | MASK_PRP2_PAGE);
    datasetMgmtCmd->SetNSID(namspcData.id);
    datasetMgmtCmd->SetNR(NUM_RANGES - 1);  // convert to 0-based
    datasetMgmtCmd->SetPrpBuffer(prpBitmask, rangeMem);
    datasetMgmtCmd->SetAD(true);

    LOG_NRM("Deallocate every LBA in namespace ID #%d", namspcData.id);
    RangeDef *rangePtr = (RangeDef *)rangeMem->GetBuffer();
    rangePtr->slba = 0;
    rangePtr->length = ncap;

    work = str(boost::format("deallocate.%08Xh") % ncap);
    IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), iosq, iocq,
        datasetMgmtCmd, work, true);

    LOG_NRM("Create identify cmd & assoc some buffer memory");
    SharedIdentifyPtr idCmdNamSpc = SharedIdentifyPtr(new Identify());
    LOG_NRM("Force identify to request namespace struct");
    idCmdNamSpc->SetCNS(false);
    idCmdNamSpc->SetNSID(namspcData.id);
    SharedMemBufferPtr idMemNamSpc = SharedMemBufferPtr(new MemBuffer());
    idMemNamSpc->InitAlignment(Identify::IDEAL_DATA_SIZE, PRP_BUFFER_ALIGNMENT,
        true, 0);
    send_64b_bitmask idPrpNamSpc =
        (send_64b_bitmask)(MASK_PRP1_PAGE | MASK_PRP2_PAGE);
    idCmdNamSpc->SetPrpBuffer(idPrpNamSpc, idMemNamSpc);

    work = str(boost::format("IdentifyNamspc.nsid.%d.lba.all") % namspcData.id);
    IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), asq, acq,
        idCmdNamSpc, work, true);

    LOG_NRM("Verify namespace utilization is zero after de-allocation.");
    uint64_t nuse = idCmdNamSpc->GetValue(IDNAMESPC_NUSE);
    if (nuse != 0x0) {
        throw FrmwkEx(HERE, "Expected namspc utilization = 0x0 but found "
            "namspc utilization = 0x%08X", nuse);
    }

    LOG_NRM("Write data pattern and issue a single block to LBA 0");
    SharedWritePtr writeCmd = SharedWritePtr(new Write());
    SharedMemBufferPtr writeMem = SharedMemBufferPtr(new MemBuffer());

    switch (namspcData.type) {
    case Informative::NS_BARE:
        writeMem->Init(lbaDataSize);
        writeMem->SetDataPattern(DATAPAT_INC_32BIT);
        break;
    case Informative::NS_METAS:
        writeMem->Init(lbaDataSize);
        if (gRsrcMngr->SetMetaAllocSize(lbaFormat.MS) == false)
            throw FrmwkEx(HERE);
        writeCmd->AllocMetaBuffer();
        writeMem->SetDataPattern(DATAPAT_INC_32BIT);
        writeCmd->SetMetaDataPattern(DATAPAT_INC_32BIT);
        break;
    case Informative::NS_METAI:
        writeMem->Init(lbaDataSize + lbaFormat.MS);
        writeMem->SetDataPattern(DATAPAT_INC_32BIT);
        break;
    case Informative::NS_E2ES:
    case Informative::NS_E2EI:
        throw FrmwkEx(HERE, "Deferring work to handle this case in future");
        break;
    }

    prpBitmask = (send_64b_bitmask)(MASK_PRP1_PAGE | MASK_PRP2_PAGE |
        MASK_PRP2_LIST);
    writeCmd->SetPrpBuffer(prpBitmask, writeMem);
    writeCmd->SetNSID(namspcData.id);
    writeCmd->SetNLB(0);

    work = str(boost::format("write.nsid.%d.nlba.0") % namspcData.id);
    IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), iosq,
        iocq, writeCmd, work, true);

    work = str(boost::format("IdentifyNamspc.nsid.%d.nlba.0") % namspcData.id);
    IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), asq, acq,
        idCmdNamSpc, work, true);

    LOG_NRM("Verify namespace utilization is one after single LBA write cmd.");
    nuse = idCmdNamSpc->GetValue(IDNAMESPC_NUSE);
    if (nuse != 0x1) {
        throw FrmwkEx(HERE, "Expected namspc utilization = 0x1 but found "
            "namspc utilization = 0x%08X", nuse);
    }
}


}   // namespace

