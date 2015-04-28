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

#include <string.h>
#include <boost/format.hpp>
#include "unsupportRsvdFields_r10b.h"
#include "globals.h"
#include "grpDefs.h"
#include "../Utils/kernelAPI.h"
#include "../Utils/irq.h"
#include "../Utils/io.h"


#define SQ0TBDL     0x1000

namespace GrpReservationsHostB {


UnsupportRsvdFields_r10b::UnsupportRsvdFields_r10b(
    string grpName, string testName) :
    Test(grpName, testName, SPECREV_10b)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section ?");
    mTestDesc.SetShort(     "Set unsupported/rsvd fields in cmd");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Reset ctrlr to cause a clearing of DUT state. Issue 1 async cmd. "
        "Delay 5 sec, verify no CE exists in ACQ. Ring doorbell for IOSQ # 1, "
        "reap a successful CE from the ASQ, validate CE.DW0 for proper error. "
        "Read error event log page to clear async event. Unsupported DW's and "
        "rsvd fields are treated identical, the recipient shall not check "
        "their value. Issue same cmd setting all unsupported/rsvd fields, "
        "expect success. Set: DW0_b15:10, DW2, DW3, DW4, DW5, DW6, DW7, DW8, "
        "DW9, DW10, DW11, DW12, DW13, DW14, DW15. Validate CE.DW0 for ringing "
        "non-existent doorbell. Read error event log page to clear "
        "async event.");
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

    // Choose to return one of these or create your own logic
    return ((preserve == true) ? RUN_FALSE : RUN_TRUE);   // Test is destructive

}


void
UnsupportRsvdFields_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) none
     *  \endverbatim
     */
    if (gCtrlrConfig->SetState(ST_DISABLE_COMPLETELY) == false)
        throw FrmwkEx(HERE);

    LOG_NRM("Create admin queues ACQ and ASQ for test lifetime");
    SharedACQPtr acq = SharedACQPtr(new ACQ(gDutFd));
    acq->Init(5);

    SharedASQPtr asq = SharedASQPtr(new ASQ(gDutFd));
    asq->Init(5);

    // All queues will use identical IRQ vector
    IRQ::SetAnySchemeSpecifyNum(1);

    gCtrlrConfig->SetCSS(CtrlrConfig::CSS_NVM_CMDSET);
    if (gCtrlrConfig->SetState(ST_ENABLE) == false)
        throw FrmwkEx(HERE);
}

}   // namespace
