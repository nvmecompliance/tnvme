#include <unistd.h>
#include "identifyCmd_r10b.h"
#include "globals.h"
#include "createACQASQ_r10b.h"
#include "../Cmds/identify.h"
#include "../Utils/kernelAPI.h"

#define DEFAULT_CMD_WAIT_ms         2000


IdentifyCmd_r10b::IdentifyCmd_r10b(int fd, string grpName, string testName) :
    Test(fd, grpName, testName, SPECREV_10b)
{
    // 66 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 7");
    mTestDesc.SetShort(     "Issue the Identify cmd");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Issue the identify cmd to the ASQ. Request both controller and all "
        "namespace pages. Report this data to the log directory filename: "
        "IdentifyCmd_r10b");
}


IdentifyCmd_r10b::~IdentifyCmd_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


IdentifyCmd_r10b::
IdentifyCmd_r10b(const IdentifyCmd_r10b &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


IdentifyCmd_r10b &
IdentifyCmd_r10b::operator=(const IdentifyCmd_r10b &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


bool
IdentifyCmd_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) The ASQ & ACQ's have been created by the RsrcMngr for group lifetime
     * 2) All interrupts are disabled.
     *  \endverbatim
     */

    KernelAPI::DumpKernelMetrics(mFd,
        FileSystem::PrepLogFile(mGrpName, mTestName, "kmetrics", "before"));

    // Lookup objs which were created in a prior test within group
    SharedASQPtr asq = CAST_TO_ASQ(gRsrcMngr->GetObj(ASQ_GROUP_ID))
    SharedACQPtr acq = CAST_TO_ACQ(gRsrcMngr->GetObj(ACQ_GROUP_ID))

    SendIdentifyCtrlrStruct(asq, acq);
    SendIdentifyNamespaceStruct(asq, acq);

    KernelAPI::DumpKernelMetrics(mFd,
        FileSystem::PrepLogFile(mGrpName, mTestName, "kmetrics", "after"));
    return true;
}


void
IdentifyCmd_r10b::SendIdentifyCtrlrStruct(SharedASQPtr asq, SharedACQPtr acq)
{
    uint16_t numCE;


    LOG_NRM("Create 1st identify cmd and assoc some buffer memory");
    SharedIdentifyPtr idCmdCap = CAST_TO_IDENTIFY(
        gRsrcMngr->AllocObj(Trackable::OBJ_IDENTIFY,
        IDENTIFY_CTRLR_STRUCT_GROUP_ID));
    LOG_NRM("Force identify to request ctrlr capabilities struct");
    idCmdCap->SetCNS(true);
    SharedMemBufferPtr idMemCap = SharedMemBufferPtr(new MemBuffer());
    idMemCap->InitAlignment(Identify::IDEAL_DATA_SIZE, sizeof(uint64_t),
        true, 0);
    send_64b_bitmask idPrpCap =
        (send_64b_bitmask)(MASK_PRP1_PAGE | MASK_PRP2_PAGE);
    idCmdCap->SetPrpBuffer(idPrpCap, idMemCap);


    LOG_NRM("Send the 1st identify cmd to hdw");
    asq->Send(idCmdCap);
    asq->LogSE(0);
    asq->Dump(FileSystem::PrepLogFile(mGrpName, mTestName, "asq", "idMemCap"),
        "Just B4 ringing SQ0 doorbell, dump entire SQ contents");
    asq->Ring();


    LOG_NRM("Wait for the CE to arrive in ACQ");
    if (acq->ReapInquiryWaitSpecify(DEFAULT_CMD_WAIT_ms, 1, numCE) == false) {
        LOG_ERR("Unable to see completion of identify cmd");
        acq->Dump(
            FileSystem::PrepLogFile(mGrpName, mTestName, "acq", "idMemCap"),
            "Unable to see any CE's in CQ0, dump entire CQ contents");
        throw exception();
    }
    acq->LogCE(0);
    acq->Dump(FileSystem::PrepLogFile(mGrpName, mTestName, "acq", "idMemCap"),
        "Just B4 reaping CQ0, dump entire CQ contents");

    {
        uint16_t ceRemain;
        uint16_t numReaped;

        LOG_NRM("Reaping CE from ACQ, requires memory to hold reaped CE");
        SharedMemBufferPtr ceMemCap = SharedMemBufferPtr(new MemBuffer());
        if ((numReaped = acq->Reap(ceRemain, ceMemCap, numCE, true)) != 1) {
            LOG_ERR("Verified there was 1 CE, but reaping produced %d",
                numReaped);
            throw exception();
        }
        LOG_NRM("The reaped identify CE is...");
        ceMemCap->Log();
        idCmdCap->Dump(FileSystem::PrepLogFile(mGrpName, mTestName, "idMemCap"),
            "The complete admin cmd identify ctgrlr data structure decoded:");
    }
}


void
IdentifyCmd_r10b::SendIdentifyNamespaceStruct(SharedASQPtr asq,
    SharedACQPtr acq)
{
    uint16_t numCE;


    LOG_NRM("Create 2nd identify cmd and assoc some buffer memory");
    SharedIdentifyPtr idCmdNamSpc = CAST_TO_IDENTIFY(
        gRsrcMngr->AllocObj(Trackable::OBJ_IDENTIFY,
        IDENTIFY_NAMESPACE_STRUCT_GROUP_ID));
    LOG_NRM("Force identify to request namespace struct");
    idCmdNamSpc->SetCNS(false);
    SharedMemBufferPtr idMemNamSpc = SharedMemBufferPtr(new MemBuffer());
    idMemNamSpc->InitAlignment(Identify::IDEAL_DATA_SIZE, sizeof(uint64_t),
        true, 0);
    send_64b_bitmask idPrpNamSpc =
        (send_64b_bitmask)(MASK_PRP1_PAGE | MASK_PRP2_PAGE);
    idCmdNamSpc->SetPrpBuffer(idPrpNamSpc, idMemNamSpc);


    LOG_NRM("Send the 1st identify cmd to hdw");
    asq->Send(idCmdNamSpc);
    asq->LogSE(1);
    asq->Dump(
        FileSystem::PrepLogFile(mGrpName, mTestName, "asq", "idCmdNamSpc"),
        "Just B4 ringing SQ0 doorbell, dump entire SQ contents");
    asq->Ring();


    LOG_NRM("Wait for the CE to arrive in ACQ");
    if (acq->ReapInquiryWaitSpecify(DEFAULT_CMD_WAIT_ms, 1, numCE) == false) {
        LOG_ERR("Unable to see completion of identify cmd");
        acq->Dump(
            FileSystem::PrepLogFile(mGrpName, mTestName, "acq", "idCmdNamSpc"),
            "Unable to see any CE's in CQ0, dump entire CQ contents");
        throw exception();
    }
    acq->LogCE(1);
    acq->Dump(
        FileSystem::PrepLogFile(mGrpName, mTestName, "acq", "idCmdNamSpc"),
        "Just B4 reaping CQ0, dump entire CQ contents");


    {
        uint16_t ceRemain;
        uint16_t numReaped;

        LOG_NRM("Reaping CE from ACQ, requires memory to hold reaped CE");
        SharedMemBufferPtr ceMemNamSpc = SharedMemBufferPtr(new MemBuffer());
        if ((numReaped = acq->Reap(ceRemain, ceMemNamSpc, numCE, true)) != 1) {
            LOG_ERR("Verified there was 1 CE, but reaping produced %d",
                numReaped);
            throw exception();
        }
        LOG_NRM("The reaped identify CE is...");
        ceMemNamSpc->Log();
        idCmdNamSpc->Dump(
            FileSystem::PrepLogFile(mGrpName, mTestName, "idMemNamSpc"),
            "The complete admin cmd identify namespace structure decoded:");
    }
}
