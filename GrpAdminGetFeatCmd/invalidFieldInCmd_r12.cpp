/*
 * Copyright (c) 2015, UNH-IOL - All Rights Reserved.
 */

#include <boost/format.hpp>
#include "invalidFieldInCmd_r12.h"
#include "globals.h"
#include "grpDefs.h"
#include "../Utils/kernelAPI.h"
#include "../Utils/io.h"
#include "../Cmds/getFeatures.h"

namespace GrpAdminGetFeatCmd {


InvalidFieldInCmd_r12::InvalidFieldInCmd_r12(
    string grpName, string testName) :
    Test(grpName, testName, SPECREV_11),
    InvalidFieldInCmd_r10b(grpName, testName)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.1, section 5");
    mTestDesc.SetShort(     "Issue reserved FID's; SC = Invalid field in cmd");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Create a baseline GetFeatures cmd, with no PRP payload. Issue cmd "
        "for all reserved DW10.FID = {0x00, 0x0C to 0x7F, 0x81 to 0xBF}, "
        "expect failure.");
}


InvalidFieldInCmd_r12::~InvalidFieldInCmd_r12()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


InvalidFieldInCmd_r12::
InvalidFieldInCmd_r12(const InvalidFieldInCmd_r12 &other) : Test(other),
    InvalidFieldInCmd_r10b(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


InvalidFieldInCmd_r12 &
InvalidFieldInCmd_r12::operator=(const InvalidFieldInCmd_r12 &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


void
InvalidFieldInCmd_r12::getInvalidFIDs(vector<uint16_t> invalidFIDs) const
{
    uint8_t invalFID;

    // Reserved FIDs
    invalidFIDs.push_back(0x00);
    for (uint8_t invalFID = 0x0E; invalFID <= 0x7F; invalFID++)
        invalidFIDs.push_back(invalFID);

    // Command Set Specific reserved FIDs
    if ((gInformative->GetIdentifyCmdCtrlr()->GetValue(IDCTRLRCAP_ONCS))
            & ONCS_SUP_RSRV)
       invalFID = 0x84;
    else
       invalFID = 0x81;

    for (; invalFID <= 0xBF; invalFID++)
       invalidFIDs.push_back(invalFID);
}

}   // namespace
