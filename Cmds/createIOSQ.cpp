#include "createIOSQ.h"
#include "../Utils/buffers.h"


CreateIOSQ::CreateIOSQ() :
    AdminCmd(0, Trackable::OBJTYPE_FENCE)
{
    // This constructor will throw
}


CreateIOSQ::CreateIOSQ(int fd) :
    AdminCmd(fd, Trackable::OBJ_IOSQ)
{
    AdminCmd::Init(0x01, DATADIR_TO_DEVICE);
}


CreateIOSQ::~CreateIOSQ()
{
}


void
CreateIOSQ::Init(const SharedIOSQPtr iosq)
{
    SetPrpBuffer((send_64b_bitmask)MASK_PRP1_PAGE, iosq->GetQBuffer(),
        iosq->GetQSize());

    {   // Handle DWORD 10
        uint32_t dword10 = GetDword(10);

        // Handle q size
        dword10 &= ~0xffff0000;
        dword10 |= (((uint32_t)iosq->GetNumEntries()) << 16);

        // Handle Q ID
        dword10 &= ~0x0000ffff;
        dword10 |= (uint32_t)iosq->GetQId();

        SetDword(dword10, 10);
    }   // Handle DWORD 10

    {   // Handle DWORD 11
        uint32_t dword11 = GetDword(11);

        // Handle the PC bit
        if (iosq->GetIsContig())
            dword11 |= 0x00000001;
        else
            dword11 &= ~0x00000001;

        // Handle Q priority
        dword11 &= ~0x00000006;
        dword11 |= (((uint32_t)iosq->GetPriority()) << 1);

        // Handle CQ ID
        dword11 &= ~0xffff0000;
        dword11 |= iosq->GetCqId();

        SetDword(dword11, 11);
    }   // Handle DWORD 11
}


void
CreateIOSQ::Dump(LogFilename filename, string fileHdr)
{
    const uint8_t *buf = GetROPrpBuffer();

    // Do a raw dump of the data
    Buffers::Dump(filename, buf, 0, ULONG_MAX, GetPrpBufferSize(), fileHdr);
}
