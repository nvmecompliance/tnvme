#include "ctrlCapabilities_r10a.h"


CtrlCapabilities_r10a::CtrlCapabilities_r10a()
{
    // 72 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0a, section 3");
    mTestDesc.SetShort(     "Validate controller capabilities register syntactically");
    mTestDesc.SetLong(
        "Validates the following; the RO fields cannot be written; all ASCII "
        "fields only contain chars 0x20 to 0x7e; writing reserved bits are "
        "ignored.");
}


CtrlCapabilities_r10a::~CtrlCapabilities_r10a()
{
}


bool
CtrlCapabilities_r10a::RunCoreTest()
{
    return (false); // not implemented
}


