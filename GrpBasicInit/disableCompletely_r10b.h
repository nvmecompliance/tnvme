#ifndef _DISABLECOMPLETELY_r10b_H_
#define _DISABLECOMPLETELY_r10b_H_

#include "test.h"


/**
 * The purpose of this class resides in the constructor
 */
class DisableCompletely_r10b : public Test
{
public:
    DisableCompletely_r10b(int fd);
    virtual ~DisableCompletely_r10b();


protected:
    virtual bool RunCoreTest();


private:
};


#endif
