#include "rsrcMngr.h"


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


RsrcMngr::RsrcMngr(int fd, SpecRev specRev) :
    ObjRsrc(fd), MetaRsrc(fd)
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


void
RsrcMngr::Update(const bool &disabled)
{
    // The disabling of the ctrlr causes all kernel objects to be dealloc'd...
    if (disabled == true) {
        // All outstanding Q memory will be released.
        LOG_DBG("Disabling causes all Q memory to be released");
        FreeAllObj();

        // All outstanding meta data buffer ID's will be released.
        LOG_DBG("Disabling causes all meta unique ID's to be released");
        ReleaseAllMetaId();
    }
}
