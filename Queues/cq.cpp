#include <sys/mman.h>
#include "cq.h"
#include "globals.h"


CQ::CQ() :
    Queue(0, Trackable::OBJTYPE_FENCE, Trackable::LIFETIME_FENCE, false),
    MMAP_QTYPE_BITMASK(0x00000)
{
    // This constructor will throw
}


CQ::CQ(int fd, Trackable::ObjType objBeingCreated, Trackable::Lifetime life,
    bool ownByRsrcMngr) :
        Queue(fd, objBeingCreated, life, ownByRsrcMngr),
        MMAP_QTYPE_BITMASK(0x00000)
{
    mIrqEnabled = false;
    mIrqVec = 0;
}


CQ::~CQ()
{
    try {
        // Cleanup duties for this Q's buffer
        if (GetIsContig()) {
            // Contiguous memory is alloc'd and owned by the kernel
            munmap(mContigBuf, GetQSize());
        } else {
            // Only assume ownership if and only if the RsrcMngr doesn't own it
            if (mDiscontigBuf->GetOwnByRsrcMngr() == false)
                delete mDiscontigBuf;
        }
    } catch (...) {
        ;   // Destructors should never throw. If the object is deleted B4
            // it is Init'd() properly, it could throw, so catch and ignore
    }
}


void
CQ::Init(uint16_t qId, uint16_t entrySize, uint16_t numEntries,
    bool irqEnabled, uint16_t irqVec)
{
    Queue::Init(qId, entrySize, numEntries);
    mIrqEnabled = irqEnabled;
    mIrqVec = irqVec;

    // dnvme guarantees page aligned memory allocation and zero's it out.
    if (GetIsAdmin()) {
        if (gCtrlrConfig->GetStateEnabled()) {
            // At best this will cause tnvme to seg fault or a kernel crash
            // The NVME spec states unpredictable outcomes will occur.
            LOG_DBG("Creating an ASQ while ctrlr is enabled is a shall not");
            throw exception();
        }

        // We are creating a contiguous ACQ. ACQ's have a constant well known
        // element size and no setup is required for this type of Q.
        int ret;
        struct nvme_create_admn_q q;
        q.elements = GetNumEntries();
        q.type = ADMIN_CQ;

        LOG_NRM("Init contig ACQ: (id, entrySize, numEntries) = (%d, %d, %d)",
            GetQId(), GetEntrySize(), GetNumEntries());

        if ((ret = ioctl(mFd, NVME_IOCTL_CREATE_ADMN_Q, &q)) < 0) {
            LOG_DBG("Q Creation failed by dnvme with error: 0x%02X", ret);
            throw exception();
        }
    } else {
        // We are creating a contiguous IOCQ. IOCQ's have variable entry
        // sizes which must be setup beforehand.
        uint8_t value;
        if (gCtrlrConfig->GetIOCQES(value) == false) {
            LOG_ERR("Unable to determine Q entry size");
            throw exception();
        } else if ((2^value) != GetEntrySize()) {
            LOG_DBG("Q entry sizes do not match %d != %d", 2^value,
                GetEntrySize());
            throw exception();
        }

        struct nvme_prep_cq q;
        q.cq_id = GetQId();
        q.elements = GetNumEntries();
        q.contig = true;
        CreateIOCQ(q);
    }

    // Contiguous Q's are created in dnvme and must be mapped back to user space
    off_t encodeOffset = GetQId();
    encodeOffset |= MMAP_QTYPE_BITMASK;
    encodeOffset *= sysconf(_SC_PAGESIZE);
    mContigBuf = (uint8_t *)mmap(0, GetQSize(), PROT_READ, MAP_SHARED, mFd,
        encodeOffset);
    if (mContigBuf == NULL) {
        LOG_DBG("Unable to mmap contig memory to user space");
        throw exception();
    }
}


void
CQ::Init(uint16_t qId, uint16_t entrySize, uint16_t numEntries,
    MemBuffer &memBuffer, bool irqEnabled, uint16_t irqVec)
{
    Queue::Init(qId, entrySize, numEntries);
    mIrqEnabled = irqEnabled;
    mIrqVec = irqVec;

    if (GetIsAdmin()) {
        // There are no appropriate methods for an NVME device to report ASC/ACQ
        // creation errors, thus don't allow these problems to be injected, at
        // best they will only succeed to seg fault the app or crash the kernel.
        // IOQ's do have ways to report these types of errors, thus allow it.
        LOG_DBG("Illegal memory alignment will corrupt");
        throw exception();
    } else if (memBuffer.GetBufSize() < GetQSize()) {
        LOG_DBG("Q buffer memory ambiguous to passed params");
        throw exception();
    } else if (memBuffer.GetAlignment() != sysconf(_SC_PAGESIZE)) {
        // Nonconformance to page alignment will seg fault the app or crash
        // the kernel. This state is not testable since no errors can be
        // reported by hdw, thus disallow this attempt.
        LOG_DBG("Q content memory shall be page aligned");
        throw exception();
    } else if (memBuffer.GetOwnByRsrcMngr() != this->GetOwnByRsrcMngr()) {
        // If one obj is created by RsrcMngr then all its member must also
        LOG_DBG("MemBuffer wasn't created via same means as Q object");
        throw exception();
    } else if (memBuffer.GetObjLife() != this->GetObjLife()) {
        // Can't have the memory of the Q live shorter than the Q itself
        LOG_DBG("MemBuffer doesn't have same life span as Q object");
        throw exception();
    }

    // Zero out the content memory so the P-bit correlates to a newly alloc'd Q.
    // Also assuming life time ownership of this object if it wasn't created
    // by the RsrcMngr.
    mDiscontigBuf = &memBuffer;
    memBuffer.Reset();

    // We are creating a contiguous IOCQ. IOCQ's have variable entry
    // sizes which must be setup beforehand.
    uint8_t value;
    if (gCtrlrConfig->GetIOCQES(value) == false) {
        LOG_ERR("Unable to determine Q entry size");
        throw exception();
    } else if ((2^value) != GetEntrySize()) {
        LOG_DBG("Q entry sizes do not match %d != %d", 2^value, GetEntrySize());
        throw exception();
    }

    // We are creating a discontiguous IOCQ
    struct nvme_prep_cq q;
    q.cq_id = GetQId();
    q.elements = GetNumEntries();
    q.contig = false;
    CreateIOCQ(q);
}


void
CQ::CreateIOCQ(struct nvme_prep_cq &q)
{
    int ret;

    LOG_NRM("Init %s CQ: (id, entrySize, numEntries) = (%d, %d, %d)",
        q.contig ? "contig" : "discontig", GetQId(), GetEntrySize(),
        GetNumEntries());

    if ((ret = ioctl(mFd, NVME_IOCTL_PREPARE_CQ_CREATION, &q)) < 0) {
        LOG_DBG("Q Creation failed by dnvme with error: 0x%02X", ret);
        throw exception();
    }
}


struct nvme_gen_cq
CQ::GetQMetrics()
{
    int ret;
    struct nvme_gen_cq qMetrics;
    struct nvme_get_q_metrics getQMetrics;

    getQMetrics.q_id = GetQId();
    getQMetrics.type = METRICS_CQ;
    getQMetrics.nBytes = GetQSize();
    getQMetrics.buffer = (uint8_t *)&qMetrics;

    if ((ret = ioctl(mFd, NVME_IOCTL_GET_Q_METRICS, &getQMetrics)) < 0) {
        LOG_DBG("Get Q metrics failed by dnvme with error: 0x%02X", ret);
        throw exception();
    }
    return qMetrics;
}


union CE
CQ::GetCE(uint16_t indexPtr)
{
    union CE *dataPtr;

    if (GetIsContig())
        dataPtr = (union CE *)mContigBuf;
    else
        dataPtr = (union CE *)mDiscontigBuf;

    for (int i = 0; i < GetNumEntries(); i++, dataPtr++) {
        if (i == indexPtr)
            return *dataPtr;
    }

    LOG_DBG("Unable to locate index within Q");
    throw exception();
}
