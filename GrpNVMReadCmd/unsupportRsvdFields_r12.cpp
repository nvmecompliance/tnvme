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

#include <boost/utility/binary.hpp>

#include "unsupportRsvdFields_r12.h"
#include "globals.h"
#include "grpDefs.h"
#include "../Queues/iocq.h"
#include "../Queues/iosq.h"
#include "../Utils/io.h"

namespace GrpNVMReadCmd {


UnsupportRsvdFields_r12::UnsupportRsvdFields_r12(
    string grpName, string testName) :
    Test(grpName, testName, SPECREV_12)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.2, section 6.9");
    mTestDesc.SetShort(     "Set unsupported/rsvd fields in cmd");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Search for 1 of the following namspcs to run test. Find 1st bare "
        "namspc, or find 1st meta namspc, or find 1st E2E namspc. Unsupported "
        "DW's and rsvd fields are treated identical, the recipient is not "
        "required to check their value. Receipt of reserved coded values shall "
        "be reported as an error. Issue a read cmd requesting 1 block and "
        "approp supporting meta/E2E if necessary from the selected namspc at "
        "LBA 0, expect success. Issue same cmd setting all unsupported/rsvd "
        "fields, expect success. Set: DW0_b14:10, DW2, DW3, DW12_b25:16, "
        "DW13_b31:8. Issue same cmd setting all rsvd coded values, expect "
        "fail. Set: DW13_b3:0");
}


UnsupportRsvdFields_r12::~UnsupportRsvdFields_r12()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


UnsupportRsvdFields_r12::
UnsupportRsvdFields_r12(const UnsupportRsvdFields_r12 &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


UnsupportRsvdFields_r12 &
UnsupportRsvdFields_r12::operator=(const UnsupportRsvdFields_r12 &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


Test::RunType
UnsupportRsvdFields_r12::RunnableCoreTest(bool preserve)
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
UnsupportRsvdFields_r12::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) Test CreateResources_r10b has run prior.
     * \endverbatim
     */

    // Lookup objs which were created in a prior test within group
    SharedIOSQPtr iosq = CAST_TO_IOSQ(gRsrcMngr->GetObj(IOSQ_GROUP_ID));
    SharedIOCQPtr iocq = CAST_TO_IOCQ(gRsrcMngr->GetObj(IOCQ_GROUP_ID));

    SharedReadPtr readCmd = CreateCmd();
    IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), iosq, iocq,
        readCmd, "none.set", true);

    LOG_NRM("Set all cmd's rsvd bits");
    uint32_t work = readCmd->GetDword(0);
    work |= 0x00003c00;      // Set DW0_b13:10 bits
    readCmd->SetDword(work, 0);

    readCmd->SetDword(0xffffffff, 2);
    readCmd->SetDword(0xffffffff, 3);

    work = readCmd->GetDword(12);
    work |= 0x03ff0000;      // Set DW12_b25:16 bits
    readCmd->SetDword(work, 12);

    work = readCmd->GetDword(13);
    work |= 0xffffff00;     // Set DW13_b31:8 bits
    readCmd->SetDword(work, 13);

    IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), iosq, iocq,
        readCmd, "all.set", true);

    LOG_NRM("Set DSM field reserved coded values");
    uint32_t cdw13 = readCmd->GetDword(13) & ~0xf;
    for (int accFreq = BOOST_BINARY(111); accFreq <= BOOST_BINARY(1111);
        ++accFreq) {
        work = cdw13 | accFreq;
        readCmd->SetDword(work, 13);

        /* Controller may ignore context attributes */
        IO::SendAndReapCmdIgnore(mGrpName, mTestName, CALC_TIMEOUT_ms(1), iosq,
            iocq, readCmd, "all.set", true);
    }
}


SharedReadPtr
UnsupportRsvdFields_r12::CreateCmd()
{
    Informative::Namspc namspcData = gInformative->Get1stBareMetaE2E();
    LBAFormat lbaFormat = namspcData.idCmdNamspc->GetLBAFormat();
    LOG_NRM("Processing read cmd using namspc id %d", namspcData.id);

    ConstSharedIdentifyPtr namSpcPtr = namspcData.idCmdNamspc;
    uint64_t lbaDataSize = namSpcPtr->GetLBADataSize();;
    SharedMemBufferPtr dataPat = SharedMemBufferPtr(new MemBuffer());

    SharedReadPtr readCmd = SharedReadPtr(new Read());
    send_64b_bitmask prpBitmask = (send_64b_bitmask)(MASK_PRP1_PAGE
        | MASK_PRP2_PAGE | MASK_PRP2_LIST);

    switch (namspcData.type) {
    case Informative::NS_BARE:
        dataPat->Init(lbaDataSize);
        break;
    case Informative::NS_METAS:
        dataPat->Init(lbaDataSize);
        if (gRsrcMngr->SetMetaAllocSize(lbaFormat.MS) == false)
            throw FrmwkEx(HERE);
        readCmd->AllocMetaBuffer();
        break;
    case Informative::NS_METAI:
        dataPat->Init(lbaDataSize + lbaFormat.MS);
        break;
    case Informative::NS_E2ES:
    case Informative::NS_E2EI:
        throw FrmwkEx(HERE, "Deferring work to handle this case in future");
        break;
    }

    readCmd->SetPrpBuffer(prpBitmask, dataPat);
    readCmd->SetNSID(namspcData.id);
    readCmd->SetNLB(0);
    return readCmd;
}


}   // namespace

