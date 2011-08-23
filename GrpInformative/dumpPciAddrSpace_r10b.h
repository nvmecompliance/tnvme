#ifndef _DUMPPCIADDRSPACE_r10b_H_
#define _DUMPPCIADDRSPACE_r10b_H_

#include "test.h"


/**
 * The purpose of this class resides in the constructor
 */
class DumpPciAddrSpace_r10b : public Test
{
public:
    DumpPciAddrSpace_r10b(int fd);
    virtual ~DumpPciAddrSpace_r10b();


protected:
    virtual bool RunCoreTest();


private:
    void WriteToFile(int fd, const PciSpcType regMetrics,
        unsigned long long value);
};


#endif
