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

#include "unsupportRsvdFields_r10b.h"
#include "globals.h"
#include "grpDefs.h"
#include "../Queues/iocq.h"
#include "../Queues/iosq.h"
#include "../Utils/io.h"
#include "../Cmds/datasetMgmt.h"

namespace GrpNVMDatasetMgmtCmd {


UnsupportRsvdFields_r10b::UnsupportRsvdFields_r10b(
    string grpName, string testName) :
    Test(grpName, testName, SPECREV_10b)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 6");
    mTestDesc.SetShort(     "Set unsupported/rsvd fields in cmd");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Unsupported DW's and rsvd fields are treated identical, the recipient "
        "shall not check their value. Determine Identify.NN and issue flush "
        "cmd to all namspc, expect success. Then issue same cmd setting all "
        "unsupported/rsvd fields, expect success. Set: DW0_b15:10, DW2, DW3, "
        "DW4, DW5, DW8, DW9, DW10_b31:8, DW11_b31:3, DW12, DW13, DW14, DW15.");
}


UnsupportRsvdFields_r10b::~UnsupportRsvdFields_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


UnsupportRsvdFields_r10b::
UnsupportRsvdFields_r10b(const UnsupportRsvdFields_r10b &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


UnsupportRsvdFields_r10b &
UnsupportRsvdFields_r10b::operator=(const UnsupportRsvdFields_r10b &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


Test::RunType
UnsupportRsvdFields_r10b::RunnableCoreTest(bool preserve)
{
    ///////////////////////////////////////////////////////////////////////////
    // All code contained herein must never permanently modify the state or
    // configuration of the DUT. Permanence is defined as state or configuration
    // changes that will not be restored after a cold hard reset.
    ///////////////////////////////////////////////////////////////////////////

    ConstSharedIdentifyPtr idCtrlrCap = gInformative->GetIdentifyCmdCtrlr();
    uint64_t oncs = idCtrlrCap->GetValue(IDCTRLRCAP_ONCS);
    if ((oncs & ONCS_SUP_DSM_CMD) == 0) {
        LOG_NRM("Reporting not supoorted (oncs)%ld", oncs);
        return RUN_FALSE;
    }

    LOG_NRM("Reporting runnable (oncs)%ld", oncs);

    if (gCmdLine.rsvdfields == false)
        return RUN_FALSE;   // Optional rsvd fields test skipped.

    preserve = preserve;    // Suppress compiler error/warning
    return RUN_TRUE;        // This test is never destructive

}


void
UnsupportRsvdFields_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) Test CreateResources_r10b has run prior.
     * \endverbatim
     */

    // Lookup objs which were created in a prior test within group
    SharedIOSQPtr iosq = CAST_TO_IOSQ(gRsrcMngr->GetObj(IOSQ_GROUP_ID));
    SharedIOCQPtr iocq = CAST_TO_IOCQ(gRsrcMngr->GetObj(IOCQ_GROUP_ID));

    SharedDatasetMgmtPtr datasetMgmtCmd =
        SharedDatasetMgmtPtr(new DatasetMgmt());

    ConstSharedIdentifyPtr idCtrlr = gInformative->GetIdentifyCmdCtrlr();
    for (uint64_t i = 1; i <= idCtrlr->GetValue(IDCTRLRCAP_NN); i++) {
        LOG_NRM("Processing namspc %ld", i);
        datasetMgmtCmd->SetNSID(i);

        LOG_NRM("Create memory to contain 1 dataset range def");
        SharedMemBufferPtr rangeMem = SharedMemBufferPtr(new MemBuffer());
        rangeMem->Init(sizeof(RangeDef), true);
        send_64b_bitmask prpReq =
            (send_64b_bitmask)(MASK_PRP1_PAGE | MASK_PRP2_PAGE);
        datasetMgmtCmd->SetPrpBuffer(prpReq, rangeMem);

        LOG_NRM("Setup generic range def guidelines");
        RangeDef *rangeDef = (RangeDef *)rangeMem->GetBuffer();
        rangeDef->length = 1;

        IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), iosq, iocq,
            datasetMgmtCmd, "none.set", true);

        LOG_NRM("Set all cmd's rsvd bits");
        uint32_t work = datasetMgmtCmd->GetDword(0);
        work |= 0x0000fc00;      // Set DW0_b15:10 bits
        datasetMgmtCmd->SetDword(work, 0);

        datasetMgmtCmd->SetDword(0xffffffff, 2);
        datasetMgmtCmd->SetDword(0xffffffff, 3);
        datasetMgmtCmd->SetDword(0xffffffff, 4);
        datasetMgmtCmd->SetDword(0xffffffff, 5);
        datasetMgmtCmd->SetDword(0xffffffff, 8);
        datasetMgmtCmd->SetDword(0xffffffff, 9);

        work = datasetMgmtCmd->GetDword(10);
        work |= 0xffffff00;     // Set DW10_b31:8 bits
        datasetMgmtCmd->SetDword(work, 10);


        work = datasetMgmtCmd->GetDword(11);
        work |= 0xfffffff8;     // Set DW11_b31:3 bits
        datasetMgmtCmd->SetDword(work, 11);

        datasetMgmtCmd->SetDword(0xffffffff, 12);
        datasetMgmtCmd->SetDword(0xffffffff, 13);
        datasetMgmtCmd->SetDword(0xffffffff, 14);
        datasetMgmtCmd->SetDword(0xffffffff, 15);

        IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), iosq, iocq,
            datasetMgmtCmd, "all.set", true);
    }
}


}   // namespace

