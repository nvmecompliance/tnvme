#include "grpInformative.h"
#include "dumpPciAddrSpace_r10b.h"
#include "dumpCtrlrAddrSpace_r10b.h"


GrpInformative::GrpInformative(size_t grpNum, SpecRev specRev, int fd) :
    Group(grpNum, specRev, "Informative")
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
        APPEND_TEST_AT_GROUP_LEVEL(DumpPciAddrSpace_r10b, fd, GrpInformative)
        APPEND_TEST_AT_GROUP_LEVEL(DumpCtrlrAddrSpace_r10b, fd, GrpInformative)
        break;

    default:
    case SPECREVTYPE_FENCE:
        LOG_DBG("Object created with an unknown SpecRev=%d", specRev);
        break;
    }
}


GrpInformative::~GrpInformative()
{
    // mTests deallocated in parent
}
