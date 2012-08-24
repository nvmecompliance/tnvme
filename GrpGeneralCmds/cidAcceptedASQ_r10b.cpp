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
#include "cidAcceptedASQ_r10b.h"
#include "globals.h"
#include "grpDefs.h"
#include "../Utils/io.h"


namespace GrpGeneralCmds {


#define MAX_CMDS        (65536 + 1)


CIDAcceptedASQ_r10b::CIDAcceptedASQ_r10b(
    string grpName, string testName) :
    Test(grpName, testName, SPECREV_10b)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 4");
    mTestDesc.SetShort(     "Verify all CID values are accepted in ASQ.");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Issue Identify cmd (65536 + 1) times and verify that the dnvme "
        "assigned CID value is unique each time. Each command must be "
        "completed in success and be reaped from the ACQ.");
}


CIDAcceptedASQ_r10b::~CIDAcceptedASQ_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


CIDAcceptedASQ_r10b::
CIDAcceptedASQ_r10b(const CIDAcceptedASQ_r10b &other) :
    Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


CIDAcceptedASQ_r10b &
CIDAcceptedASQ_r10b::operator=(const CIDAcceptedASQ_r10b
    &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


Test::RunType
CIDAcceptedASQ_r10b::RunnableCoreTest(bool preserve)
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
CIDAcceptedASQ_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) Test CreateResources_r10b has run prior.
     * \endverbatim
     */
    uint32_t isrCount;
    // Lookup objs which were created in a prior test within group
    SharedASQPtr asq = CAST_TO_ASQ(gRsrcMngr->GetObj(ASQ_GROUP_ID))
    SharedACQPtr acq = CAST_TO_ACQ(gRsrcMngr->GetObj(ACQ_GROUP_ID))

    LOG_NRM("Verifying that the ACQ is empty");
    if (acq->ReapInquiry(isrCount, true) != 0) {
        acq->Dump(
            FileSystem::PrepDumpFile(mGrpName, mTestName, "acq",
            "notEmpty"), "Test assumption have not been met");
        throw FrmwkEx(HERE, 
            "The ACQ should not have any CE's waiting before testing");
    }

    LOG_NRM("Create identify cmd and assoc some buffer memory");
    SharedIdentifyPtr idCmdCap = SharedIdentifyPtr(new Identify());
    LOG_NRM("Force identify to request ctrlr capabilities struct");
    idCmdCap->SetCNS(true);
    SharedMemBufferPtr idMemCap = SharedMemBufferPtr(new MemBuffer());
    idMemCap->InitAlignment(Identify::IDEAL_DATA_SIZE, PRP_BUFFER_ALIGNMENT,
        false, 0);
    send_64b_bitmask idPrpCap =
        (send_64b_bitmask)(MASK_PRP1_PAGE | MASK_PRP2_PAGE);
    idCmdCap->SetPrpBuffer(idPrpCap, idMemCap);

    LOG_NRM("Learn initial unique command id assigned by dnvme.");
    uint16_t currCID;
    asq->Send(idCmdCap, currCID);
    uint16_t prevCID = currCID;
    for (uint32_t nCmds = 0; nCmds < MAX_CMDS; nCmds++) {
        asq->Ring();
        LOG_NRM("Verify unique CID #%d for Cmd #%d", currCID, nCmds + 1);
        ReapVerifyCID(asq, acq, currCID);

        asq->Send(idCmdCap, currCID);
        if (currCID != (uint16_t)(prevCID + 1)) {
            asq->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName, "asq.fail"),
                "Dump Entire ASQ");
            acq->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName, "acq.fail"),
                "Dump Entire ACQ");
            throw FrmwkEx(HERE, "Current CID(%d) != prev + 1(%d)", currCID,
                prevCID);
        }

        prevCID = currCID;
    }
}


void
CIDAcceptedASQ_r10b::ReapVerifyCID(SharedASQPtr asq, SharedACQPtr acq,
    uint16_t currCID)
{
    uint32_t isrCount;
    uint32_t ceRemain;
    uint32_t numReaped;
    uint32_t numCE;

    if (acq->ReapInquiryWaitSpecify(CALC_TIMEOUT_ms(1), 1, numCE, isrCount)
        == false) {
        acq->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName, "acq.fail"),
            "Dump Entire ACQ");
        throw FrmwkEx(HERE, "Unable to see CEs for issued cmd");
    }

    SharedMemBufferPtr ceMem = SharedMemBufferPtr(new MemBuffer());
    if ((numReaped = acq->Reap(ceRemain, ceMem, isrCount, numCE, true)) != 1) {
        acq->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName, "acq.fail"),
            "Dump Entire ACQ");
        throw FrmwkEx(HERE, "Unable to reap on ACQ");
    }

    union CE *ce = (union CE *)ceMem->GetBuffer();
    ProcessCE::Validate(*ce);  // throws upon error
    if (ce->n.CID != currCID) {
        asq->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName, "asq.fail"),
            "Dump Entire ASQ");
        acq->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName, "acq.fail"),
            "Dump Entire ACQ");
        throw FrmwkEx(HERE, "Received CID %d but expected %d", ce->n.CID,
            currCID);
    }
}


}   // namespace
