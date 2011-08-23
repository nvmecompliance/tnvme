#ifndef _ALLCTRLREGS_r10b_H_
#define _ALLCTRLREGS_r10b_H_

#include "test.h"


/**
 * The purpose of this class resides in the constructor
 */
class AllCtrlRegs_r10b : public Test
{
public:
    AllCtrlRegs_r10b(int fd);
    virtual ~AllCtrlRegs_r10b();


protected:
    virtual bool RunCoreTest();

};


#endif
