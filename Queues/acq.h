#ifndef _ACQ_H_
#define _ACQ_H_

#include "cq.h"

class ACQ;    // forward definition
typedef boost::shared_ptr<ACQ>        SharedACQPtr;
#define CAST_TO_ACQ(shared_trackable_ptr)  \
        boost::shared_polymorphic_downcast<ACQ>(shared_trackable_ptr);


/**
* This class is meant to be instantiated and represents an ACQ. After
* instantiation the Init() methods must be called to attain something useful.
*
* @note This class may throw exceptions.
*/
class ACQ : public CQ
{
public:
    /**
     * @param fd Pass the opened file descriptor for the device under test
     */
    ACQ(int fd);
    virtual ~ACQ();

    /// Used to compare for NULL pointers being returned by allocations
    static SharedACQPtr NullACQPtr;

    /**
     * Initialize this object and allocates a contiguous ACQ
     * @param numEntries Pass the number of elements within the Q
     */
    void Init(uint16_t numEntries);


private:
    ACQ();
};


#endif
