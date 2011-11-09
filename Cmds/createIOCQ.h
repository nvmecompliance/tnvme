#ifndef _CREATEIOCQ_H_
#define _CREATEIOCQ_H_

#include "cmd.h"

class CreateIOCQ;    // forward definition
typedef boost::shared_ptr<CreateIOCQ>               SharedCreateIOCQPtr;
#define CAST_TO_CreateIOCQ(shared_trackable_ptr)    \
        boost::shared_polymorphic_downcast<CreateIOCQ>(shared_trackable_ptr);


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
