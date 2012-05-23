/*
 * Copyright (c) 2011, Intel Corporation.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include "rsrcMngr.h"
#include "../Exception/frmwkEx.h"


bool RsrcMngr::mInstanceFlag = false;
RsrcMngr *RsrcMngr::mSingleton = NULL;
RsrcMngr *RsrcMngr::GetInstance(int fd, SpecRev specRev)
{
    LOG_NRM("Instantiating global RsrcMngr object");
    if(mInstanceFlag == false) {
        mSingleton = new RsrcMngr(fd, specRev);
        mInstanceFlag = true;
    }
    return mSingleton;
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
    if (mFd < 0)
        throw FrmwkEx("Object created with a bad FD=%d", fd);

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
