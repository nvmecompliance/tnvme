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

#include "ctrlrResetIOQDeleted_r10b.h"
#include "globals.h"
#include "../Utils/kernelAPI.h"


CtrlrResetIOQDeleted_r10b::CtrlrResetIOQDeleted_r10b(int fd, string grpName,
    string testName, ErrorRegs errRegs) :
    Test(fd, grpName, testName, SPECREV_10b, errRegs)
{
    // 66 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 7");
    mTestDesc.SetShort(     "Ctrlr resets causes IOSQ & IOCQ's to be deleted");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Create a ratio of 2 IOSQ's associated with a single IOCQ, cause "
        "CC.EN=0, then verify all Q's were deleted by attempting to recreate "
        "those same Q's with identical parameters and verifying the successful "
        "CE within the ACQ. It succeeds because they are not duplicates and "
        "thus are allowed to be created again.");
}


CtrlrResetIOQDeleted_r10b::~CtrlrResetIOQDeleted_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


CtrlrResetIOQDeleted_r10b::
CtrlrResetIOQDeleted_r10b(const CtrlrResetIOQDeleted_r10b &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


CtrlrResetIOQDeleted_r10b &
CtrlrResetIOQDeleted_r10b::operator=(const CtrlrResetIOQDeleted_r10b &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


bool
CtrlrResetIOQDeleted_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) This is the 1st within GrpResets.
     * 2) The NVME device is disabled completely.
     * 3) All interrupts are disabled.
     *  \endverbatim
     */

     // Place local variables here

    return true;
}
