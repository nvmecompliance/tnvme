#ifndef _DUMPPCIADDRSPACE_r10a_H_
#define _DUMPPCIADDRSPACE_r10a_H_

#include "test.h"


class DumpPciAddrSpace_r10a : public Test
{
public:
    DumpPciAddrSpace_r10a(int fd);
    virtual ~DumpPciAddrSpace_r10a();


protected:
    virtual bool RunCoreTest();

};


#endif
