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
#include "acceptQPriority_r10b.h"
#include "globals.h"
#include "grpDefs.h"
#include "../Utils/queues.h"
#include "../Utils/io.h"
#include "../Utils/irq.h"

#define PRIORITY_RANGE      4

namespace GrpAdminCreateIOSQCmd {


AcceptQPriority_r10b::AcceptQPriority_r10b(
    string grpName, string testName) :
    Test(grpName, testName, SPECREV_10b)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 5");
    mTestDesc.SetShort(     "Issue CreateIOSQ and verify queue priority is accepted.");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Issue a CreateIOCQ cmd, with QID = 1, num elements = 2, expect "
        "success. Then issue a correlating CreateIOSQ cmds, with QID = 1, "
        "and range DW11_b2:1 thru all possible values, expect success. "
        "Delete the IOSQ between creations.");
}


AcceptQPriority_r10b::~AcceptQPriority_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


AcceptQPriority_r10b::
AcceptQPriority_r10b(const AcceptQPriority_r10b &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


AcceptQPriority_r10b &
AcceptQPriority_r10b::operator=(const AcceptQPriority_r10b &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


Test::RunType
AcceptQPriority_r10b::RunnableCoreTest(bool preserve)
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
AcceptQPriority_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * None.
     * \endverbatim
     */
    uint64_t maxIOQEntries = 2;

    if (gCtrlrConfig->SetState(ST_DISABLE_COMPLETELY) == false)
        throw FrmwkEx(HERE);

    LOG_NRM("Create admin queues ACQ and ASQ");
    SharedACQPtr acq = SharedACQPtr(new ACQ(gDutFd));
    acq->Init(5);

    SharedASQPtr asq = SharedASQPtr(new ASQ(gDutFd));
    asq->Init(5);

    // All queues will use identical IRQ vector
    IRQ::SetAnySchemeSpecifyNum(1);

    gCtrlrConfig->SetCSS(CtrlrConfig::CSS_NVM_CMDSET);
    if (gCtrlrConfig->SetState(ST_ENABLE) == false)
        throw FrmwkEx(HERE);

    LOG_NRM("Setup element sizes for the IOQ's");
    gCtrlrConfig->SetIOCQES(gInformative->GetIdentifyCmdCtrlr()->
        GetValue(IDCTRLRCAP_CQES) & 0xf);
    gCtrlrConfig->SetIOSQES(gInformative->GetIdentifyCmdCtrlr()->
        GetValue(IDCTRLRCAP_SQES) & 0xf);

    LOG_NRM("Create IOCQ with QID = %d", IOQ_ID);
    SharedIOCQPtr iocq = Queues::CreateIOCQContigToHdw(mGrpName, mTestName,
        CALC_TIMEOUT_ms(1), asq, acq, IOQ_ID, maxIOQEntries, false,
        IOCQ_GROUP_ID, true, 0);

    for (uint8_t priority = 0; priority < PRIORITY_RANGE; priority++) {
        LOG_NRM("Create IOSQ with QID = %d and priority #%d", IOQ_ID, priority);
        SharedIOSQPtr iosq = Queues::CreateIOSQContigToHdw(mGrpName,
            mTestName, CALC_TIMEOUT_ms(1), asq, acq, IOQ_ID, maxIOQEntries,
            false, IOSQ_GROUP_ID, IOQ_ID, priority);

        LOG_NRM("Delete IOSQ with QID = %d and priority #%d", IOQ_ID, priority);
        Queues::DeleteIOSQToHdw(mGrpName, mTestName, CALC_TIMEOUT_ms(1), iosq,
            asq, acq, "", false);
    }
}


}   // namespace

