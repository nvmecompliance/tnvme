#include "ctrlrConfig.h"

bool CtrlrConfig::mInstanceFlag = false;
CtrlrConfig* CtrlrConfig::mSingleton = NULL;
CtrlrConfig* CtrlrConfig::GetInstance(int fd, SpecRev specRev)
{
    if(mInstanceFlag == false) {
        mSingleton = new CtrlrConfig(fd, specRev);
        mInstanceFlag = true;
        return mSingleton;
    } else {
        return mSingleton;
    }
}
void CtrlrConfig::KillInstance()
{
    if(mInstanceFlag) {
        mInstanceFlag = false;
        delete mSingleton;
        mSingleton = NULL;
    }
}


CtrlrConfig::CtrlrConfig(int fd, SpecRev specRev)
{
    mFd = fd;
    if (mFd < 0) {
        LOG_DBG("Object created with a bad FD=%d", fd);
        return;
    }

    mSpecRev = specRev;
}


CtrlrConfig::~CtrlrConfig()
{
    mInstanceFlag = false;
}
