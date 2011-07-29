#ifndef _ALLPCIREGS_r10a_H_
#define _ALLPCIREGS_r10a_H_

#include "test.h"


class AllPciRegs_r10a : public Test
{
public:
    AllPciRegs_r10a(int fd);
    virtual ~AllPciRegs_r10a();


protected:
    virtual bool RunCoreTest();

};


#endif
