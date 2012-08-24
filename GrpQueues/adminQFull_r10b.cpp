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

#include "adminQFull_r10b.h"
#include "globals.h"
#include "grpDefs.h"
#include "../Utils/kernelAPI.h"
#include "../Utils/io.h"

namespace GrpQueues {


AdminQFull_r10b::AdminQFull_r10b(
    string grpName, string testName) :
    Test(grpName, testName, SPECREV_10b)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 4");
    mTestDesc.SetShort(     "Issue cmds until both ASQ and ACQ fill up.");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Create 2 cases where size: {ASQ=ACQ, ASQ=ACQ+1}, however within "
        "each case, 3 samples of the queue pairs are {min, mid, max} sized. "
        "Submit ((ASQ.size-1) cmds into ASQ always fills both queues, "
        "submit, ring each cmd, wait for CE to arrive, continue pattern. "
        "Verify in = case all CE's arrive, in > case all but 1 CE's "
        "arrive in ACQ.");
}


AdminQFull_r10b::~AdminQFull_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


AdminQFull_r10b::
AdminQFull_r10b(const AdminQFull_r10b &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


AdminQFull_r10b &
AdminQFull_r10b::operator=(const AdminQFull_r10b &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


Test::RunType
AdminQFull_r10b::RunnableCoreTest(bool preserve)
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
AdminQFull_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) none
     *  \endverbatim
     */
    LOG_NRM("Create identify cmd and assoc some buffer memory");
    SharedIdentifyPtr idCmdCtrlr = SharedIdentifyPtr(new Identify());
    LOG_NRM("Force identify to request ctrlr capabilities struct");
    idCmdCtrlr->SetCNS(true);
    SharedMemBufferPtr idMemCap = SharedMemBufferPtr(new MemBuffer());
    idMemCap->InitAlignment(Identify::IDEAL_DATA_SIZE, PRP_BUFFER_ALIGNMENT,
        false, 0);
    send_64b_bitmask prpReq =
        (send_64b_bitmask)(MASK_PRP1_PAGE | MASK_PRP2_PAGE);
    idCmdCtrlr->SetPrpBuffer(prpReq, idMemCap);

    // Case 1 - ASQ = ACQ (min, middle and max)
    AdminQFull(2, 2, idCmdCtrlr);
    AdminQFull((MAX_ADMIN_Q_SIZE/2), (MAX_ADMIN_Q_SIZE/2), idCmdCtrlr);
    AdminQFull(MAX_ADMIN_Q_SIZE, MAX_ADMIN_Q_SIZE, idCmdCtrlr);

    // Case 2 - ASQ =  ACQ + 1 (min , middle and max)
    AdminQFull(3, 2, idCmdCtrlr);
    AdminQFull((MAX_ADMIN_Q_SIZE/2), ((MAX_ADMIN_Q_SIZE/2) - 1), idCmdCtrlr);
    AdminQFull(MAX_ADMIN_Q_SIZE, (MAX_ADMIN_Q_SIZE - 1), idCmdCtrlr);
}


void
AdminQFull_r10b::AdminQFull(uint16_t numASQEntries, uint16_t numACQEntries,
    SharedIdentifyPtr idCmdCtrlr)
{
    uint32_t numCE;
    uint32_t isrCount;
    uint16_t uniqueId;

    if (gCtrlrConfig->SetState(ST_DISABLE_COMPLETELY) == false)
        throw FrmwkEx(HERE);

    // Create Admin Q Objects for test lifetime
    SharedACQPtr acq = CAST_TO_ACQ(SharedACQPtr(new ACQ(gDutFd)))
    acq->Init(numACQEntries);

    SharedASQPtr asq = CAST_TO_ASQ(SharedASQPtr(new ASQ(gDutFd)))
    asq->Init(numASQEntries);

    gCtrlrConfig->SetCSS(CtrlrConfig::CSS_NVM_CMDSET);
    if (gCtrlrConfig->SetState(ST_ENABLE) == false)
        throw FrmwkEx(HERE);

    uint32_t nCmdsToSubmit = numASQEntries - 1;
    LOG_NRM("Send #%d cmds to hdw via ASQ", nCmdsToSubmit);
    for (uint32_t nCmds = 0; nCmds < nCmdsToSubmit; nCmds++) {
        LOG_NRM("Sending #%d of #%d Identify Cmds thru ASQ", nCmds + 1,
            nCmdsToSubmit);
        asq->Send(idCmdCtrlr, uniqueId);
        asq->Ring();

        LOG_NRM("Wait for the CE to arrive in ACQ");
        if (acq->ReapInquiryWaitSpecify(CALC_TIMEOUT_ms(1), (nCmds + 1), numCE,
            isrCount) == false) {

            // when asq size = acq size + 1, last CE will never arrive.
            if ((numASQEntries == numACQEntries + 1) &&
                (nCmds == nCmdsToSubmit - 1)) {
                // Reap one element from IOCQ to make room for last CE.
                IO::ReapCE(acq, 1, isrCount, mGrpName, mTestName, "ACQCE",
                    CESTAT_SUCCESS);
                if (acq->ReapInquiryWaitSpecify(CALC_TIMEOUT_ms(1), nCmds,
                    numCE, isrCount) == false) {
                    acq->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName,
                        "acq." + idCmdCtrlr->GetName()), "Dump entire ACQ");
                    throw FrmwkEx(HERE, "Unable to see last CE as expected");
                }
                break;
            }
            acq->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName,
                "acq." + idCmdCtrlr->GetName()), "Dump Entire ACQ");
            throw FrmwkEx(HERE, "Unable to see CE for issued cmd #%d", nCmds + 1);

        } else if (numCE != nCmds + 1) {
            acq->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName,
                "acq." + idCmdCtrlr->GetName()), "Dump Entire ACQ");
            throw FrmwkEx(HERE, "Missing last CE, #%d cmds of #%d received",
                nCmds + 1, numCE);
        }
    }
}


}   // namespace
