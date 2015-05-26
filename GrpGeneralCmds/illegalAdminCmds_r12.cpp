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

#include <initializer_list>
#include <boost/format.hpp>
#include "illegalAdminCmds_r12.h"
#include "globals.h"
#include "grpDefs.h"
#include "../Queues/iocq.h"
#include "../Queues/iosq.h"
#include "../Utils/io.h"
#include "../Cmds/adminCmd.h"


namespace GrpGeneralCmds {


const uint8_t IllegalAdminCmds_r12::NAMESPACE_MNGMT_OPCODE     = 0x0D;
const uint8_t IllegalAdminCmds_r12::NAMESPACE_ATTACH_OPCODE    = 0x15;


IllegalAdminCmds_r12::IllegalAdminCmds_r12(string grpName, string testName) :
    Test(grpName, testName, SPECREV_12),
        IllegalAdminCmds_r10b(grpName, testName)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.2, section 5");
}


IllegalAdminCmds_r12::~IllegalAdminCmds_r12()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


IllegalAdminCmds_r12::
IllegalAdminCmds_r12(const IllegalAdminCmds_r12 &other) :
    Test(other), IllegalAdminCmds_r10b(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


IllegalAdminCmds_r12 &
IllegalAdminCmds_r12::operator=(const IllegalAdminCmds_r12
    &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


list<uint8_t>
IllegalAdminCmds_r12::GetIllegalOpcodes()
{
    list<uint8_t> illegalOpCodes;

    illegalOpCodes = IllegalAdminCmds_r10b::GetIllegalOpcodes();

    uint8_t optAdminCmds = gInformative->GetIdentifyCmdCtrlr()->
        GetValue(IDCTRLRCAP_OACS);

    if ((optAdminCmds & OACS_SUP_NSMANAGEMENT_CMD) != 0) {
        illegalOpCodes.remove(NAMESPACE_MNGMT_OPCODE);
        illegalOpCodes.remove(NAMESPACE_ATTACH_OPCODE);
    }

    return illegalOpCodes;
}



}   // namespace
