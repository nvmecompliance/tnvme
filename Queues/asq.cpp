#include "asq.h"
#include "globals.h"

SharedASQPtr ASQ::NullASQPtr;
const uint16_t ASQ::IDEAL_ELEMENT_SIZE = 64;


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
    SQ::Init(0, IDEAL_ELEMENT_SIZE, numEntries, 0);
}
