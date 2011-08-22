#ifndef _ALLCTRLREGS_r10a_H_
#define _ALLCTRLREGS_r10a_H_

#include "test.h"


/**
 * The purpose of this class resides in the constructor
 */
class AllCtrlRegs_r10a : public Test
{
public:
    AllCtrlRegs_r10a(int fd);
    virtual ~AllCtrlRegs_r10a();


protected:
    virtual bool RunCoreTest();

};


#endif
