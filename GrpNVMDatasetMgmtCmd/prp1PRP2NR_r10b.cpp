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
#include "prp1PRP2NR_r10b.h"
#include "globals.h"
#include "grpDefs.h"
#include "../Queues/iocq.h"
#include "../Queues/iosq.h"
#include "../Utils/io.h"
#include "../Cmds/datasetMgmt.h"

#define OUTER_LOOP_MAX          4096
#define INNER_LOOP_MAX          256


namespace GrpNVMDatasetMgmtCmd {


PRP1PRP2NR_r10b::PRP1PRP2NR_r10b(
    string grpName, string testName) :
    Test(grpName, testName, SPECREV_10b)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 6");
    mTestDesc.SetShort(     "Verify all combos of PRP1, PRP2, and Number of Ranges (NR)");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "For all namspcs from Identify.NN, issue multiple dataset mgmt cmds "
        "with DW11 = 0, and vary the following cmd parameters. Outer loop must "
        "iterate from 0 to 4KB representing the offset into the 1st memory "
        "page of the data payload. An inner loop will iterate from 0 to 255 "
        "representing the DW10.NR field for each cmd. Each buffer will "
        "initialize all ranges in the data payload identically, where every "
        "context attribute = 0, and the starting LBA plus the LBA length of "
        "each range must be divided equally to consume the entire address "
        "space of the current namspc spec'd by Identify.NCAP. Expect success "
        "for all cmds. NOTE: Before sending each cmd to dnvme, preset PRP2 to "
        "a random 64b number. If the buffer does not require usage of PRP2 the "
        "random number will be issued to the DUT, otherwise it will contain a "
        "valid ptr to memory. If PRP2 is not needed it is considered a rsvd "
        "field. All rsvd fields are treated identical and the recipient shall "
        "not check their value. Always initiate the random gen with seed 97 "
        "before this test starts.");
}


PRP1PRP2NR_r10b::~PRP1PRP2NR_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


PRP1PRP2NR_r10b::
PRP1PRP2NR_r10b(const PRP1PRP2NR_r10b &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


PRP1PRP2NR_r10b &
PRP1PRP2NR_r10b::operator=(const PRP1PRP2NR_r10b &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


Test::RunType
PRP1PRP2NR_r10b::RunnableCoreTest(bool preserve)
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

    preserve = preserve;    // Suppress compiler error/warning
    return RUN_TRUE;        // This test is never destructive
}


void
PRP1PRP2NR_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) Test CreateResources_r10b has run prior.
     * \endverbatim
     */
    string work;

    LOG_NRM("Initialize random seed");
    srand(97);

    // Lookup objs which were created in a prior test within group
    SharedIOSQPtr iosq = CAST_TO_IOSQ(gRsrcMngr->GetObj(IOSQ_GROUP_ID));
    SharedIOCQPtr iocq = CAST_TO_IOCQ(gRsrcMngr->GetObj(IOCQ_GROUP_ID));

    LOG_NRM("Create memory to contain 256 dataset range defs");
    SharedMemBufferPtr rangeMem = SharedMemBufferPtr(new MemBuffer());
    send_64b_bitmask prpBitmask =
        (send_64b_bitmask)(MASK_PRP1_PAGE | MASK_PRP2_PAGE);
    SharedDatasetMgmtPtr datasetMgmtCmd =
        SharedDatasetMgmtPtr(new DatasetMgmt());

    ConstSharedIdentifyPtr idCtrlr = gInformative->GetIdentifyCmdCtrlr();
    for (uint64_t i = 1; i <= idCtrlr->GetValue(IDCTRLRCAP_NN); i++) {
        LOG_NRM("Processing namspc %ld", i);
        datasetMgmtCmd->SetNSID(i);

        ConstSharedIdentifyPtr idNamspc = gInformative->GetIdentifyCmdNamspc(i);
        uint64_t nsze = idNamspc->GetValue(IDNAMESPC_NSZE);
        for (int64_t pgOff = 0; pgOff < OUTER_LOOP_MAX; pgOff += 4) {

            // pow2 = {1, 2, 4, 8, 16, 32, 64, ...}
            for (uint64_t pow2 = 2; pow2 <= INNER_LOOP_MAX; pow2 <<= 1) {
                // nr = {(1, 2, 3), (3, 4, 5), (7, 8, 9), ...}
                for (uint32_t nr = (pow2 - 1);
                    (nr <= (pow2 + 1)) && (nr <= pow2); nr++) {

                    LOG_NRM("Issue dataset mgmt cmd for %d ranges", nr);
                    datasetMgmtCmd->SetNR(nr - 1);  // convert to 0-based

                    rangeMem->InitOffset1stPage((nr * sizeof(RangeDef)),
                        pgOff, true);
                    datasetMgmtCmd->SetPrpBuffer(prpBitmask, rangeMem);

                    LOG_NRM("Set random value for PRP2; dnvme could overwrite");
                    datasetMgmtCmd->SetDword(rand(), 8);
                    datasetMgmtCmd->SetDword(rand(), 9);

                    uint64_t slba = 0;
                    RangeDef *rangePtr = (RangeDef *)rangeMem->GetBuffer();
                    for (uint32_t idx = 1; idx <= nr; idx++, rangePtr++) {
                        rangePtr->slba = slba;
                        slba += (nsze / nr);

                        if ((idx == nr) && (nsze % nr)) {
                            LOG_NRM("Handle odd number of namspc's");
                            rangePtr->length = ((nsze / nr) + (nsze % nr));
                        } else {
                            rangePtr->length = (nsze / nr);
                        }
                    }

                    bool enableLog = false;
                    if ((pgOff % 512) == 0)
                        enableLog = true;

                    work = str(boost::format("offset%d.nr%d ") % pgOff % nr);
                    IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1),
                        iosq, iocq, datasetMgmtCmd, work, enableLog);
                }
            }
        }
    }
}


}   // namespace

