#ifndef _ASQ_H_
#define _ASQ_H_

#include "sq.h"


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
     * @param life Pass the lifetime of the object being created
     * @param ownByRsrcMngr Pass true if the RsrcMngr created this obj,
     *      otherwise false.
     */
    ASQ(int fd, Trackable::Lifetime life, bool ownByRsrcMngr);
    virtual ~ASQ();

    /**
     * Initialize this object and allocates a contiguous ACQ
     * @param numEntries Pass the number of elements within the Q
     */
    void Init(uint16_t numEntries);

    /**
     * Initialize this object and allocates discontiguous ACQ.
     * @param numEntries Pass the number of elements within the Q
     * @param memBuffer Hand off a buffer which must satisfy
     *        MemBuffer.GetBufSize()>=(numEntries * entrySize). It must have
     *        the same life span as this object, it must have been created
     *        by the same means as this object, and must only ever be accessed
     *        as RO. Writing to this buffer will have unpredictable results.
     *        It will also become owned by this object, it won't have to be
     *        explicitly deleted when this object goes out of scope.
     */
    void Init(uint16_t numEntries, MemBuffer &memBuffer);


private:
    ASQ();
};


#endif
