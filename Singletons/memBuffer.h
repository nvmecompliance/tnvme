#ifndef _MEMBUFFER_H_
#define _MEMBUFFER_H_

#include "../tnvme.h"
#include "trackable.h"


/**
* This class is a wrapper for a raw heap allocated memory buffer. It may
* be created and destroyed by the RsrcMngr:: however that is not strictly
* necessary. These buffers can be specified to have certain alignment criteria
* to be used for CQ/SQ memory and user data buffers. After instantiation the
* Initxxxxxx() methods must be called to attain something useful.
*
* @note This class may throw exceptions.
*/
class MemBuffer : public Trackable
{
public:
    /**
     * @param life Pass the lifetime of the object being created
     * @param ownByRsrcMngr Pass true if the RsrcMngr created this obj,
     *      otherwise false.
     */
    MemBuffer(Trackable::Lifetime life, bool ownByRsrcMngr = false);
    virtual ~MemBuffer();

    /**
     * Allocates memory allowing to specify an offset into the 1st page of
     * allocated memory, and thus also the alignment of that buffer. More memory
     * may be allocated than requested to satisfy the request. If a specific
     * offset into a page is not strictly necessary then calling
     * InitAlignment() should be more efficient at allocations, because entire
     * pages of memory may not be required to satisfy the request.
     * @param bufSize Pass the minimum number of bytes for buffer creation
     * @param initMem Pass true to initialize all elements, otherwise don't init
     * @param initVal Pass the init value if suppose to init the buffer
     * @param offset1stPg Pass the byte offset into the 1st page of allocated
     *        memory which the buffer is intended to start. offset == 0 implies
     *        page aligned memory. This value is enforced to a multiple of 32
     *        bit alignment.
     */
    void InitOffset1stPage(uint32_t bufSize, bool initMem = false,
        uint8_t initVal = 0, uint32_t offset1stPg = 0);

    /**
     * Allocates memory allowing to specify the alignment of that buffer, but
     * not the offset into the 1st page of the allocation. Residual memory
     * may be consumed to satisfy this request since entire pages of memory
     * may not be necessary, and therefore the allocation of > what was
     * requested will not be necessary.
     * @param bufSize Pass the number of bytes for buffer creation
     * @param initMem Pass true to initialize all elements, otherwise don't init
     * @param initVal Pass the init value if suppose to init the buffer
     * @param align Pass the alignment requirements of the buffer. This value
     *        is enforced to a multiple of sizeof(void *) alignment.
     */
    void InitAlignment(uint32_t bufSize, bool initMem = false,
        uint8_t initVal = 0, uint32_t align = 0);

    uint8_t *GetBuffer() { return mVirBaseAddr; }
    uint32_t GetBufSize() { return mVirBufSize; }
    uint32_t GetAlignment() { return mAlignment; }

    /**
     * Zero out all memory bytes
     */
    void Reset();


private:
    MemBuffer();

    uint8_t *mRealBaseAddr;     // System address returned by posix_memalign()
    uint8_t *mVirBaseAddr;      // User buffer address to satisfy mOffset1stPg
    uint32_t mVirBufSize;       // User request buffer size
    uint32_t mAlignment;

    void InitMemberVariables();
};


#endif
