#include <sys/mman.h>
#include "sq.h"
#include "globals.h"


SQ::SQ() :
    Queue(0, Trackable::OBJTYPE_FENCE, Trackable::LIFETIME_FENCE, false),
    MMAP_QTYPE_BITMASK(0x10000)
{
}


SQ::SQ(int fd, Trackable::ObjType objBeingCreated, Trackable::Lifetime life,
    bool ownByRsrcMngr) :
        Queue(fd, objBeingCreated, life, ownByRsrcMngr),
        MMAP_QTYPE_BITMASK(0x10000)
{
    mCqId = 0;
    mPriority = 0;
}


SQ::~SQ()
{
    // Cleanup duties for this Q's buffer
    if (GetIsContig()) {
        // Contiguous memory is alloc'd and owned by the kernel
        munmap(mContigBuf, GetQSize());
    } else {
        // Only assume ownership if and only if the RsrcMngr doesn't own it
        if (mDiscontigBuf->GetOwnByRsrcMngr() == false)
            delete mDiscontigBuf;
    }
}


void
SQ::Init(uint16_t qId, uint16_t entrySize, uint16_t numEntries,
    uint16_t cqId, uint8_t priority)
{
    Queue::Init(qId, entrySize, numEntries);
    mCqId = cqId;

    switch (priority) {
    case 0x00:
    case 0x01:
    case 0x10:
    case 0x11:
        mPriority = priority;
        break;

    default:
        LOG_DBG("Illegal priority value, can't fit within 2 bits");
        throw exception();
        break;
    }


    // dnvme guarantees page aligned memory allocation and zero's it out.
    if (GetIsAdmin()) {
        // We are creating a contiguous ASQ. ASQ's have a constant well known
        // element size and no setup is required for this type of Q.
        int ret;
        struct nvme_create_admn_q q;
        q.elements = GetNumEntries();
        q.type = ADMIN_SQ;

        LOG_NRM(
            "Init contig ASQ: (id, cqid, entrySize, numEntries) = "
            "(%d, %d, %d, %d)",
            GetQId(), GetCqId(), GetEntrySize(), GetNumEntries());

        if ((ret = ioctl(mFd, NVME_IOCTL_CREATE_ADMN_Q, &q)) < 0) {
            LOG_DBG("Q Creation failed by dnvme with error: 0x%02X", ret);
            throw exception();
        }
    } else {
        // We are creating a contiguous IOSQ. IOSQ's have variable entry
        // sizes which must be setup beforehand.
        uint8_t value;
        if (gCtrlrConfig->GetIOSQES(value) == false) {
            LOG_ERR("Unable to determine Q entry size");
            throw exception();
        } else if ((2^value) != GetEntrySize()) {
            LOG_DBG("Q entry sizes do not match %d != %d", 2^value,
                GetEntrySize());
            throw exception();
        }

        struct nvme_prep_sq q;
        q.sq_id = GetQId();
        q.cq_id = GetCqId();
        q.elements = GetNumEntries();
        q.contig = true;
        CreateIOSQ(q);
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
SQ::Init(uint16_t qId, uint16_t entrySize, uint16_t numEntries,
    MemBuffer &memBuffer, uint16_t cqId, uint8_t priority)
{
    Queue::Init(qId, entrySize, numEntries);
    mCqId = cqId;

    switch (priority) {
    case 0x00:
    case 0x01:
    case 0x10:
    case 0x11:
        mPriority = priority;
        break;

    default:
        LOG_DBG("Illegal priority value, can't fit within 2 bits");
        throw exception();
        break;
    }

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

    // We are creating a contiguous IOSQ. IOSQ's have variable entry
    // sizes which must be setup beforehand.
    uint8_t value;
    if (gCtrlrConfig->GetIOSQES(value) == false) {
        LOG_ERR("Unable to determine Q entry size");
        throw exception();
    } else if ((2^value) != GetEntrySize()) {
        LOG_DBG("Q entry sizes do not match %d != %d", 2^value, GetEntrySize());
        throw exception();
    }

    // We are creating a discontiguous IOSQ
    struct nvme_prep_sq q;
    q.sq_id = GetQId();
    q.cq_id = GetCqId();
    q.elements = GetNumEntries();
    q.contig = false;
    CreateIOSQ(q);
}


void
SQ::CreateIOSQ(struct nvme_prep_sq &q)
{
    int ret;

    LOG_NRM("Init %s SQ: (id, cqid, entrySize, numEntries) = (%d, %d, %d, %d)",
        q.contig ? "contig" : "discontig", GetQId(), GetCqId(), GetEntrySize(),
        GetNumEntries());

    if ((ret = ioctl(mFd, NVME_IOCTL_PREPARE_SQ_CREATION, &q)) < 0) {
        LOG_DBG("Q Creation failed by dnvme with error: 0x%02X", ret);
        throw exception();
    }
}


struct nvme_gen_sq
SQ::GetQMetrics()
{
    int ret;
    struct nvme_gen_sq qMetrics;
    struct nvme_get_q_metrics getQMetrics;

    getQMetrics.q_id = GetQId();
    getQMetrics.type = METRICS_SQ;
    getQMetrics.nBytes = GetQSize();
    getQMetrics.buffer = (uint8_t *)&qMetrics;

    if ((ret = ioctl(mFd, NVME_IOCTL_GET_Q_METRICS, &getQMetrics)) < 0) {
        LOG_DBG("Get Q metrics failed by dnvme with error: 0x%02X", ret);
        throw exception();
    }
    return qMetrics;
}
