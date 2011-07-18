#ifndef _CTRLCAPABILITIES_r10a_H_
#define _CTRLCAPABILITIES_r10a_H_

#include "test.h"


class CtrlCapabilities_r10a : public Test
{
public:
    CtrlCapabilities_r10a(int fd);
    virtual ~CtrlCapabilities_r10a();


protected:
    virtual bool RunCoreTest();

};


#endif
