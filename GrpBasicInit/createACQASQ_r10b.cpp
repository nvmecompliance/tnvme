#include "createACQASQ_r10b.h"
#include "globals.h"
#include "../Queues/acq.h"
#include "../Queues/asq.h"


CreateACQASQ_r10b::CreateACQASQ_r10b(int fd) : Test(fd, SPECREV_10b)
{
    // 66 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 7");
    mTestDesc.SetShort(     "Validate basic hardware initialization duties");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Create ACQ & ASQ kernel objects with group lifespan.");
}


CreateACQASQ_r10b::~CreateACQASQ_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


CreateACQASQ_r10b::
CreateACQASQ_r10b(const CreateACQASQ_r10b &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


CreateACQASQ_r10b &
CreateACQASQ_r10b::operator=(const CreateACQASQ_r10b &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


bool
CreateACQASQ_r10b::RunCoreTest()
{
    // Assumptions: (KernelAPI::SoftReset() does the following)
    // 1) This is the 1st within GrpBasicInit.
    // 2) The NVME device is disabled
    // 3) All interrupts are disabled.

    if (gCtrlrConfig->SetMPS() == false)
        throw exception();

    SharedACQPtr acq = CAST_TO_ACQ(
        gRsrcMngr->AllocObj(Trackable::OBJ_ACQ, "ACQ"));
    acq->Init(5);

    SharedASQPtr asq = CAST_TO_ASQ(
        gRsrcMngr->AllocObj(Trackable::OBJ_ASQ, "ASQ"));
    asq->Init(5);

    if (gCtrlrConfig->SetState(ST_ENABLE) == false)
        throw exception();

    return true;
}
