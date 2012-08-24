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
#include "illegalNVMCmds_r10b.h"
#include "globals.h"
#include "grpDefs.h"
#include "../Queues/iocq.h"
#include "../Queues/iosq.h"
#include "../Utils/io.h"
#include "../Cmds/nvmCmd.h"


namespace GrpGeneralCmds {


#define VENDOR_SPEC_OPC             0x80


const uint8_t IllegalNVMCmds_r10b::WRITE_UNCORR_OPCODE  = 0x04;
const uint8_t IllegalNVMCmds_r10b::COMPARE_OPCODE       = 0x05;
const uint8_t IllegalNVMCmds_r10b::DSM_OPCODE           = 0x09;


IllegalNVMCmds_r10b::IllegalNVMCmds_r10b(
    string grpName, string testName) :
    Test(grpName, testName, SPECREV_10b)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 6");
    mTestDesc.SetShort(     "Issue illegal nvm cmd set opcodes.");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Don't test vendor specific opcodes, then determine all supported NVM "
        "cmds and issue all other illegal opcodes. Verify status code in the "
        "CE of IOCQ is 1h.");
}


IllegalNVMCmds_r10b::~IllegalNVMCmds_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


IllegalNVMCmds_r10b::
IllegalNVMCmds_r10b(const IllegalNVMCmds_r10b &other) :
    Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


IllegalNVMCmds_r10b &
IllegalNVMCmds_r10b::operator=(const IllegalNVMCmds_r10b
    &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


Test::RunType
IllegalNVMCmds_r10b::RunnableCoreTest(bool preserve)
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
IllegalNVMCmds_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) Test CreateResources_r10b has run prior.
     * \endverbatim
     */
    string work;
    list<uint8_t> illegalOpCodes = GetIllegalOpcodes();

    LOG_NRM("Lookup objs which were created in a prior test within group");
    SharedIOSQPtr iosq = CAST_TO_IOSQ(gRsrcMngr->GetObj(IOSQ_GROUP_ID));
    SharedIOCQPtr iocq = CAST_TO_IOCQ(gRsrcMngr->GetObj(IOCQ_GROUP_ID));

    LOG_NRM("Form a Generic NVM cmd to send to an IOSQ.");
    SharedNVMCmdPtr genericNVMCmd = SharedNVMCmdPtr(new NVMCmd());

    for (list<uint8_t>::iterator opCode = illegalOpCodes.begin();
        opCode != illegalOpCodes.end(); opCode++) {

        genericNVMCmd->Init(*opCode);
        genericNVMCmd->SetNSID(1);

        work = str(boost::format("IllegalOpcode.%d") % (uint)*opCode);
        IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), iosq,
            iocq, genericNVMCmd, work, true, CESTAT_INVAL_OPCODE);
    }
}


list<uint8_t>
IllegalNVMCmds_r10b::GetIllegalOpcodes()
{
    list<uint8_t> illegalOpCodes;

    for (uint8_t opCode = 0x3; opCode < VENDOR_SPEC_OPC; opCode++)
        illegalOpCodes.push_back(opCode);

    uint8_t optNVMCmds = (gInformative->GetIdentifyCmdCtrlr()->
        GetValue(IDCTRLRCAP_ONCS) & 0x7);

    if ((optNVMCmds & ONCS_SUP_COMP_CMD) != 0)
        illegalOpCodes.remove(COMPARE_OPCODE);

    if ((optNVMCmds & ONCS_SUP_WR_UNC_CMD) != 0)
        illegalOpCodes.remove(WRITE_UNCORR_OPCODE);

    if ((optNVMCmds & ONCS_SUP_DSM_CMD) != 0)
        illegalOpCodes.remove(DSM_OPCODE);

    return illegalOpCodes;
}



}   // namespace
