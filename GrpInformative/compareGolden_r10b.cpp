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

#include "boost/format.hpp"
#include <unistd.h>
#include "grpDefs.h"
#include "globals.h"
#include "compareGolden_r10b.h"
#include "../Cmds/identify.h"
#include "../Utils/io.h"

namespace GrpInformative {


CompareGolden_r10b::CompareGolden_r10b(int fd, string grpName,
    string testName, ErrorRegs errRegs) :
    Test(fd, grpName, testName, SPECREV_10b, errRegs)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 5");
    mTestDesc.SetShort(     "Compare golden identify cmd data");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Parse user supplied identify cmd data via the cmd line option "
        "\"--golden\" and issue identical cmds to the DUT and compare the"
        "correlating payload. Any miscompare constitutes  a test failure.");
}


CompareGolden_r10b::~CompareGolden_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


CompareGolden_r10b::
CompareGolden_r10b(const CompareGolden_r10b &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


CompareGolden_r10b &
CompareGolden_r10b::operator=(const CompareGolden_r10b &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


void
CompareGolden_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) Test CreateResources_r10b has run prior.
     *  \endverbatim
     */
    string work;

    // Lookup objs which were created in a prior test within group
    SharedASQPtr asq = CAST_TO_ASQ(gRsrcMngr->GetObj(ASQ_GROUP_ID))
    SharedACQPtr acq = CAST_TO_ACQ(gRsrcMngr->GetObj(ACQ_GROUP_ID))

    const Golden golden = gInformative->GetGoldenIdentify();
    if (golden.req == false) {
        LOG_NRM("No golden identify data supplied, nothing to compare");
        return;
    }

    SharedIdentifyPtr idCmd = SharedIdentifyPtr(new Identify());
    SharedMemBufferPtr idMem = SharedMemBufferPtr(new MemBuffer());
    idMem->InitAlignment(Identify::IDEAL_DATA_SIZE, sizeof(uint64_t), true, 0);
    send_64b_bitmask prpReq =
        (send_64b_bitmask)(MASK_PRP1_PAGE | MASK_PRP2_PAGE);
    idCmd->SetPrpBuffer(prpReq, idMem);

    for (size_t i = 0; i < golden.cmds.size(); i++) {
        LOG_DBG("Identify cmd #%ld", i);
        LOG_DBG("  Identify:DW1.nsid = 0x%02x", golden.cmds[i].nsid);
        LOG_DBG("  Identify.DW10.cns = %c", golden.cmds[i].cns ? 'T' : 'F');
        LOG_DBG("  sizeof(Identify.raw) = %ld", golden.cmds[i].raw.size());

        LOG_NRM("Formulate an identical idenitfy cmd to issue");
        idCmd->SetCNS(golden.cmds[i].cns);
        idCmd->SetNSID(golden.cmds[i].nsid);

        idMem->InitAlignment(Identify::IDEAL_DATA_SIZE, sizeof(uint64_t),
            true, 0);
        work = str(boost::format("IdCmd%d") % i);
        IO::SendAndReapCmd(mGrpName, mTestName, DEFAULT_CMD_WAIT_ms, asq, acq,
            idCmd, work, false);

        if (idMem->Compare(golden.cmds[i].raw) == false) {
            idMem->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName,
                "identify", "dut.miscompare"), "DUT data miscompare");
            SharedMemBufferPtr userMem = SharedMemBufferPtr(
                new MemBuffer(golden.cmds[i].raw));
            userMem->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName,
                "identify", "user.miscompare"), "Golden user data miscompare");
            throw FrmwkEx(HERE, "Golden identify data miscompare");
        }
    }
}

}   // namespace
