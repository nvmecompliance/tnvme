#ifndef _ALLCTRLREGS_r10a_H_
#define _ALLCTRLREGS_r10a_H_

#include "test.h"


class AllCtrlRegs_r10a : public Test
{
public:
    AllCtrlRegs_r10a(int fd);
    virtual ~AllCtrlRegs_r10a();


protected:
    virtual bool RunCoreTest();

};


#endif
