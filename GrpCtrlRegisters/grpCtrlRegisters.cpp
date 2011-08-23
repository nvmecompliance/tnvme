#include "grpCtrlRegisters.h"
#include "allCtrlRegs_r10b.h"


GrpCtrlRegisters::GrpCtrlRegisters(size_t grpNum, SpecRev specRev, int fd) :
    Group(grpNum, specRev, "Controller registers syntactic")
{
    // IMPORTANT: Once a test case is assigned a position in the vector, i.e.
    //            a test index/number of the form major.minor, then it should
    //            reside in that position forever so test reference numbers
    //            don't change per release. Future tests can be appended at
    //            either the group level or the test level.
    //            Tests 0.0, 1.0, <next_test_num=2>.0  Major num; group level
    //            Tests x.0, x.1, x.<next_test_num=2>  Minor num; test level
    switch (mSpecRev) {
    case SPECREV_10b:
        APPEND_TEST_AT_GROUP_LEVEL(AllCtrlRegs_r10b, fd)
        break;

    default:
    case SPECREVTYPE_FENCE:
        LOG_DBG("Object created with an unknown SpecRev=%d", specRev);
        break;
    }
}


GrpCtrlRegisters::~GrpCtrlRegisters()
{
    // mTests deallocated in parent
}
