#include "disableCompletely_r10b.h"
#include "../globals.h"


DisableCompletely_r10b::DisableCompletely_r10b(int fd) : Test(fd, SPECREV_10b)
{
    // 66 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 7");
    mTestDesc.SetShort(     "Validate basic hardware initialization duties");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Setup for forthcoming tests. It will completely disabled the "
        "controller. All driver and NVME device memory is freed to the system, "
        "nothing will be operational, not even IRQ's");
}


DisableCompletely_r10b::~DisableCompletely_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


bool
DisableCompletely_r10b::RunCoreTest()
{
    // Create an object we expect to be freed after this test ends
    SharedTrackablePtr someMemory1;
    someMemory1 = gRsrcMngr->AllocObjTestLife(Trackable::OBJ_MEMBUFFER);
    if (someMemory1 == RsrcMngr::NullTrackablePtr) {
        LOG_DBG("Allocation of object failed");
        throw exception();
    }

    // Create an object we expect to be freed after this group ends
    SharedTrackablePtr someMemory2;
    someMemory2 = gRsrcMngr->AllocObjGrpLife(Trackable::OBJ_MEMBUFFER, "MyObj");
    if (someMemory2 == RsrcMngr::NullTrackablePtr) {
        LOG_DBG("Allocation of object failed");
        throw exception();
    }

    return true;
}
