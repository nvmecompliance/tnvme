#ifndef _ASQ_H_
#define _ASQ_H_

#include "sq.h"

class ASQ;    // forward definition
typedef boost::shared_ptr<ASQ>        SharedASQPtr;
#define CAST_TO_ASQ(shared_trackable_ptr)  \
        boost::shared_polymorphic_downcast<ASQ>(shared_trackable_ptr);


/**
* This class is meant to be instantiated and represents an ASQ. After
* instantiation the Init() methods must be called to attain something useful.
*
* @note This class may throw exceptions.
*/
class ASQ : public SQ
{
public:
    /**
     * @param fd Pass the opened file descriptor for the device under test
     */
    ASQ(int fd);
    virtual ~ASQ();

    /// Used to compare for NULL pointers being returned by allocations
    static SharedASQPtr NullASQPtr;

    /**
     * Initialize this object and allocates a contiguous ACQ
     * @param numEntries Pass the number of elements within the Q
     */
    void Init(uint16_t numEntries);


private:
    ASQ();
};


#endif
