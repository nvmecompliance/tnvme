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

#include "readWriteToUnacquiredReservation.h"
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
#include "../Cmds/read.h"
#include "../Cmds/write.h"
namespace GrpReservationsHostB {


ReadWriteToUnacquiredReservation::ReadWriteToUnacquiredReservation(
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


ReadWriteToUnacquiredReservation::~ReadWriteToUnacquiredReservation()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


ReadWriteToUnacquiredReservation::
ReadWriteToUnacquiredReservation(const ReadWriteToUnacquiredReservation &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


ReadWriteToUnacquiredReservation &
ReadWriteToUnacquiredReservation::operator=(const ReadWriteToUnacquiredReservation &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


Test::RunType
ReadWriteToUnacquiredReservation::RunnableCoreTest(bool preserve)
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
ReadWriteToUnacquiredReservation::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * None.
     * \endverbatim
     */
	LOG_NRM("Start ReadWriteToUnacquiredReservation::RunCoreTest");

	SharedIOSQPtr iosq = CAST_TO_IOSQ(gRsrcMngr->GetObj(IOSQ_GROUP_ID));
	SharedIOCQPtr iocq = CAST_TO_IOCQ(gRsrcMngr->GetObj(IOCQ_GROUP_ID));
	SharedASQPtr   asq = CAST_TO_ASQ(gRsrcMngr->GetObj(ASQ_GROUP_ID));
	SharedACQPtr   acq = CAST_TO_ACQ(gRsrcMngr->GetObj(ACQ_GROUP_ID));

	SharedWritePtr writeCmd = SharedWritePtr(new Write());
	SharedReadPtr readCmd = SharedReadPtr(new Read());

	SharedMemBufferPtr lbaWriteBuffer = SharedMemBufferPtr(new MemBuffer());
	SharedMemBufferPtr lbaReadBuffer = SharedMemBufferPtr(new MemBuffer());

	LOG_NRM("Create nvmeWrite Cmd and write 1 block of data to LBA 5, expecting 0:0x83 for HostB");
	lbaWriteBuffer->Init(512, true, 0xDD);
	writeCmd->SetPrpBuffer( (send_64b_bitmask)( MASK_PRP1_PAGE | MASK_PRP2_PAGE), lbaWriteBuffer);
	writeCmd->SetNSID(1);
	writeCmd->SetSLBA(5);
	writeCmd->SetNLB(0); // 0's based!
    IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), iosq, iocq, writeCmd, "write 0xDD's to LBA5 using hostB", true, CESTAT_RSRV_CONFLICT);

	LOG_NRM("Create nvmeRead Cmd and read back 1 block of data to LBA 5, expecting 0:0x83 for HostA");
	lbaReadBuffer->Init(512, true, 0x00);
	readCmd->SetPrpBuffer( (send_64b_bitmask) (MASK_PRP1_PAGE | MASK_PRP2_PAGE), lbaReadBuffer);
	readCmd->SetNSID(1);
	readCmd->SetSLBA(5);
	readCmd->SetNLB(0); // 0's based!
    IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), iosq, iocq, readCmd, "read from LBA5 using hostB", true, CESTAT_RSRV_CONFLICT);

    LOG_NRM("Completed ReadWriteToUnacquiredReservation::RunCoreTest");
}

}   // namespace
