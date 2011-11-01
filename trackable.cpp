#include "trackable.h"
#include "tnvme.h"


SharedTrackablePtr Trackable::NullTrackablePtr;


Trackable::Trackable(ObjType objBeingCreated)
{
    if (objBeingCreated >= OBJTYPE_FENCE) {
        LOG_DBG("Illegal constructor");
        throw exception();
    }

    mObjType = objBeingCreated;
}


Trackable::~Trackable()
{
    switch (mObjType) {
    case OBJ_MEMBUFFER:     LOG_DBG("Destroying obj MemBuffer");       break;
    case OBJ_ACQ:           LOG_DBG("Destroying obj ACQ");             break;
    case OBJ_ASQ:           LOG_DBG("Destroying obj ASQ");             break;
    case OBJ_IOCQ:          LOG_DBG("Destroying obj IOCQ");            break;
    case OBJ_IOSQ:          LOG_DBG("Destroying obj IOSQ");            break;
    default:                LOG_DBG("Destroying obj unknown");         break;
    }
}
