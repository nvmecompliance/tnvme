#include <unistd.h>
#include "identifyCmd_r10b.h"
#include "globals.h"
#include "../Queues/asq.h"
#include "../Queues/acq.h"
#include "../Cmds/identify.h"
#include "../Utils/kernelAPI.h"

#define DEFAULT_CMD_WAIT_ms       2000


IdentifyCmd_r10b::IdentifyCmd_r10b(int fd) : Test(fd, SPECREV_10b)
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
     *  \endverbatim
     */

    // Create the Identify cmd and assoc some buffer memory to it
    SharedIdentifyPtr idCmd = SharedIdentifyPtr(new Identify(mFd));
    idCmd->SetCNS(true);
    SharedMemBufferPtr idMem = SharedMemBufferPtr(new MemBuffer());
    idMem->Init(Identify::IDEAL_DATA_SIZE, sysconf(_SC_PAGESIZE));
    send_64b_bitmask idPrp =
        (send_64b_bitmask)(MASK_PRP1_PAGE | MASK_PRP2_PAGE);
    idCmd->SetBuffer(idPrp, idMem);

    // Send the identify cmd to hdw
    SharedASQPtr asq = CAST_TO_ASQ(gRsrcMngr->GetObj("ASQ"))
    asq->Send(idCmd);
    asq->Ring();

    // Wait for the CE back from hdw
    uint16_t numCE;
    SharedACQPtr acq = CAST_TO_ACQ(gRsrcMngr->GetObj("ACQ"))
    if (acq->ReapInquiryWaitSpecify(DEFAULT_CMD_WAIT_ms, 1, numCE) == false) {
        LOG_ERR("Unable to see completion of identify cmd");
        throw exception();
    }
    acq->LogCE(0);

    // Extract the CE from the CQ
    uint16_t ceRemain;
    uint16_t numReaped;
    SharedMemBufferPtr ceMem = SharedMemBufferPtr(new MemBuffer());
    if ((numReaped = acq->Reap(ceRemain, ceMem, numCE, true)) != 1) {
        LOG_ERR("Verified there was 1 CE, but reaping produced %d", numReaped);
        throw exception();
    }



KernelAPI::DumpKernelMetrics(mFd, string(FORM_LOGNAME(IdentifyCmd_r10b)));
    return true;
}
