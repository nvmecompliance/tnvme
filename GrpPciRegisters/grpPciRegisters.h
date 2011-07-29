#ifndef _GRPPCIREGISTERS_H_
#define _GRPPCIREGISTERS_H_

#include "../group.h"

/**
* This class implements a logical grouping of test cases for all NVME
* specification document releases. It is logically grouping the PCI space
* register syntactical compliance.
*/
class GrpPciRegisters : public Group
{
public:
    GrpPciRegisters(size_t grpNum, SpecRev specRev, int fd);
    virtual ~GrpPciRegisters();
};


#endif
