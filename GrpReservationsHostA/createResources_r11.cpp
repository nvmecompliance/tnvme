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

#include "createResources_r11.h"
#include "globals.h"
#include "grpDefs.h"
#include "../Singletons/informative.h"
#include "../Queues/acq.h"
#include "../Queues/asq.h"
#include "../Queues/iocq.h"
#include "../Queues/iosq.h"
#include "../Utils/kernelAPI.h"
#include "../Utils/queues.h"
#include "../Utils/irq.h"


namespace GrpReservationsHostA {

static uint32_t NumEntriesIOQ =     2;

CreateResources_r11::CreateResources_r11(
    string grpName, string testName) :
    Test(grpName, testName, SPECREV_11)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 5");
    mTestDesc.SetShort(     "Create resources needed by subsequent tests");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Create resources with group lifetime which are needed by subsequent "
        "tests");
}


CreateResources_r11::~CreateResources_r11()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


CreateResources_r11::
CreateResources_r11(const CreateResources_r11 &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


CreateResources_r11 &
CreateResources_r11::operator=(const CreateResources_r11 &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


Test::RunType
CreateResources_r11::RunnableCoreTest(bool preserve)
{
    ///////////////////////////////////////////////////////////////////////////
    // All code contained herein must never permanently modify the state or
    // configuration of the DUT. Permanence is defined as state or configuration
    // changes that will not be restored after a cold hard reset.
    ///////////////////////////////////////////////////////////////////////////

    ConstSharedIdentifyPtr idCtrlrCap = gInformative->GetIdentifyCmdCtrlr();
    uint64_t oncs = idCtrlrCap->GetValue(IDCTRLRCAP_ONCS);
    if ((oncs & ONCS_SUP_RSRV) == 0) {
        LOG_NRM("Reporting Reservations not supported (oncs)%ld", oncs);
        return RUN_FALSE;
    }

    preserve = preserve;    // Suppress compiler error/warning
    return RUN_TRUE;        // This test is never destructive
}


void
CreateResources_r11::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * None.
     * \endverbatim
     */
    if (gCtrlrConfig->SetState(ST_DISABLE_COMPLETELY) == false)
        throw FrmwkEx(HERE);

    SharedACQPtr acq = CAST_TO_ACQ(gRsrcMngr->AllocObj(Trackable::OBJ_ACQ, ACQ_GROUP_ID))
    acq->Init(5);

    SharedASQPtr asq = CAST_TO_ASQ(gRsrcMngr->AllocObj(Trackable::OBJ_ASQ, ASQ_GROUP_ID))
    asq->Init(5);

    // All queues will use identical IRQ vector
    IRQ::SetAnySchemeSpecifyNum(1);     // throws upon error

    gCtrlrConfig->SetCSS(CtrlrConfig::CSS_NVM_CMDSET);
    if (gCtrlrConfig->SetState(ST_ENABLE) == false)
        throw FrmwkEx(HERE);

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

	gCtrlrConfig->SetIOCQES(gInformative->GetIdentifyCmdCtrlr()->
		GetValue(IDCTRLRCAP_CQES) & 0xf);
	Queues::CreateIOCQContigToHdw(mGrpName, mTestName,
		CALC_TIMEOUT_ms(1), asq, acq, IOQ_ID, NumEntriesIOQ, true,
		IOCQ_GROUP_ID, true, 0);

	gCtrlrConfig->SetIOSQES(gInformative->GetIdentifyCmdCtrlr()->
		GetValue(IDCTRLRCAP_SQES) & 0xf);
	Queues::CreateIOSQContigToHdw(mGrpName, mTestName,
		CALC_TIMEOUT_ms(1), asq, acq, IOQ_ID, NumEntriesIOQ, true,
		IOSQ_GROUP_ID, IOQ_ID, 0);
}

}   // namespace
