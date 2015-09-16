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

#include "registerReservation.h"
#include "globals.h"
#include "grpDefs.h"
#include "../Queues/acq.h" 
#include "../Queues/asq.h"
#include "../Queues/iocq.h"
#include "../Queues/iosq.h"
#include "../Utils/kernelAPI.h"
#include "../Utils/irq.h"
#include "../Utils/queues.h"
#include "../Utils/io.h"
#include "../Singletons/informative.h"
#include "../Cmds/setFeatures.h"
#include "../Cmds/getFeatures.h"
#include "../Cmds/reservationRegister.h"

namespace GrpReservationsHostA {


RegisterReservation::RegisterReservation(
    string grpName, string testName) :
    Test(grpName, testName, SPECREV_11)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.1b");
    mTestDesc.SetShort(     "Register a reservation");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Using FID=0x81, register a reservation (set) and read it back (get)"
        "tests");
}


RegisterReservation::~RegisterReservation()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


RegisterReservation::
RegisterReservation(const RegisterReservation &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


RegisterReservation &
RegisterReservation::operator=(const RegisterReservation &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


Test::RunType
RegisterReservation::RunnableCoreTest(bool preserve)
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
RegisterReservation::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * None.
     * \endverbatim
     */
	LOG_NRM("Start RegisterReservation::RunCoreTest");

	SharedIOSQPtr iosq = CAST_TO_IOSQ(gRsrcMngr->GetObj(IOSQ_GROUP_ID));
	SharedIOCQPtr iocq = CAST_TO_IOCQ(gRsrcMngr->GetObj(IOCQ_GROUP_ID));
	SharedASQPtr   asq = CAST_TO_ASQ(gRsrcMngr->GetObj(ASQ_GROUP_ID));
	SharedACQPtr   acq = CAST_TO_ACQ(gRsrcMngr->GetObj(ACQ_GROUP_ID));
	//uint8_t keyToRegister[16];
	//uint32_t memAlignment = sysconf(_SC_PAGESIZE);
	CEStat retStat;

	LOG_NRM("Create Set features cmd to set HostID");
	SharedSetFeaturesPtr setFeaturesCmd = SharedSetFeaturesPtr(new SetFeatures());
	setFeaturesCmd->SetFID(0x81); // Host Identifier
    LOG_NRM("Create memory to contain HostID payload");
    SharedMemBufferPtr writeHostIDmem = SharedMemBufferPtr(new MemBuffer());
    // Init(uint32_t bufSize, bool initMem = false, uint8_t initVal = 0)
    writeHostIDmem->Init(8, true, 0xAA);
    //writeHostIDmem->InitAlignment(8, true, true, 0xAA); // HostID = 0xAAAAAAAAAAAAAAAA
    send_64b_bitmask prpBitmask = (send_64b_bitmask) MASK_PRP1_PAGE;
    setFeaturesCmd->SetPrpBuffer(prpBitmask, writeHostIDmem);
    IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), asq, acq, setFeaturesCmd, "Set 0xAA..AA as hostID", true, CESTAT_SUCCESS);

	LOG_NRM("Create Get features cmd to read back HostID");
	SharedGetFeaturesPtr getFeaturesCmd = SharedGetFeaturesPtr(new GetFeatures());
	getFeaturesCmd->SetFID(0x81); // Host Identifier
    SharedMemBufferPtr readHostIDmem = SharedMemBufferPtr(new MemBuffer());
    readHostIDmem->Init(8, true, 0x00); // HostID = 0xAAAAAAAAAAAAAAAA
    getFeaturesCmd->SetPrpBuffer(prpBitmask, readHostIDmem);
    retStat = IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), asq, acq, getFeaturesCmd, "Read back hostID", true, CESTAT_SUCCESS);

    // This should be returning back using APL FW... BUGBUG
    if(retStat != CESTAT_SUCCESS)
    {
    	LOG_NRM("Was unable to get back HostId after setting...");
    } else
    {
		LOG_NRM("Compare returned HostID to what was just previously set...");
		if (writeHostIDmem->Compare(readHostIDmem) == false)
		{
			LOG_NRM("HostID MISMATCH!!!");
			writeHostIDmem->Dump(
				FileSystem::PrepDumpFile(mGrpName, mTestName, "WriteHostID"),
				"setFeatures HostID");
			readHostIDmem->Dump(
				FileSystem::PrepDumpFile(mGrpName, mTestName, "ReadHostId"),
				"getFeatures HostID");
			throw FrmwkEx(HERE, "Data miscompare");
		}
    }

    // HostID should be set... now we can register a key, first we will try to release
	SharedMemBufferPtr writeRegKey = SharedMemBufferPtr(new MemBuffer());
	SharedReservationRegisterPtr reservationRegisterCmd = SharedReservationRegisterPtr(new ReservationRegister());

    LOG_NRM("Release any current key with IKEY=1. If pass we cleared a key, else was already clear.");
	reservationRegisterCmd->SetNSID(1);
	reservationRegisterCmd->SetCPTPL(0); // No PTPL change
	reservationRegisterCmd->SetIEKEY(1);
	reservationRegisterCmd->SetRREGA(1); // Unregister Key
	writeRegKey->Init(16, true, 0); // 0's in buffer, IKEY will ignore CKEY/NKEY
	reservationRegisterCmd->SetPrpBuffer(prpBitmask, writeRegKey);

	std::vector<CEStat> possibleReturnStatuses = {CESTAT_SUCCESS, CESTAT_RSRV_CONFLICT, CESTAT_IGNORE};
	retStat = IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), iosq, iocq, reservationRegisterCmd, "Release Any Key HostA", true, possibleReturnStatuses);
	switch(retStat) {
		case CESTAT_SUCCESS:
			LOG_NRM("Success status returned, a key was assumed to have been present and is now cleared.");
			break;
		case CESTAT_RSRV_CONFLICT:
			LOG_NRM("Rsrv Conflict status returned, a key was assumed have not been present to be able to be cleared.");
			break;
		default:
			LOG_NRM("Unknown stat returned back while attempting to unregister a potential exhisting key... continuing.");
	}

	LOG_NRM("Register our key (0xAE's), expecting pass");
	reservationRegisterCmd->SetNSID(1);
	reservationRegisterCmd->SetCPTPL(0); // No PTPL change
	reservationRegisterCmd->SetIEKEY(1);
	reservationRegisterCmd->SetRREGA(0); // Register Key
	writeRegKey->Init(16, true, 0xAE); // 0xAF's as arbitrary new key
	reservationRegisterCmd->SetPrpBuffer(prpBitmask, writeRegKey);
    retStat = IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), iosq, iocq, reservationRegisterCmd, "Register Key 0xAE HostA", true, CESTAT_SUCCESS);


	//LOG_NRM("Try to register (not replace) a new key. Should always fail even with IEKEY=1 and same key as before... Expecting Rsvr Conflict.");
	// Same command as before...
	/*
	reservationRegisterCmd->SetNSID(1);
	reservationRegisterCmd->SetCPTPL(0); // No PTPL change
	reservationRegisterCmd->SetIEKEY(1);
	reservationRegisterCmd->SetRREGA(0); // Register Key
	writeRegKey->InitAlignment(16, true, 0xAE); // 0xAF's as arbitrary new key
	reservationRegisterCmd->SetPrpBuffer(prpBitmask, writeRegKey);
	*/
    //retStat = IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), iosq, iocq, reservationRegisterCmd, "Register Key 0xAE HostA", true, CESTAT_RSRV_CONFLICT);

    LOG_NRM("Completed RegisterReservation::RunCoreTest");
}

}   // namespace
