#ifndef _DELETEIOCQ_H_
#define _DELETEIOCQ_H_

#include "adminCmd.h"
#include "../Queues/iocq.h"


class DeleteIOCQ;    // forward definition
typedef boost::shared_ptr<DeleteIOCQ>             SharedDeleteIOCQPtr;
#define CAST_TO_DELETEIOCQ(shared_trackable_ptr)  \
        boost::shared_polymorphic_downcast<DeleteIOCQ>(shared_trackable_ptr);


/**
* This class implements the Delete IO Completion Queue admin cmd. After
* instantiation the Init() methods must be called to attain something useful.
*
* @note This class may throw exceptions.
*/
class DeleteIOCQ : public AdminCmd
{
public:
    DeleteIOCQ(int fd);
    virtual ~DeleteIOCQ();

    /// Used to compare for NULL pointers being returned by allocations
    static SharedDeleteIOCQPtr NullDeleteIOCQPtr;

    /**
     * Initialize this object and prepares it to send to the hdw.
     * @param iocq Pass the IOCQ object which will initialize this cmd.
     */
    void Init(const SharedIOCQPtr iocq);


private:
    DeleteIOCQ();
};


#endif
