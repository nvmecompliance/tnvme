#ifndef _METARSRC_H_
#define _METARSRC_H_

#include <deque>
#include "tnvme.h"

#define METADATA_UNIQUE_ID_BITS         18      // 18 bits to rep a unique ID


/**
* This base class will handle the controlling aspect of meta data management in
* user space. Meta data buffers are separated into 2 components. The controlling
* component and the working component. Please see class header MetaData for
* details concerning the working component.
*
* Meta data buffers are allocated in dnvme. Each new allocation is the same
* size as the prior. The allocation size is controlled by this class. Once set
* the allocation size is not allowed to be changed until the ctrlr is disabled
* by CtrlrConfig::SetState(DISABLE_XXXX).
*
* This class also tracks in use meta data IDs. These IDs are used to reference
* allocated meta data buffers. The disabling of the controller releases all
* memory and meta data IDs. If a new ID is desired this class will yield a
* non-reserved unique meta data ID. Remember to release previously reserved ID's
* because they are limited in number.
*
* @note This class does not throw exceptions.
*/
class MetaRsrc
{
public:
    /**
     * @param fd Pass the opened file descriptor for the device under test
     */
    MetaRsrc(int fd);
    ~MetaRsrc();

    /**
     * Sets the size of meta data buffer allocation within the kernel. Each
     * allocation requested by MetaData::AllocBuffer() must and will be of the
     * same size, as will be set by this method only. This allocation size
     * will persist, unchangeable until such time a
     * gCtrlrConfig->SetState(DISABLE_XXX) occurs, at which time this method
     * can be called again to setup a new allocation size.
     * @param allocSize Pass the requested size of meta data buffers; The
     *      value must be modulo sizeof(uint32_t). The max allowed size = 16KB.
     * @return true upon success, otherwise false
     */
    bool     SetMetaAllocSize(uint16_t allocSize);
    uint16_t GetMetaAllocSize() { return mMetaAllocSize; }

    /**
     * Meta data buffer's are tracked within the kernel by a unique reference
     * number. The burden of managing these ID's is left to user space, and this
     * object tracks all meta data unique ID's. Thus, if a meta data buffer is
     * needed an object must first reserve a unique ID to pass to the kernel
     * which will associate the allocated memory to this ID. Henceforth that
     * kernel memory is referenced by its ID.
     * @param uniqueId Returns the next available unique ID
     * @return true when a unique ID is available, otherwise false
     */
    bool ReserveMetaId(uint32_t &uniqueId);
    void ReleaseMetaId(uint32_t uniqueId);


protected:
    void ReleaseAllMetaId();


private:
    MetaRsrc();

    /// file descriptor to the device under test
    int mFd;

    /// Stores the size of each meta data allocation
    uint16_t mMetaAllocSize;
    /// Track all outstanding/reserved meta data unique IDs
    deque<uint32_t> mMetaUniqueIds;
};


#endif
