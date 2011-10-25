#include "trackable.h"
#include "tnvme.h"


Trackable::Trackable(ObjType objBeingCreated, Lifetime objLife,
    bool ownByRsrcMngr)
{
    if (objBeingCreated >= OBJTYPE_FENCE) {
        LOG_DBG("Illegal constructor");
        throw exception();
    }
    mObjType = objBeingCreated;
    mObjLife = objLife;
    mOwnByRsrcMngr = ownByRsrcMngr;
}


Trackable::~Trackable()
{
    switch (mObjType) {
    case OBJ_MEMBUFFER:     LOG_DBG("Destructing obj MemBuffer");       break;
    default:                LOG_DBG("Destructing obj unknown");         break;
    }
}
