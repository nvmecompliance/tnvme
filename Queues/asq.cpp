#include "asq.h"
#include "globals.h"

SharedASQPtr ASQ::NullASQPtr;


ASQ::ASQ() : SQ(0, Trackable::OBJTYPE_FENCE)
{
    // This constructor will throw
}


ASQ::ASQ(int fd) : SQ(fd, Trackable::OBJ_ASQ)
{
}


ASQ::~ASQ()
{
}


void
ASQ::Init(uint16_t numEntries)
{
    SQ::Init(0, 16, numEntries, 0);
}
