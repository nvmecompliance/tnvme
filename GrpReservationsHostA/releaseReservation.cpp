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

#include "releaseReservation.h"
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
#include "../Cmds/reservationRelease.h"

namespace GrpReservationsHostA {


ReleaseReservation::ReleaseReservation(
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


ReleaseReservation::~ReleaseReservation()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


ReleaseReservation::
ReleaseReservation(const ReleaseReservation &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


ReleaseReservation &
ReleaseReservation::operator=(const ReleaseReservation &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


Test::RunType
ReleaseReservation::RunnableCoreTest(bool preserve)
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
ReleaseReservation::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * None.
     * \endverbatim
     */
	LOG_NRM("Start ReleaseReservation::RunCoreTest");

	/*
	SharedIOSQPtr iosq = CAST_TO_IOSQ(gRsrcMngr->GetObj(IOSQ_GROUP_ID));
	SharedIOCQPtr iocq = CAST_TO_IOCQ(gRsrcMngr->GetObj(IOCQ_GROUP_ID));
	SharedASQPtr   asq = CAST_TO_ASQ(gRsrcMngr->GetObj(ASQ_GROUP_ID));
	SharedACQPtr   acq = CAST_TO_ACQ(gRsrcMngr->GetObj(ACQ_GROUP_ID));

	SharedMemBufferPtr writeRegKey = SharedMemBufferPtr(new MemBuffer());
	send_64b_bitmask prpBitmask = (send_64b_bitmask) MASK_PRP1_PAGE;
	uint8_t keyToRegister[16];
	uint32_t memAlignment = sysconf(_SC_PAGESIZE);
    */

	/*
	LOG_NRM("Create ReservationRelease Cmd and attempt to replace previous key 0xAE to new key 0xAF, expect pass");
	SharedReservationReleasePtr releaseRegisterCmd = SharedReservationReleasePtr(new ReservationRelease());
	releaseRegisterCmd->SetNSID(1);
	releaseRegisterCmd->SetCPTPL(0);
	releaseRegisterCmd->SetIEKEY(0);
	releaseRegisterCmd->SetRREGA(2);
	for(uint8_t keyIndex = 0; keyIndex < 8;  keyIndex++) keyToRegister[keyIndex] = 0xAE;
	for(uint8_t keyIndex = 8; keyIndex < 16; keyIndex++) keyToRegister[keyIndex] = 0xAF;
	writeRegKey->InitAlignment(16, memAlignment, false, 0x0, keyToRegister); // 0xAE's -> 0xAF's as arbitrary new key
	releaseRegisterCmd->SetPrpBuffer(prpBitmask, writeRegKey);
    IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), iosq, iocq, releaseRegisterCmd, "Replace Key 0xAE to 0xAF for HostA", true, CESTAT_SUCCESS);

	LOG_NRM("Create ReservationRelease Cmd and attempt to replace previous key to 0xAD, and provide invalid CRKEY (0xAA instead of 0xAF), expect fail");
	releaseRegisterCmd->SetNSID(1);
	releaseRegisterCmd->SetCPTPL(0);
	releaseRegisterCmd->SetIEKEY(0);
	releaseRegisterCmd->SetRREGA(2);
	for(uint8_t keyIndex = 0; keyIndex < 8;  keyIndex++) keyToRegister[keyIndex] = 0xAA;
	for(uint8_t keyIndex = 8; keyIndex < 16; keyIndex++) keyToRegister[keyIndex] = 0xAD;
	writeRegKey->InitAlignment(16, memAlignment, false, 0x0, keyToRegister); // 0xAE's -> 0xAF's as arbitrary new key
	releaseRegisterCmd->SetPrpBuffer(prpBitmask, writeRegKey);
    IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), iosq, iocq, releaseRegisterCmd, "Invalid replace, wrong CRKEY", true, CESTAT_RSRV_CONFLICT);

	LOG_NRM("Create ReservationRelease Cmd and attempt to replace previous key to 0xAD, and provide invalid CRKEY (0xAA instead of 0xAF) but with IEKEY = 1, expect pass");
	releaseRegisterCmd->SetNSID(1);
	releaseRegisterCmd->SetCPTPL(0);
	releaseRegisterCmd->SetIEKEY(1); // IGNORE CRKEY
	releaseRegisterCmd->SetRREGA(2); // Replace key
    IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), iosq, iocq, releaseRegisterCmd, "Invalid replace, wrong CRKEY", true, CESTAT_SUCCESS);

	LOG_NRM("Create ReservationRelease Cmd and attempt to remove previous key to 0xAD, and provide invalid CRKEY (0xAA instead of 0xAF), expect fail");
	releaseRegisterCmd->SetNSID(1);
	releaseRegisterCmd->SetCPTPL(0);
	releaseRegisterCmd->SetIEKEY(0);
	releaseRegisterCmd->SetRREGA(1); // Remove
	for(uint8_t keyIndex = 0; keyIndex < 8;  keyIndex++) keyToRegister[keyIndex] = 0xAA;
	for(uint8_t keyIndex = 8; keyIndex < 16; keyIndex++) keyToRegister[keyIndex] = 0x00;
	writeRegKey->InitAlignment(16, memAlignment, false, 0x0, keyToRegister); // 0xAE's -> 0xAF's as arbitrary new key
	releaseRegisterCmd->SetPrpBuffer(prpBitmask, writeRegKey);
    IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), iosq, iocq, releaseRegisterCmd, "Invalid replace, wrong CRKEY", true, CESTAT_RSRV_CONFLICT);

    LOG_NRM("Completed ReleaseReservation::RunCoreTest")
    */
}

}   // namespace
