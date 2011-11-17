#include "deleteIOCQ.h"
#include "../Utils/buffers.h"


DeleteIOCQ::DeleteIOCQ() :
    AdminCmd(0, Trackable::OBJTYPE_FENCE)
{
    // This constructor will throw
}


DeleteIOCQ::DeleteIOCQ(int fd) :
    AdminCmd(fd, Trackable::OBJ_DELETEIOCQ)
{
    AdminCmd::Init(0x04, DATADIR_NONE);
}


DeleteIOCQ::~DeleteIOCQ()
{
}


void
DeleteIOCQ::Init(const SharedIOCQPtr iocq)
{
    {   // Handle DWORD 10
        uint32_t dword10 = GetDword(10);

        // Handle Q ID
        dword10 &= ~0x0000ffff;
        dword10 |= (uint32_t)iocq->GetQId();

        SetDword(dword10, 10);
    }   // Handle DWORD 10
}
