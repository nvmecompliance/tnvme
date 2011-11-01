#ifndef _DUMPCTRLRADDRSPACE_r10b_H_
#define _DUMPPCIADDRSPACE_r10b_H_

#include "test.h"


/**
 * -----------------------------------------------------------------------------
 * ----------------Mandatory rules for children to follow-----------------------
 * -----------------------------------------------------------------------------
 * 1) See notes in the header file of the Test base class
 */
class DumpCtrlrAddrSpace_r10b : public Test
{
public:
    DumpCtrlrAddrSpace_r10b(int fd);
    virtual ~DumpCtrlrAddrSpace_r10b();

    /**
     * IMPORTANT: Read Test::Clone() header comment.
     */
    virtual DumpCtrlrAddrSpace_r10b *Clone() const
        { return new DumpCtrlrAddrSpace_r10b(*this); }
    DumpCtrlrAddrSpace_r10b &operator=(const DumpCtrlrAddrSpace_r10b &other);
    DumpCtrlrAddrSpace_r10b(const DumpCtrlrAddrSpace_r10b &other);


protected:
    virtual bool RunCoreTest();


private:
    ///////////////////////////////////////////////////////////////////////////
    // Adding a member variable? Then edit the copy constructor and operator().
    ///////////////////////////////////////////////////////////////////////////
};


#endif
