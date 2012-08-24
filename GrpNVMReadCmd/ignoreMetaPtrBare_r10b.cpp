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
#include "ignoreMetaPtrBare_r10b.h"
#include "globals.h"
#include "grpDefs.h"
#include "../Queues/iocq.h"
#include "../Queues/iosq.h"
#include "../Utils/io.h"
#include "../Cmds/read.h"

namespace GrpNVMReadCmd {


IgnoreMetaPtrBare_r10b::IgnoreMetaPtrBare_r10b(
    string grpName, string testName) :
    Test(grpName, testName, SPECREV_10b)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 4,6");
    mTestDesc.SetShort(     "Verify metadata ptr is not used for bare namspc");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "MPTR is only used if metadata is not interleaved with the data. For "
        "all bare namspcs from Identify.NN issue a single read cmd requesting "
        "1 data block at LBA 0; set the meta ptr to max value, expect "
        "success.");
}


IgnoreMetaPtrBare_r10b::~IgnoreMetaPtrBare_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


IgnoreMetaPtrBare_r10b::
IgnoreMetaPtrBare_r10b(const IgnoreMetaPtrBare_r10b &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


IgnoreMetaPtrBare_r10b &
IgnoreMetaPtrBare_r10b::operator=(const IgnoreMetaPtrBare_r10b &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


Test::RunType
IgnoreMetaPtrBare_r10b::RunnableCoreTest(bool preserve)
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
IgnoreMetaPtrBare_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) Test CreateResources_r10b has run prior.
     * \endverbatim
     */
    string work;
    ConstSharedIdentifyPtr namSpcPtr;

    // Lookup objs which were created in a prior test within group
    SharedIOSQPtr iosq = CAST_TO_IOSQ(gRsrcMngr->GetObj(IOSQ_GROUP_ID));
    SharedIOCQPtr iocq = CAST_TO_IOCQ(gRsrcMngr->GetObj(IOCQ_GROUP_ID));

    LOG_NRM("For all bare namspc's issue cmd with non-zero meta ptr");
    vector<uint32_t> bare = gInformative->GetBareNamespaces();
    for (size_t i = 0; i < bare.size(); i++) {
        namSpcPtr = gInformative->GetIdentifyCmdNamspc(bare[i]);

        LOG_NRM("Setup read cmd's values that won't change per namspc");
        SharedMemBufferPtr readMem = SharedMemBufferPtr(new MemBuffer());
        uint64_t lbaDataSize = namSpcPtr->GetLBADataSize();
        readMem->Init(lbaDataSize);

        SharedReadPtr readCmd = SharedReadPtr(new Read());
        send_64b_bitmask prpBitmask = (send_64b_bitmask)
            (MASK_PRP1_PAGE | MASK_PRP2_PAGE | MASK_PRP2_LIST);
        readCmd->SetPrpBuffer(prpBitmask, readMem);
        readCmd->SetNLB(0);    // convert to 0-based value
        readCmd->SetSLBA(0);

        LOG_NRM("Set MPTR in cmd to max value");
        readCmd->SetDword(0xffffffff, 4);
        readCmd->SetDword(0xffffffff, 5);

        readCmd->SetNSID(bare[i]);
        work = str(boost::format("namspc%d") % bare[i]);
        IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), iosq, iocq,
            readCmd, work, true);
    }
}

}   // namespace

