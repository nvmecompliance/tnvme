#include "rsrcMngr.h"
#include "memBuffer.h"
#include "../Queues/acq.h"
#include "../Queues/asq.h"
#include "../Queues/iocq.h"
#include "../Queues/iosq.h"


typedef pair<string, SharedTrackablePtr> TrackablePair;


bool RsrcMngr::mInstanceFlag = false;
RsrcMngr* RsrcMngr::mSingleton = NULL;
RsrcMngr* RsrcMngr::GetInstance(int fd, SpecRev specRev)
{
    if(mInstanceFlag == false) {
        mSingleton = new RsrcMngr(fd, specRev);
        mInstanceFlag = true;
        return mSingleton;
    } else {
        return mSingleton;
    }
}
void RsrcMngr::KillInstance()
{
    if(mInstanceFlag) {
        mInstanceFlag = false;
        delete mSingleton;
        mSingleton = NULL;
    }
}


RsrcMngr::RsrcMngr(int fd, SpecRev specRev)
{
    mFd = fd;
    if (mFd < 0) {
        LOG_DBG("Object created with a bad FD=%d", fd);
        return;
    }

    mSpecRev = specRev;
}


RsrcMngr::~RsrcMngr()
{
    mInstanceFlag = false;
}


SharedTrackablePtr
RsrcMngr::Allocate(Trackable::ObjType type)
{
    switch (type) {
    case Trackable::OBJ_MEMBUFFER:
        LOG_NRM("Obj MemBuffer is born with group lifetime");
        return SharedTrackablePtr(new MemBuffer());
        break;

    case Trackable::OBJ_ACQ:
        LOG_NRM("Obj ACQ is born with group lifetime");
        return SharedTrackablePtr(new ACQ(mFd));
        break;

    case Trackable::OBJ_ASQ:
        LOG_NRM("Obj ASQ is born with group lifetime");
        return SharedTrackablePtr(new ASQ(mFd));
        break;

    case Trackable::OBJ_IOCQ:
        LOG_NRM("Obj IOCQ is born with group lifetime");
        return SharedTrackablePtr(new IOCQ(mFd));
        break;

    case Trackable::OBJ_IOSQ:
        LOG_NRM("Obj IOSQ is born with group lifetime");
        return SharedTrackablePtr(new IOSQ(mFd));
        break;

    default:
        LOG_DBG("Unknown obj type specified: 0x%02X", type);
        break;
    }

    return Trackable::NullTrackablePtr;
}


SharedTrackablePtr
RsrcMngr::AllocObj(Trackable::ObjType type, string lookupName)
{
    if (lookupName.length() == 0) {
        LOG_DBG("Parameter lookupName has no value");
        return Trackable::NullTrackablePtr;
    }

    SharedTrackablePtr newObj = Allocate(type);
    if (newObj == Trackable::NullTrackablePtr) {
        LOG_DBG("System unable to create object from heap");
        return Trackable::NullTrackablePtr;
    }

    // Store this allocated object in a more permanent container
    pair<TrackableMap::iterator, bool> result;
    result = mObjGrpLife.insert(TrackablePair(lookupName, newObj));
    if (result.second == false) {
        LOG_DBG("Created object with collisions in lookupName: %s",
            lookupName.c_str());
        return Trackable::NullTrackablePtr;
    }

    return GetObj(lookupName);
}


SharedTrackablePtr
RsrcMngr::GetObj(string lookupName)
{
    TrackableMap::iterator item;

    item = mObjGrpLife.find(lookupName);
    if (item == mObjGrpLife.end())
        return SharedTrackablePtr();  // (SharedTrackablePtr->expired() == true)
    return (*item).second;
}


void
RsrcMngr::FreeObj()
{
    // By removing all pointers contained within the container it will destroy
    // the contained share_ptr's and thus the objects being pointed to. This, of
    // course, assumes that all other shared_ptr's to those objects have been
    // destroyed. This should be the case since the resource manager creates
    // objects on behalf of tests and all test objects within a group are
    // deleted after they complete, i.e. replace with TestNULL objects.
    LOG_NRM("Group level resources are being freed: %ld", mObjGrpLife.size());
    mObjGrpLife.clear();
}
