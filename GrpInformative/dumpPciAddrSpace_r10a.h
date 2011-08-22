#ifndef _DUMPPCIADDRSPACE_r10a_H_
#define _DUMPPCIADDRSPACE_r10a_H_

#include "test.h"


/**
 * The purpose of this class resides in the constructor
 */
class DumpPciAddrSpace_r10a : public Test
{
public:
    DumpPciAddrSpace_r10a(int fd);
    virtual ~DumpPciAddrSpace_r10a();


protected:
    virtual bool RunCoreTest();


private:
    void WriteToFile(int fd, const PciSpcType regMetrics,
        unsigned long long value);
};


#endif
