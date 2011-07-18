#ifndef _GRPCTRLREGISTERS_H_
#define _GRPCTRLREGISTERS_H_

#include "group.h"


/**
* This class implements a logical grouping of test cases for all NVME
* specification document releases. It is logically grouping the PCI BAR0/BAR1,
* i.e. all the NVME controller's registers address space for syntactical
* compliance.
*/
class GrpCtrlRegisters : public Group
{
public:
    GrpCtrlRegisters(size_t grpNum, SpecRev specRev, int fd);
    virtual ~GrpCtrlRegisters();
};


#endif
