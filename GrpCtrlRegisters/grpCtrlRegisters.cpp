#include "grpCtrlRegisters.h"
#include "ctrlCapabilities_r10a.h"


GrpCtrlRegisters::GrpCtrlRegisters(size_t grpNum, SpecRevType specRev) :
    Group(grpNum, specRev, "Controller registers syntactic")
{
    // IMPORTANT: Once a test case is assigned a position in the vector, i.e.
    //            a test index/number, then it should stay in that position so
    //            that the test references won't change per release. Future
    //            test can be appended 1.0, 1.1, 1.2, 2.0, 3.0, 4.0, 4.1, etc.
    //            Tests 1.0, 2.0, 3.0  Major num; related at the group level
    //            Tests x.0, x.1, x.2  Minor num; related at the test level
    switch (mSpecRev) {
    case SPECREV_10:    // 1.0 is identical to 1.0a
    case SPECREV_10a:
        APPEND_TEST_AT_GROUP_LEVEL(CtrlCapabilities_r10a)
        break;

    default:
    case SPECREVTYPE_FENCE:
        break;
    }
}


GrpCtrlRegisters::~GrpCtrlRegisters()
{
    // mTests deallocated in parent
}
