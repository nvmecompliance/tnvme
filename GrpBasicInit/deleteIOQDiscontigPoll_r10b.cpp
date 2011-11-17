#include "deleteIOQDiscontigPoll_r10b.h"
#include "globals.h"
#include "../Queues/iocq.h"
#include "../Queues/iosq.h"
#include "../Utils/kernelAPI.h"
#include "../Cmds/deleteIOCQ.h"
#include "../Cmds/deleteIOSQ.h"
#include "createACQASQ_r10b.h"
#include "createIOQDiscontigPoll_r10b.h"

#define DEFAULT_CMD_WAIT_ms         2000


DeleteIOQDiscontigPoll_r10b::DeleteIOQDiscontigPoll_r10b(int fd, string grpName,
    string testName) :
    Test(fd, grpName, testName, SPECREV_10b)
{
    // 66 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 7");
    mTestDesc.SetShort(     "Delete discontiguous IOCQ and IOSQ's");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Issue the admin commands Delete I/O SQ and Delete I/Q CQ"
        "to the ASQ and reap the resulting CE's from the ACQ to certify "
        "those the discontiguous IOQ's have been deleted. Dumping driver "
        "metrics before and after the deletion will prove the dnvme/hdw has "
        "removed those Q's");
}


DeleteIOQDiscontigPoll_r10b::~DeleteIOQDiscontigPoll_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


DeleteIOQDiscontigPoll_r10b::
DeleteIOQDiscontigPoll_r10b(const DeleteIOQDiscontigPoll_r10b &other) :
    Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


DeleteIOQDiscontigPoll_r10b &
DeleteIOQDiscontigPoll_r10b::operator=(const DeleteIOQDiscontigPoll_r10b &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


bool
DeleteIOQDiscontigPoll_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions: (KernelAPI::SoftReset() does the following)
     * 1) The ASQ & ACQ's have been created by the RsrcMngr for group lifetime
     * 2) All interrupts are disabled.
     * 3) CreateIOQDisontigPoll_r10b test case has setup the Q's to delete
     * 4) CC.IOCQES and CC.IOSQES are already setup with correct values.
     * \endverbatim
     */

    KernelAPI::DumpKernelMetrics(mFd,
        FileSystem::PrepLogFile(mGrpName, mTestName, "kmetrics", "before"));

    // Lookup objs which were created in a prior test within group
    SharedASQPtr asq = CAST_TO_ASQ(gRsrcMngr->GetObj(ASQ_GROUP_ID))
    SharedACQPtr acq = CAST_TO_ACQ(gRsrcMngr->GetObj(ACQ_GROUP_ID))

    DeleteIOCQDiscontigPoll(asq, acq);
    DeleteIOSQDiscontigPoll(asq, acq);

    KernelAPI::DumpKernelMetrics(mFd,
        FileSystem::PrepLogFile(mGrpName, mTestName, "kmetrics", "after"));
    return true;
}


void
DeleteIOQDiscontigPoll_r10b::DeleteIOCQDiscontigPoll(SharedASQPtr asq,
    SharedACQPtr acq)
{
    uint16_t numCE;

    LOG_NRM("Lookup IOCQ which was created in a prior test within group");
    SharedIOCQPtr iocq = CAST_TO_IOCQ(
        gRsrcMngr->GetObj(IOCQ_DISCONTIG_GROUP_ID))

    LOG_NRM("Create a Delete IOCQ cmd to perform the IOCQ deletion");
    SharedDeleteIOCQPtr deleteIOCQCmd =
        SharedDeleteIOCQPtr(new DeleteIOCQ(mFd));
    deleteIOCQCmd->Init(iocq);


    LOG_NRM("Send the Delete IOCQ cmd to hdw");
    asq->Send(deleteIOCQCmd);
    asq->Dump(FileSystem::PrepLogFile(mGrpName, mTestName, "asq",
        "deleteIOCQCmd"),
        "Just B4 ringing SQ0 doorbell, dump entire SQ contents");
    asq->Ring();


    LOG_NRM("Wait for the CE to arrive in ACQ");
    if (acq->ReapInquiryWaitSpecify(DEFAULT_CMD_WAIT_ms, 1, numCE) == false) {
        LOG_ERR("Unable to see completion of Delete IOCQ cmd");
        acq->Dump(
            FileSystem::PrepLogFile(mGrpName, mTestName, "acq","deleteIOCQCmd"),
            "Unable to see any CE's in CQ0, dump entire CQ contents");
        throw exception();
    }
    acq->Dump(FileSystem::PrepLogFile(mGrpName, mTestName, "acq",
        "deleteIOCQCmd"), "Just B4 reaping CQ0, dump entire CQ contents");


    {
        uint16_t ceRemain;
        uint16_t numReaped;

        LOG_NRM("Reaping CE from ACQ, requires memory to hold reaped CE");
        SharedMemBufferPtr ceMemIOCQ = SharedMemBufferPtr(new MemBuffer());
        if ((numReaped = acq->Reap(ceRemain, ceMemIOCQ, numCE, true)) != 1) {
            LOG_ERR("Verified there was 1 CE, but reaping produced %d",
                numReaped);
            throw exception();
        }
        LOG_NRM("The reaped identify CE is...");
        ceMemIOCQ->Log();
    }
}


void
DeleteIOQDiscontigPoll_r10b::DeleteIOSQDiscontigPoll(SharedASQPtr asq,
    SharedACQPtr acq)
{
    uint16_t numCE;

    LOG_NRM("Lookup IOSQ which was created in a prior test within group");
    SharedIOSQPtr iosq = CAST_TO_IOSQ(
        gRsrcMngr->GetObj(IOSQ_DISCONTIG_GROUP_ID))

    LOG_NRM("Create a Delete IOSQ cmd to perform the IOSQ deletion");
    SharedDeleteIOSQPtr deleteIOSQCmd =
        SharedDeleteIOSQPtr(new DeleteIOSQ(mFd));
    deleteIOSQCmd->Init(iosq);


    LOG_NRM("Send the Delete IOSQ cmd to hdw");
    asq->Send(deleteIOSQCmd);
    asq->Dump(FileSystem::PrepLogFile(mGrpName, mTestName, "asq",
        "deleteIOSQCmd"),
        "Just B4 ringing SQ0 doorbell, dump entire SQ contents");
    asq->Ring();


    LOG_NRM("Wait for the CE to arrive in ACQ");
    if (acq->ReapInquiryWaitSpecify(DEFAULT_CMD_WAIT_ms, 1, numCE) == false) {
        LOG_ERR("Unable to see completion of Delete IOSQ cmd");
        acq->Dump(
            FileSystem::PrepLogFile(mGrpName, mTestName, "acq","deleteIOSQCmd"),
            "Unable to see any CE's in CQ0, dump entire CQ contents");
        throw exception();
    }
    acq->Dump(FileSystem::PrepLogFile(mGrpName, mTestName, "acq",
        "deleteIOSQCmd"), "Just B4 reaping CQ0, dump entire CQ contents");


    {
        uint16_t ceRemain;
        uint16_t numReaped;

        LOG_NRM("Reaping CE from ACQ, requires memory to hold reaped CE");
        SharedMemBufferPtr ceMemIOSQ = SharedMemBufferPtr(new MemBuffer());
        if ((numReaped = acq->Reap(ceRemain, ceMemIOSQ, numCE, true)) != 1) {
            LOG_ERR("Verified there was 1 CE, but reaping produced %d",
                numReaped);
            throw exception();
        }
        LOG_NRM("The reaped identify CE is...");
        ceMemIOSQ->Log();
    }
}
