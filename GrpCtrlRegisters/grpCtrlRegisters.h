#ifndef _GRPCTRLREGISTERS_H_
#define _GRPCTRLREGISTERS_H_

#include <vector>
#include "group.h"

using namespace std;


/**
* This class implements a logical grouping of test cases for all NVME
* specification document releases.
*/
class GrpCtrlRegisters : public Group
{
public:
    GrpCtrlRegisters(size_t grpNum, SpecRevType specRev);
    virtual ~GrpCtrlRegisters();
};


#endif
