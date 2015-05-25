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

#include "allCtrlRegs_r12.h"
#include "globals.h"

namespace GrpCtrlRegisters {


AllCtrlRegs_r12::AllCtrlRegs_r12(
    string grpName, string testName) :
    Test(grpName, testName, SPECREV_12), AllCtrlRegs_r10b(grpName, testName)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.2, section 3");
    mTestDesc.SetShort(     "Validate all controller registers syntactically");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Validates the following; the RO fields which are not implementation "
        "specific contain default values; The RO fields cannot be written; All "
        "ASCII fields only contain chars 0x20 to 0x7e.");
}


AllCtrlRegs_r12::~AllCtrlRegs_r12()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


AllCtrlRegs_r12::AllCtrlRegs_r12(const AllCtrlRegs_r12 &other) :
    Test(other), AllCtrlRegs_r10b(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


AllCtrlRegs_r12 &
AllCtrlRegs_r12::operator=(const AllCtrlRegs_r12 &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


Test::RunType
AllCtrlRegs_r12::RunnableCoreTest(bool preserve)
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
AllCtrlRegs_r12::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) none
     *  \endverbatim
     */
    if (gCtrlrConfig->SetState(ST_DISABLE_COMPLETELY) == false)
        throw FrmwkEx(HERE);

    ValidateDefaultValues();
    ValidateROBitsAfterWriting();
}


}   // namespace


