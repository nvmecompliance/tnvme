#include "rsrcMngr.h"
#include "memBuffer.h"

typedef pair<string, SharedTrackablePtr> TrackablePair;
SharedTrackablePtr RsrcMngr::NullTrackablePtr;

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
RsrcMngr::AllocObj(Trackable::ObjType type, Trackable::Lifetime life)
{
    string lt = (life == Trackable::LIFE_TEST) ? "test" : "group";

    switch (type) {
    case Trackable::OBJ_MEMBUFFER:
        LOG_NRM("Obj MemBuffer is created with %s lifetime", lt.c_str());
        return SharedTrackablePtr(new MemBuffer(life, true));
        break;

    default:
        LOG_DBG("Unknown obj type specified: 0x%02X", type);
        break;
    }

    return NullTrackablePtr;
}


SharedTrackablePtr
RsrcMngr::AllocObjGrpLife(Trackable::ObjType type, string lookupName)
{
    if (lookupName.length() == 0) {
        LOG_DBG("Parameter lookupName has no value");
        return NullTrackablePtr;
    }

    SharedTrackablePtr newObj = AllocObj(type, Trackable::LIFE_GROUP);
    if (newObj == NullTrackablePtr) {
        LOG_DBG("System unable to create object from heap");
        return NullTrackablePtr;
    }

    // Store this allocated object in a more permanent container
    pair<TrackableMap::iterator, bool> result;
    result = mObjGrpLife.insert(TrackablePair(lookupName, newObj));
    if (result.second == false) {
        LOG_DBG("Created object with collisions in lookupName: %s",
            lookupName.c_str());
        return NullTrackablePtr;
    }

    return GetObjGrpLife(lookupName);
}


SharedTrackablePtr
RsrcMngr::AllocObjTestLife(Trackable::ObjType type)
{
    SharedTrackablePtr newObj = AllocObj(type, Trackable::LIFE_TEST);
    if (newObj == NullTrackablePtr) {
        LOG_DBG("System unable to create object from heap");
        return NullTrackablePtr;
    }

    // Store this allocated object in a more permanent container
    mObjTesLife.push_back(newObj);
    return newObj;
}


SharedTrackablePtr
RsrcMngr::GetObjGrpLife(string lookupName)
{
    TrackableMap::iterator item;

    item = mObjGrpLife.find(lookupName);
    if (item == mObjGrpLife.end())
        return SharedTrackablePtr();  // (SharedTrackablePtr->expired() == true)
    return (*item).second;
}


void
RsrcMngr::FreeObjGrpLife()
{
    // Enforce that test lifetimes are shorter than group lifetimes.
    FreeObjTestLife();

    // By removing all pointers contained within the container it will destroy
    // the contained share_ptr's and thus the objects being pointed to. This, of
    // course, assumes that all other shared_ptr's to those objects have been
    // destroyed. This should be the case since the resource manager creates
    // objects on behalf of tests and all test objects within a group are
    // deleted after they complete, i.e. replace with TestNULL objects.
    LOG_NRM("Group level resources are being freed: %ld", mObjGrpLife.size());
    mObjGrpLife.clear();
}


void
RsrcMngr::FreeObjTestLife()
{
    // By removing all pointers contained within the container it will destroy
    // the contained share_ptr's and thus the objects being pointed to. This, of
    // course, assumes that all other shared_ptr's to those objects have been
    // destroyed. This should be the case since the resource manager creates
    // objects on behalf of tests and all test objects within a group are
    // deleted after they complete, i.e. replace with TestNULL objects.
    LOG_NRM("Test level resources are being freed: %ld", mObjTesLife.size());
    mObjTesLife.clear();
}
