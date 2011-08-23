#ifndef _DUMPCTRLRADDRSPACE_r10b_H_
#define _DUMPPCIADDRSPACE_r10b_H_

#include "test.h"


/**
 * The purpose of this class resides in the constructor
 */
class DumpCtrlrAddrSpace_r10b : public Test
{
public:
    DumpCtrlrAddrSpace_r10b(int fd);
    virtual ~DumpCtrlrAddrSpace_r10b();


protected:
    virtual bool RunCoreTest();

};


#endif
