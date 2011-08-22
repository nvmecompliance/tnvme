#ifndef _DUMPCTRLRADDRSPACE_r10a_H_
#define _DUMPPCIADDRSPACE_r10a_H_

#include "test.h"


/**
 * The purpose of this class resides in the constructor
 */
class DumpCtrlrAddrSpace_r10a : public Test
{
public:
    DumpCtrlrAddrSpace_r10a(int fd);
    virtual ~DumpCtrlrAddrSpace_r10a();


protected:
    virtual bool RunCoreTest();

};


#endif
