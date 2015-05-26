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
#include "illegalNVMCmds_r11.h"
#include "globals.h"
#include "grpDefs.h"
#include "../Queues/iocq.h"
#include "../Queues/iosq.h"
#include "../Utils/io.h"
#include "../Cmds/nvmCmd.h"


namespace GrpGeneralCmds {

const uint8_t IllegalNVMCmds_r11::RSV_REG_OPCODE	    = 0x0D;
const uint8_t IllegalNVMCmds_r11::RSV_REPORT_OPCODE	    = 0x0E;
const uint8_t IllegalNVMCmds_r11::RSV_ACQUIRE_OPCODE	= 0x11;
const uint8_t IllegalNVMCmds_r11::RSV_RELEASE_OPCODE	= 0x15;


IllegalNVMCmds_r11::IllegalNVMCmds_r11(string grpName, string testName) :
    Test(grpName, testName, SPECREV_11),
    IllegalNVMCmds_r10b(grpName, testName)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.1, section 6");
}


IllegalNVMCmds_r11::~IllegalNVMCmds_r11()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


IllegalNVMCmds_r11::
IllegalNVMCmds_r11(const IllegalNVMCmds_r11 &other) :
    Test(other), IllegalNVMCmds_r10b(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


IllegalNVMCmds_r11 &
IllegalNVMCmds_r11::operator=(const IllegalNVMCmds_r11
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
IllegalNVMCmds_r11::GetIllegalOpcodes()
{
    list<uint8_t> illegalOpCodes;

    illegalOpCodes = IllegalNVMCmds_r10b::GetIllegalOpcodes();

    uint8_t optNVMCmds = gInformative->GetIdentifyCmdCtrlr()->
        GetValue(IDCTRLRCAP_ONCS);

    if ((optNVMCmds & ONCS_SUP_RSRV) != 0) {
        illegalOpCodes.remove(RSV_REG_OPCODE);
        illegalOpCodes.remove(RSV_REPORT_OPCODE);
        illegalOpCodes.remove(RSV_ACQUIRE_OPCODE);
        illegalOpCodes.remove(RSV_RELEASE_OPCODE);
    }

    return illegalOpCodes;
}



}   // namespace
