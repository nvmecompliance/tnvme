#ifndef _GRPBASICINIT_H_
#define _GRPBASICINIT_H_

#include "../group.h"


/**
* This class implements a logical grouping of test cases for basic
* initialization of NVME hardware. This mainly consists of the operations
* one needs during a system power up to get the hardware operational.
*/
class GrpBasicInit : public Group
{
public:
    GrpBasicInit(size_t grpNum, SpecRev specRev, int fd);
    virtual ~GrpBasicInit();
};


#endif
