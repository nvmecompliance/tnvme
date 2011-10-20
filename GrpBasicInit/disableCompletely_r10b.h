#ifndef _DISABLECOMPLETELY_r10b_H_
#define _DISABLECOMPLETELY_r10b_H_

#include "test.h"


/**
 * -----------------------------------------------------------------------------
 * ----------------Mandatory rules for children to follow-----------------------
 * -----------------------------------------------------------------------------
 * 1) See notes in the header file of the Test base class
 */
class DisableCompletely_r10b : public Test
{
public:
    DisableCompletely_r10b(int fd);
    virtual ~DisableCompletely_r10b();

    virtual DisableCompletely_r10b *Clone() const
        { return new DisableCompletely_r10b(*this); }


protected:
    virtual bool RunCoreTest();


private:
    ///////////////////////////////////////////////////////////////////////////
    // Adding a member variable? Think carefully, see Test::Clone() hdr comment
    // Adding a member functions is fine.
    ///////////////////////////////////////////////////////////////////////////
};


#endif
