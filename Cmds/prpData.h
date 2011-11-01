#ifndef _PRPDATA_H_
#define _PRPDATA_H_

#include "tnvme.h"
#include "dnvme.h"
#include "../Singletons/memBuffer.h"


/**
* This class is the interface for the user data buffer associated with the PRP
* entries of a cmd.
*
* @note This class may throw exceptions.
*/
class PrpData
{
public:
    /**
     * @param life Pass the lifetime of the object being create
     * @param ownByRsrcMngr Pass true if the RsrcMngr created this obj,
     *      otherwise false.
     */
    PrpData(Trackable::Lifetime life, bool ownByRsrcMngr);
    virtual ~PrpData();

    /**
     * Accept a previously created user space buffer as the user data buffer to
     * be populated in the PRP fields of a cmd.
     * @param memBuffer Hand off a buffer to be assoc. with this cmd. It must
     *      have the same life span as this object, it must have been created
     *      by the same means as this object. It will also become owned by
     *      this object, it won't have to be explicitly deleted when this
     *      object goes out of scope.
     * @param prpFields Pass the appropriate combination of bitfields to
     *      indicate to dnvme how to populate the PRP fields of a cmd with
     *      this the buffer.
     */
    void SetBuffer(MemBuffer &memBuffer, send_64b_bitmask prpFields);

    MemBuffer *GetBuffer() { return mBuf; }

    send_64b_bitmask GetPrpFields() { return mPrpFields; }


private:
    PrpData();

    Trackable::Lifetime mLifetime;
    bool mOwnByRsrcMngr;
    MemBuffer *mBuf;

    /// What fields in a cmd can we populate for the buffer?
    send_64b_bitmask mPrpFields;
};


#endif
