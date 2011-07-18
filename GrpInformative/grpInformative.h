#ifndef _GRPINFORMATIVE_H_
#define _GRPINFORMATIVE_H_

#include "group.h"

#define FILENAME_DUMP_PCI_REGS      "./dump.reg.pci"
#define FILENAME_DUMP_CTRLR_REGS    "./dump.reg.ctrlr"


/**
* This class implements a logical grouping of test cases for all NVME
* specification document releases. It is logically grouping information
* gathering duties. More specifically it is needed to gather any data
* someone would want to be dumped in order to help debug the reason for a
* test failure. These things could be the configuration of the controller
* or PCI address space, etc.
*/
class GrpInformative : public Group
{
public:
    GrpInformative(size_t grpNum, SpecRev specRev, int fd);
    virtual ~GrpInformative();
};


#endif
