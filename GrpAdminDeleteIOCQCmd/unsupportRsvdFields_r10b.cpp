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
#include "../Cmds/deleteIOCQ.h"
#include "../Utils/queues.h"
#include "../Utils/io.h"

namespace GrpAdminDeleteIOCQCmd {


UnsupportRsvdFields_r10b::UnsupportRsvdFields_r10b(
    string grpName, string testName) :
    Test(grpName, testName, SPECREV_10b)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 6");
    mTestDesc.SetShort(     "Set unsupported/rsvd fields in cmd");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Unsupported DW's and rsvd fields are treated identical, the "
        "recipient shall not check their value. Issue a CreateIOCQ cmd with "
        "QID=1, num elements=2. Issue a DeleteIOCQ cmd deleting QID=1, expect "
        "success. Reissue identical CreateIOCQ, and issue same DeleteIOCQ cmd "
        "but set all unsupported/rsvd fields, expect success. Set: DW0_b15:10, "
        "DW2, DW3, DW4, DW5, DW6, DW7, DW8, DW9, DW10_b31:16, DW11, DW12, "
        "DW13, DW14, DW15");
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
    uint64_t maxIOQEntries = 2;

    // Lookup objs which were created in a prior test within group
    SharedASQPtr asq = CAST_TO_ASQ(gRsrcMngr->GetObj(ASQ_GROUP_ID))
    SharedACQPtr acq = CAST_TO_ACQ(gRsrcMngr->GetObj(ACQ_GROUP_ID))

    LOG_NRM("Setup element sizes for the IOQ's");
    uint8_t iocqes = (gInformative->GetIdentifyCmdCtrlr()->
        GetValue(IDCTRLRCAP_CQES) & 0xf);
    uint8_t iosqes = (gInformative->GetIdentifyCmdCtrlr()->
        GetValue(IDCTRLRCAP_SQES) & 0xf);
    gCtrlrConfig->SetIOCQES(iocqes);
    gCtrlrConfig->SetIOSQES(iosqes);

    LOG_NRM("Create IOCQ/IOSQ pairs");
    SharedIOCQPtr iocq = Queues::CreateIOCQContigToHdw(mGrpName, mTestName,
        CALC_TIMEOUT_ms(1), asq, acq, IOQ_ID, maxIOQEntries, false,
        IOCQ_GROUP_ID, true, 0);

    LOG_NRM("Delete the IOCQ");
    SharedDeleteIOCQPtr deleteIOCQCmd = SharedDeleteIOCQPtr(new DeleteIOCQ());
    deleteIOCQCmd->Init(iocq);
    IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), asq, acq,
        deleteIOCQCmd, "", true);

    LOG_NRM("Recreate IOCQ");
    iocq = Queues::CreateIOCQContigToHdw(mGrpName, mTestName,
        CALC_TIMEOUT_ms(1), asq, acq, IOQ_ID, maxIOQEntries, false,
        IOCQ_GROUP_ID, true, 0);

    deleteIOCQCmd->Init(iocq);

    LOG_NRM("Set all cmd's rsvd bits");
    uint32_t work = deleteIOCQCmd->GetDword(0);
    work |= 0x0000fc00;      // Set DW0_b15:10 bits
    deleteIOCQCmd->SetDword(work, 0);

    deleteIOCQCmd->SetDword(0xffffffff, 2);
    deleteIOCQCmd->SetDword(0xffffffff, 3);
    deleteIOCQCmd->SetDword(0xffffffff, 4);
    deleteIOCQCmd->SetDword(0xffffffff, 5);
    deleteIOCQCmd->SetDword(0xffffffff, 6);
    deleteIOCQCmd->SetDword(0xffffffff, 7);
    deleteIOCQCmd->SetDword(0xffffffff, 8);
    deleteIOCQCmd->SetDword(0xffffffff, 9);

    // DW10_b31:16
    work = deleteIOCQCmd->GetDword(10);
    work |= 0xffff0000;
    deleteIOCQCmd->SetDword(work, 10);

    deleteIOCQCmd->SetDword(0xffffffff, 11);
    deleteIOCQCmd->SetDword(0xffffffff, 12);
    deleteIOCQCmd->SetDword(0xffffffff, 13);
    deleteIOCQCmd->SetDword(0xffffffff, 14);
    deleteIOCQCmd->SetDword(0xffffffff, 15);

    IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), asq, acq,
        deleteIOCQCmd, "", true);
}


}   // namespace

