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

#include "createResources_r10b.h"
#include "globals.h"
#include "grpDefs.h"
#include "../Utils/irq.h"
#include "../Utils/queues.h"


namespace GrpGeneralCmds {


uint32_t CreateResources_r10b::numEntriesIOQ = 2;


CreateResources_r10b::CreateResources_r10b(
    string grpName, string testName) :
    Test(grpName, testName, SPECREV_10b)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 6");
    mTestDesc.SetShort(     "Create resources needed by subsequent tests");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Create resources with group lifetime which are needed by subsequent "
        "tests");
}


CreateResources_r10b::~CreateResources_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


CreateResources_r10b::
CreateResources_r10b(const CreateResources_r10b &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


CreateResources_r10b &
CreateResources_r10b::operator=(const CreateResources_r10b &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}



Test::RunType
CreateResources_r10b::RunnableCoreTest(bool preserve)
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
CreateResources_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) This is the 1st within GrpGeneralCmds.
     * \endverbatim
     */
    if (gCtrlrConfig->SetState(ST_DISABLE_COMPLETELY) == false)
        throw FrmwkEx(HERE);

    SharedACQPtr acq = CAST_TO_ACQ(
        gRsrcMngr->AllocObj(Trackable::OBJ_ACQ, ACQ_GROUP_ID))
    acq->Init(5);

    SharedASQPtr asq = CAST_TO_ASQ(
        gRsrcMngr->AllocObj(Trackable::OBJ_ASQ, ASQ_GROUP_ID))
    asq->Init(5);

    // All queues will use identical IRQ vector
    IRQ::SetAnySchemeSpecifyNum(2);     // throws upon error

    gCtrlrConfig->SetCSS(CtrlrConfig::CSS_NVM_CMDSET);
    gCtrlrConfig->SetIOCQES(CtrlrConfig::MIN_IOCQES);
    gCtrlrConfig->SetIOSQES(CtrlrConfig::MIN_IOSQES);
    if (gCtrlrConfig->SetState(ST_ENABLE) == false)
        throw FrmwkEx(HERE);

    {
        uint64_t maxIOQEntries;
        // Determine the max IOQ entries supported
        if (gRegisters->Read(CTLSPC_CAP, maxIOQEntries) == false)
            throw FrmwkEx(HERE, "Unable to determine MQES");

        maxIOQEntries &= CAP_MQES;
        maxIOQEntries += 1;      // convert to 1-based
        if (maxIOQEntries < (uint64_t)numEntriesIOQ) {
            LOG_NRM("Changing number of Q elements from %d to %lld",
                numEntriesIOQ, (unsigned long long)maxIOQEntries);
            numEntriesIOQ = maxIOQEntries;
        }

        uint8_t iocqes = (gInformative->GetIdentifyCmdCtrlr()->
            GetValue(IDCTRLRCAP_CQES) & 0xf);
        uint8_t iosqes = (gInformative->GetIdentifyCmdCtrlr()->
            GetValue(IDCTRLRCAP_SQES) & 0xf);
        gCtrlrConfig->SetIOCQES(iocqes);
        gCtrlrConfig->SetIOSQES(iosqes);
        if (Queues::SupportDiscontigIOQ() == true) {
             SharedMemBufferPtr iocqBackedMem =
                 SharedMemBufferPtr(new MemBuffer());
             iocqBackedMem->InitOffset1stPage
                 ((numEntriesIOQ * (1 << iocqes)), 0, true);
             Queues::CreateIOCQDiscontigToHdw(mGrpName, mTestName,
                 CALC_TIMEOUT_ms(1), asq, acq, IOQ_ID, numEntriesIOQ, true,
                 IOCQ_GROUP_ID, true, 1, iocqBackedMem);

             SharedMemBufferPtr iosqBackedMem =
                 SharedMemBufferPtr(new MemBuffer());
             iosqBackedMem->InitOffset1stPage
                 ((numEntriesIOQ * (1 << iosqes)), 0, true);
             Queues::CreateIOSQDiscontigToHdw(mGrpName, mTestName,
                 CALC_TIMEOUT_ms(1), asq, acq, IOQ_ID, numEntriesIOQ, true,
                 IOSQ_GROUP_ID, IOQ_ID, 0, iosqBackedMem);
         } else {
            Queues::CreateIOCQContigToHdw(mGrpName, mTestName,
                CALC_TIMEOUT_ms(1), asq, acq, IOQ_ID, numEntriesIOQ, true,
                IOCQ_GROUP_ID, true, 1);

            Queues::CreateIOSQContigToHdw(mGrpName, mTestName,
                CALC_TIMEOUT_ms(1), asq, acq, IOQ_ID, numEntriesIOQ, true,
                IOSQ_GROUP_ID, IOQ_ID, 0);
         }
    }
}

}   // namespace
