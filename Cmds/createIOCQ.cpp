#include "createIOCQ.h"
#include "../Utils/buffers.h"


CreateIOCQ::CreateIOCQ() :
    AdminCmd(0, Trackable::OBJTYPE_FENCE)
{
    // This constructor will throw
}


CreateIOCQ::CreateIOCQ(int fd) :
    AdminCmd(fd, Trackable::OBJ_CREATEIOCQ)
{
    AdminCmd::Init(0x05, DATADIR_TO_DEVICE);
}


CreateIOCQ::~CreateIOCQ()
{
}


void
CreateIOCQ::Init(const SharedIOCQPtr iocq)
{
    // Setup the PRP buffer based upon contig or non-contig memory
    int prpField = MASK_PRP1_PAGE;
    if (iocq->GetIsContig() == false)
        prpField |= MASK_PRP1_LIST;
    SetPrpBuffer((send_64b_bitmask)prpField, iocq->GetQBuffer(),
        iocq->GetQSize());

    {   // Handle DWORD 10
        uint32_t dword10 = GetDword(10);

        // Handle q size
        dword10 &= ~0xffff0000;
        dword10 |= (((uint32_t)iocq->GetNumEntries()) << 16);

        // Handle Q ID
        dword10 &= ~0x0000ffff;
        dword10 |= (uint32_t)iocq->GetQId();

        SetDword(dword10, 10);
    }   // Handle DWORD 10

    {   // Handle DWORD 11
        uint32_t dword11 = GetDword(11);

        // Handle the PC bit
        if (iocq->GetIsContig())
            dword11 |= 0x00000001;
        else
            dword11 &= ~0x00000001;

        // Handle IRQ support
        if (iocq->GetIrqEnabled()) {
            dword11 |= 0x00000002;
            dword11 &= ~0xffff0000;    // clear it, then set it

            enum nvme_irq_type irq;
            if (gCtrlrConfig->GetIrqScheme(irq) == false) {
                LOG_DBG("Unable to retrieve current IRQ scheme");
                throw exception();
            }
            switch (irq) {
            case INT_MSI_MULTI:
            case INT_MSIX:
                dword11 |= (((uint32_t)iocq->GetIrqVector()) << 16);
                break;
            case INT_MSI_SINGLE:
            case INT_NONE:
                ;   // Required to be zero
                break;
            default:
                LOG_DBG("Unsupported IRQ scheme, what to do?");
                throw exception();
            }
        } else {
            dword11 &= ~0x00000002;
            dword11 &= ~0xffff0000;
        }

        SetDword(dword11, 11);
    }   // Handle DWORD 11
}


void
CreateIOCQ::Dump(LogFilename filename, string fileHdr)
{
    const uint8_t *buf = GetROPrpBuffer();
    Buffers::Dump(filename, buf, 0, ULONG_MAX, GetPrpBufferSize(), fileHdr);
}
