#include "deleteIOSQ.h"
#include "../Utils/buffers.h"


DeleteIOSQ::DeleteIOSQ() :
    AdminCmd(0, Trackable::OBJTYPE_FENCE)
{
    // This constructor will throw
}


DeleteIOSQ::DeleteIOSQ(int fd) :
    AdminCmd(fd, Trackable::OBJ_DELETEIOSQ)
{
    AdminCmd::Init(0x00, DATADIR_NONE);
}


DeleteIOSQ::~DeleteIOSQ()
{
}


void
DeleteIOSQ::Init(const SharedIOSQPtr iosq)
{
    {   // Handle DWORD 10
        uint32_t dword10 = GetDword(10);

        // Handle Q ID
        dword10 &= ~0x0000ffff;
        dword10 |= (uint32_t)iosq->GetQId();

        SetDword(dword10, 10);
    }   // Handle DWORD 10
}
