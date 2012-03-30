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

#include "prpOffsetSinglePgSingleBlk_r10b.h"
#include "globals.h"
#include "grpDefs.h"

namespace GrpNVMWriteReadCombo {


PRPOffsetSinglePgSingleBlk_r10b::PRPOffsetSinglePgSingleBlk_r10b(int fd,
    string mGrpName, string mTestName, ErrorRegs errRegs) :
    Test(fd, mGrpName, mTestName, SPECREV_10b, errRegs)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 6");
    mTestDesc.SetShort(     "Vary buffer offset for single-page/single-blk "
                            "against PRP1/PRP2.");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Search for 1 of the following namspcs to run test. Find 1st bare "
        "namspc, or find 1st meta namspc, or find 1st E2E namspc. Issue "
        "identical write cmd starting at LBA 0, sending a single block with "
        "approp meta/E2E requirements if necessary, where write executes the "
        "ALGO listed below; subsequently after each write a read must verify "
        "the data pattern for success, any metadata will also need to be "
        "verified for it will have the same data pattern as the data. ALGO) "
        "Alloc discontig memory; vary offset into 1st memory page from 0 to X, "
        "where X = (CC.MPS - Identify.LBAF[Identify.FLBAS].LBADS) in steps of "
        "4B; alternate the data pattern between byteK, word++ for each write. "
        "NOTE: Since PRP1 is the only effective ptr, then PRP2 becomes "
        "reserved. Reserved fields must not be checked by the recipient. "
        "Verify this fact by injecting random 64b values into PRP2 by "
        "alternating random number, then zero, for each and every write/read "
        "cmd issued to the DUT. Always initiate the random gen with seed 17 "
        "before this test starts.");
}


PRPOffsetSinglePgSingleBlk_r10b::~PRPOffsetSinglePgSingleBlk_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


PRPOffsetSinglePgSingleBlk_r10b::
PRPOffsetSinglePgSingleBlk_r10b(const PRPOffsetSinglePgSingleBlk_r10b &other) :
    Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


PRPOffsetSinglePgSingleBlk_r10b &
PRPOffsetSinglePgSingleBlk_r10b::operator=(const PRPOffsetSinglePgSingleBlk_r10b
    &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


void
PRPOffsetSinglePgSingleBlk_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) Test CreateResources_r10b has run prior.
     * \endverbatim
     */

}


}   // namespace
