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
#include "illegalAdminCmds_r10b.h"
#include "globals.h"
#include "grpDefs.h"
#include "../Queues/iocq.h"
#include "../Queues/iosq.h"
#include "../Utils/io.h"
#include "../Cmds/adminCmd.h"


namespace GrpGeneralCmds {


#define VENDOR_SPEC_OPC             0xC0

const uint8_t IllegalAdminCmds_r10b::FW_ACTIVATE_OPCODE         = 0x10;
const uint8_t IllegalAdminCmds_r10b::FW_DOWNLOAD_OPCODE         = 0x11;
const uint8_t IllegalAdminCmds_r10b::FORMAT_NVM_OPCODE          = 0x80;
const uint8_t IllegalAdminCmds_r10b::SECURITY_SEND_OPCODE       = 0x81;
const uint8_t IllegalAdminCmds_r10b::SECURITY_RECEIVE_OPCODE    = 0x82;

IllegalAdminCmds_r10b::IllegalAdminCmds_r10b(
    string grpName, string testName) :
    Test(grpName, testName, SPECREV_10b)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 5");
    mTestDesc.SetShort(     "Issue illegal admin cmd set opcodes.");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Don't test vendor specific opcodes, then determine all supported "
        "admin cmds based upon the admin cmd set and issue all other illegal "
        "opcodes. Verify status code in the CE of ACQ is 1h.");
}


IllegalAdminCmds_r10b::~IllegalAdminCmds_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


IllegalAdminCmds_r10b::
IllegalAdminCmds_r10b(const IllegalAdminCmds_r10b &other) :
    Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


IllegalAdminCmds_r10b &
IllegalAdminCmds_r10b::operator=(const IllegalAdminCmds_r10b
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
IllegalAdminCmds_r10b::RunnableCoreTest(bool preserve)
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
IllegalAdminCmds_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) Test CreateResources_r10b has run prior.
     * \endverbatim
     */
    string work;
    list<uint8_t> illegalOpCodes = GetIllegalOpcodes();

    LOG_NRM("Lookup objs which were created in a prior test within group");
    SharedASQPtr asq = CAST_TO_ASQ(gRsrcMngr->GetObj(ASQ_GROUP_ID))
    SharedACQPtr acq = CAST_TO_ACQ(gRsrcMngr->GetObj(ACQ_GROUP_ID))

    LOG_NRM("Form a Generic Admin cmd to send to an ASQ.");
    SharedAdminCmdPtr genericAdminCmd = SharedAdminCmdPtr(new AdminCmd());

    for (list<uint8_t>::iterator opCode = illegalOpCodes.begin();
        opCode != illegalOpCodes.end(); opCode++) {
        LOG_NRM("Sending admin cmd with illegal opcode = 0x%X", (uint)*opCode);
        genericAdminCmd->Init(*opCode);

        work = str(boost::format("IllegalOpcode.%d") % (uint)*opCode);
        IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), asq,
            acq, genericAdminCmd, work, true, CESTAT_INVAL_OPCODE);
    }
}


list<uint8_t>
IllegalAdminCmds_r10b::GetIllegalOpcodes()
{
    list<uint8_t> illegalOpCodes;

    illegalOpCodes.push_back(0x03);
    illegalOpCodes.push_back(0x07);
    illegalOpCodes.push_back(0x0B);

    for (uint8_t opCode = 0x0D; opCode < VENDOR_SPEC_OPC; opCode++)
        illegalOpCodes.push_back(opCode);

    uint8_t optAdminCmds = (gInformative->GetIdentifyCmdCtrlr()->
        GetValue(IDCTRLRCAP_OACS) & 0x7);

    if ((optAdminCmds & OACS_SUP_SECURITY_CMD) != 0) {
        illegalOpCodes.remove(SECURITY_SEND_OPCODE);
        illegalOpCodes.remove(SECURITY_RECEIVE_OPCODE);
    }

    if ((optAdminCmds & OACS_SUP_FORMAT_NVM_CMD) != 0)
        illegalOpCodes.remove(FORMAT_NVM_OPCODE);

    if ((optAdminCmds & OACS_SUP_FIRMWARE_CMD) != 0) {
        illegalOpCodes.remove(FW_ACTIVATE_OPCODE);
        illegalOpCodes.remove(FW_DOWNLOAD_OPCODE);
    }

    return illegalOpCodes;
}



}   // namespace
