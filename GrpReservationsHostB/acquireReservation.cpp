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

#include "acquireReservation.h"
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
#include "../Cmds/reservationAcquire.h"
#include "../Cmds/read.h"
#include "../Cmds/write.h"
namespace GrpReservationsHostB {


AcquireReservation::AcquireReservation(
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


AcquireReservation::~AcquireReservation()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


AcquireReservation::
AcquireReservation(const AcquireReservation &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


AcquireReservation &
AcquireReservation::operator=(const AcquireReservation &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


Test::RunType
AcquireReservation::RunnableCoreTest(bool preserve)
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
AcquireReservation::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * None.
     * \endverbatim
     */
	LOG_NRM("Start AcquireReservation::RunCoreTest");

	SharedIOSQPtr iosq = CAST_TO_IOSQ(gRsrcMngr->GetObj(IOSQ_GROUP_ID));
	SharedIOCQPtr iocq = CAST_TO_IOCQ(gRsrcMngr->GetObj(IOCQ_GROUP_ID));
	SharedASQPtr   asq = CAST_TO_ASQ(gRsrcMngr->GetObj(ASQ_GROUP_ID));
	SharedACQPtr   acq = CAST_TO_ACQ(gRsrcMngr->GetObj(ACQ_GROUP_ID));

	SharedWritePtr writeCmd = SharedWritePtr(new Write());
	SharedReadPtr readCmd = SharedReadPtr(new Read());

	SharedMemBufferPtr writeRegKey = SharedMemBufferPtr(new MemBuffer());
	SharedMemBufferPtr lbaWriteBuffer = SharedMemBufferPtr(new MemBuffer());
	SharedMemBufferPtr lbaReadBuffer = SharedMemBufferPtr(new MemBuffer());
	uint8_t keyToRegister[16];
	uint32_t memAlignment = sysconf(_SC_PAGESIZE);

	LOG_NRM("Create ReservationAcquire Cmd and attempt to acquire NSID using wrong key (0xFA versus current 0xAD");
	SharedReservationAcquirePtr reservationAcquireCmd = SharedReservationAcquirePtr(new ReservationAcquire());
	reservationAcquireCmd->SetNSID(1);
	reservationAcquireCmd->SetRTYPE(2);
	reservationAcquireCmd->SetIEKEY(0);
	reservationAcquireCmd->SetRACQA(0);
	for(uint8_t keyIndex = 0; keyIndex < 8;  keyIndex++) keyToRegister[keyIndex] = 0xFA;
	writeRegKey->InitAlignment(8, memAlignment, false, 0x0, keyToRegister); // 0xAD should be current key...
	reservationAcquireCmd->SetPrpBuffer( (send_64b_bitmask)MASK_PRP1_PAGE, writeRegKey);
    IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), iosq, iocq, reservationAcquireCmd, "Acquire NSID using wrong key", true, CESTAT_RSRV_CONFLICT);

	LOG_NRM("Create ReservationAcquire Cmd and attempt to acquire NSID using right key (0xAD");
	for(uint8_t keyIndex = 0; keyIndex < 8;  keyIndex++) keyToRegister[keyIndex] = 0xAD;
	writeRegKey->InitAlignment(8, memAlignment, false, 0x0, keyToRegister); // 0xAD should be current key...
	reservationAcquireCmd->SetPrpBuffer( (send_64b_bitmask)MASK_PRP1_PAGE, writeRegKey);
    IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), iosq, iocq, reservationAcquireCmd, "Acquire NSID using right key", true, CESTAT_SUCCESS);

	LOG_NRM("Create nvmeWrite Cmd and write 1 block of data to LBA 5, expecting a pass for HostA");
	lbaWriteBuffer->Init(512, true, 0xCC);
	writeCmd->SetPrpBuffer( (send_64b_bitmask)( MASK_PRP1_PAGE | MASK_PRP2_PAGE), lbaWriteBuffer);
	writeCmd->SetNSID(1);
	writeCmd->SetSLBA(5);
	writeCmd->SetNLB(0); // 0's based!
    IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), iosq, iocq, writeCmd, "write 0xCC's to LBA5", true, CESTAT_SUCCESS);

	LOG_NRM("Create nvmeRead Cmd and read back 1 block of data to LBA 5, expecting a pass for HostA");
	lbaReadBuffer->Init(512, true, 0x00);
	readCmd->SetPrpBuffer( (send_64b_bitmask) (MASK_PRP1_PAGE | MASK_PRP2_PAGE), lbaReadBuffer);
	readCmd->SetNSID(1);
	readCmd->SetSLBA(5);
	readCmd->SetNLB(0); // 0's based!
    IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), iosq, iocq, readCmd, "read from LBA5", true, CESTAT_SUCCESS);

    LOG_NRM("Ensure the data read back matches the expected data written (0xCC's)");
	if (lbaWriteBuffer->Compare(lbaReadBuffer) == false) {
		LOG_NRM("Data MISMATCH!!!");
		lbaWriteBuffer->Dump(
			FileSystem::PrepDumpFile(mGrpName, mTestName, "Write Data"),
			"write after acquire");
		lbaReadBuffer->Dump(
			FileSystem::PrepDumpFile(mGrpName, mTestName, "Read Data"),
			"read after acquire");
		throw FrmwkEx(HERE, "Data miscompare");
	}

    LOG_NRM("Completed AcquireReservation::RunCoreTest");
}

}   // namespace
