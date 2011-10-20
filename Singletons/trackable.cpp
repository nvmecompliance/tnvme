#include "trackable.h"
#include "tnvme.h"


Trackable::Trackable(ObjType objBeingCreated)
{
    mObjType = objBeingCreated;
}


Trackable::~Trackable()
{
    switch (mObjType) {
    case OBJ_MEMBUFFER:     LOG_DBG("Destructing obj MemBuffer");       break;
    default:                LOG_DBG("Destructing obj unknown");         break;
    }
}
