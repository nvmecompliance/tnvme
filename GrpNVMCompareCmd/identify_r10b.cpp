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

#include "identify_r10b.h"
#include "globals.h"

namespace GrpNVMCompareCmd {


Identify_r10b::Identify_r10b(int fd, string grpName, string testName,
    ErrorRegs errRegs) :
    Test(fd, grpName, testName, SPECREV_10b, errRegs)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 5");
    mTestDesc.SetShort(     "Verify this optional cmd is supported");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Reference Identify.ONCS; If the command is supported then return "
        "success, otherwise return failure If the DUT doesn't support this "
        "optional feature, then utilize "
        "\"tnvme --skiptest YourSkiptestFilename.cfg\" to cause this group of "
        "tests to be skipped.");
}


Identify_r10b::~Identify_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


Identify_r10b::
Identify_r10b(const Identify_r10b &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


Identify_r10b &
Identify_r10b::operator=(const Identify_r10b &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


Test::RunType
Identify_r10b::RunnableCoreTest(bool preserve)
{
    preserve = preserve;    // Suppress compiler error/warning
    return RUN_TRUE;        // This test is never destructive
}


void
Identify_r10b::RunCoreTest()
{
   /** \verbatim
     * Assumptions:
     * 1) This is the 1st within GrpNVMCompareCmd.
     *
     * NOTE: To validate whether a DUT is suppose to support or not support an
     *       optional feature, the cmd line option --golden should be used.
     *       The reason this test is present as the 1st within any optional
     *       grouping of tests is to force a development team to make a
     *       decision as to whether these tests should be included or skipped
     *       based upon their specific hdw requirements. This way nothing will
     *       be missed by accident. The assumption is that every optional
     *       feature is support until told otherwise by the cmd line option
     *       --skiptest.
     * \endverbatim
     */
    ConstSharedIdentifyPtr idCtrlrCap = gInformative->GetIdentifyCmdCtrlr();
    uint64_t oncs = idCtrlrCap->GetValue(IDCTRLRCAP_ONCS);
    if ((oncs & ONCS_SUP_COMP_CMD) == 0)
        throw FrmwkEx(HERE, "Optional feature not supported, see this source");
}

}   // namespace
