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
#include "protInfoIgnoreBare_r10b.h"
#include "globals.h"
#include "grpDefs.h"
#include "../Queues/iocq.h"
#include "../Queues/iosq.h"
#include "../Utils/io.h"
#include "../Cmds/write.h"

namespace GrpNVMWriteCmd {


ProtInfoIgnoreBare_r10b::ProtInfoIgnoreBare_r10b(
    string grpName, string testName) :
    Test(grpName, testName, SPECREV_10b)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 4,6");
    mTestDesc.SetShort(     "Verify protection info (PRINFO) is ignored for bare namspc");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "For all bare namspcs from Identify.NN; For each namspc issue multiple "
        "write cmds where each is writing 1 data block at LBA 0 and vary the "
        "values of DW12.PRINFO.PRACT from 0x0 to 0x0f, expect success for "
        "all.");
}


ProtInfoIgnoreBare_r10b::~ProtInfoIgnoreBare_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


ProtInfoIgnoreBare_r10b::
ProtInfoIgnoreBare_r10b(const ProtInfoIgnoreBare_r10b &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


ProtInfoIgnoreBare_r10b &
ProtInfoIgnoreBare_r10b::operator=(const ProtInfoIgnoreBare_r10b &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


Test::RunType
ProtInfoIgnoreBare_r10b::RunnableCoreTest(bool preserve)
{
    ///////////////////////////////////////////////////////////////////////////
    // All code contained herein must never permanently modify the state or
    // configuration of the DUT. Permanence is defined as state or configuration
    // changes that will not be restored after a cold hard reset.
    ///////////////////////////////////////////////////////////////////////////

    return ((preserve == true) ? RUN_FALSE : RUN_TRUE);   // Test is destructive
}


void
ProtInfoIgnoreBare_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * None.
     * \endverbatim
     */
    char context[256];
    ConstSharedIdentifyPtr namSpcPtr;

    // Lookup objs which were created in a prior test within group
    SharedIOSQPtr iosq = CAST_TO_IOSQ(gRsrcMngr->GetObj(IOSQ_GROUP_ID));
    SharedIOCQPtr iocq = CAST_TO_IOCQ(gRsrcMngr->GetObj(IOCQ_GROUP_ID));

    vector<uint32_t> bare = gInformative->GetBareNamespaces();
    for (size_t i = 0; i < bare.size(); i++) {
        namSpcPtr = gInformative->GetIdentifyCmdNamspc(bare[i]);

        LOG_NRM("Create memory to contain write payload");
        SharedMemBufferPtr writeMem = SharedMemBufferPtr(new MemBuffer());
        uint64_t lbaDataSize = namSpcPtr->GetLBADataSize();
        writeMem->Init(lbaDataSize);

        LOG_NRM("Create a write cmd to read data from namspc %d", bare[i]);
        SharedWritePtr writeCmd = SharedWritePtr(new Write());
        send_64b_bitmask prpBitmask = (send_64b_bitmask)
            (MASK_PRP1_PAGE | MASK_PRP2_PAGE | MASK_PRP2_LIST);
        writeCmd->SetPrpBuffer(prpBitmask, writeMem);
        writeCmd->SetNSID(bare[i]);
        writeCmd->SetNLB(0);    // convert to 0-based value

        for (uint16_t protInfo = 0; protInfo <= 0x0f; protInfo++) {
            uint8_t work = writeCmd->GetByte(12, 3);
            work &= ~0x3c;  // PRINFO specific bits
            work |= (protInfo << 2);
            writeCmd->SetByte(work, 12, 3);

            snprintf(context, sizeof(context), "ns%d.protInfo0x%02X",
                (uint32_t)i, protInfo);
            IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), iosq,
                iocq, writeCmd, context, true);
        }
    }
}


}   // namespace
