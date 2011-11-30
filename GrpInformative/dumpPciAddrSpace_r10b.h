#ifndef _DUMPPCIADDRSPACE_r10b_H_
#define _DUMPPCIADDRSPACE_r10b_H_

#include "test.h"


/** \verbatim
 * -----------------------------------------------------------------------------
 * ----------------Mandatory rules for children to follow-----------------------
 * -----------------------------------------------------------------------------
 * 1) See notes in the header file of the Test base class
 * \endverbatim
 */
class DumpPciAddrSpace_r10b : public Test
{
public:
    DumpPciAddrSpace_r10b(int fd, string grpName, string testName);
    virtual ~DumpPciAddrSpace_r10b();

    /**
     * IMPORTANT: Read Test::Clone() header comment.
     */
    virtual DumpPciAddrSpace_r10b *Clone() const
        { return new DumpPciAddrSpace_r10b(*this); }
    DumpPciAddrSpace_r10b &operator=(const DumpPciAddrSpace_r10b &other);
    DumpPciAddrSpace_r10b(const DumpPciAddrSpace_r10b &other);


protected:
    virtual bool RunCoreTest();


private:
    ///////////////////////////////////////////////////////////////////////////
    // Adding a member variable? Then edit the copy constructor and operator=().
    ///////////////////////////////////////////////////////////////////////////

    void WriteToFile(int fd, const PciSpcType regMetrics,  uint64_t value);
};


#endif
