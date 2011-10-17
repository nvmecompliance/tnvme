#ifndef _GRPTEMPLATE_H_
#define _GRPTEMPLATE_H_

#include "../group.h"


/**
* This class implements a template for all groups to come.
*/
class GrpTemplate : public Group
{
public:
    GrpTemplate(size_t grpNum, SpecRev specRev, int fd);
    virtual ~GrpTemplate();
};


#endif
