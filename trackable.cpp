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
    string obj;

    switch (mObjType) {
    case OBJ_MEMBUFFER:     obj = "MemBuffer";          break;
    case OBJ_ACQ:           obj = "ACQ";                break;
    case OBJ_ASQ:           obj = "ASQ";                break;
    case OBJ_IOCQ:          obj = "IOCQ";               break;
    case OBJ_IOSQ:          obj = "IOSQ";               break;
    case OBJ_IDENTIFY:      obj = "Identify";           break;
    case OBJ_CREATEIOCQ:    obj = "CreateIOCQ";         break;
    default:                obj = "unknown";            break;
    }
    LOG_DBG("Destroying obj: %s", obj.c_str());
}
