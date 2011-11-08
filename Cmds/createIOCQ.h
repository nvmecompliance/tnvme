#ifndef _CREATEIOCQ_H_
#define _CREATEIOCQ_H_

#include "cmd.h"


/**
* This class implements the Create IO Completion Queue admin cmd
*
* @note This class may throw exceptions.
*/
class CreateIOCQ : public Cmd
{
public:
    /**
     * @param fd Pass the opened file descriptor for the device under test
     */
    CreateIOCQ(int fd);
    virtual ~CreateIOCQ();


private:
    CreateIOCQ();
};


#endif
