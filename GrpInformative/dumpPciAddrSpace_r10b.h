#ifndef _DUMPPCIADDRSPACE_r10b_H_
#define _DUMPPCIADDRSPACE_r10b_H_

#include "test.h"


/**
 * -----------------------------------------------------------------------------
 * ----------------Mandatory rules for children to follow-----------------------
 * -----------------------------------------------------------------------------
 * 1) See notes in the header file of the Test base class
 */
class DumpPciAddrSpace_r10b : public Test
{
public:
    DumpPciAddrSpace_r10b(int fd);
    virtual ~DumpPciAddrSpace_r10b();

    virtual DumpPciAddrSpace_r10b *Clone() const
        { return new DumpPciAddrSpace_r10b(*this); }


protected:
    virtual bool RunCoreTest();


private:
    ///////////////////////////////////////////////////////////////////////////
    // Adding a member variable? Think carefully, see Test::Clone() hdr comment
    // Adding a member functions is fine.
    ///////////////////////////////////////////////////////////////////////////

    void WriteToFile(int fd, const PciSpcType regMetrics,  uint64_t value);
};


#endif
