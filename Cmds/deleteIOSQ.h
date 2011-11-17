#ifndef _DELETEIOSQ_H_
#define _DELETEIOSQ_H_

#include "adminCmd.h"
#include "../Queues/iosq.h"


class DeleteIOSQ;    // forward definition
typedef boost::shared_ptr<DeleteIOSQ>             SharedDeleteIOSQPtr;
#define CAST_TO_DELETEIOSQ(shared_trackable_ptr)  \
        boost::shared_polymorphic_downcast<DeleteIOSQ>(shared_trackable_ptr);


/**
* This class implements the Delete IO Submission Queue admin cmd. After
* instantiation the Init() methods must be called to attain something useful.
*
* @note This class may throw exceptions.
*/
class DeleteIOSQ : public AdminCmd
{
public:
    DeleteIOSQ(int fd);
    virtual ~DeleteIOSQ();

    /// Used to compare for NULL pointers being returned by allocations
    static SharedDeleteIOSQPtr NullDeleteIOSQPtr;

    /**
     * Initialize this object and prepares it to send to the hdw.
     * @param iosq Pass the IOCQ object which will initialize this cmd.
     */
    void Init(const SharedIOSQPtr iosq);


private:
    DeleteIOSQ();
};


#endif
