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
RsrcMngr::Update(const enum nvme_state &state)
{
    // The disabling of the ctrlr causes kernel objects to be dealloc'd...
    if (state == ST_DISABLE_COMPLETELY) {
        // All outstanding Q memory will be released.
        LOG_DBG("Disabling causes all Q mem freed");
        FreeAllObj();

        // All outstanding meta data buffer ID's will be released.
        LOG_DBG("Disabling causes all meta unique ID's freed");
        FreeAllMetaBuf();
    } else if (state == ST_DISABLE) {
        // Only ACQ/ASQ objects remain, all others must be released
        LOG_DBG("Disabling causes all Q mem freed, but not ACQ/ASQ");
        FreeAllObjNotASQACQ();

        // All outstanding meta data buffer ID's will be released.
        LOG_DBG("Disabling causes all meta unique ID's freed");
        FreeAllMetaBuf();
    }
}
