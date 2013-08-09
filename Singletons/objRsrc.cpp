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

#include "objRsrc.h"
#include "memBuffer.h"
#include "../Queues/acq.h"
#include "../Queues/asq.h"
#include "../Queues/iocq.h"
#include "../Queues/iosq.h"
#include "../Cmds/adminCmd.h"
#include "../Cmds/nvmCmd.h"
#include "../Cmds/identify.h"
#include "../Cmds/createIOCQ.h"
#include "../Cmds/createIOSQ.h"
#include "../Cmds/deleteIOCQ.h"
#include "../Cmds/deleteIOSQ.h"
#include "../Cmds/getFeatures.h"
#include "../Cmds/setFeatures.h"
#include "../Cmds/getLogPage.h"
#include "../Cmds/fwActivate.h"
#include "../Cmds/fwImgDnld.h"
#include "../Cmds/formatNVM.h"
#include "../Cmds/baseSecurity.h"
#include "../Cmds/securitySend.h"
#include "../Cmds/securityRcv.h"
#include "../Cmds/write.h"
#include "../Cmds/read.h"
#include "../Cmds/flush.h"
#include "../Cmds/datasetMgmt.h"
#include "../Cmds/asyncEventReq.h"

/**
 * Instantiate a class.
 * @param capital Pass the class name in all CAPITAL letters
 * @param proper Pass the proper name of the class to instantiate
 */
#define INSTANTIATE_OBJ(capital, proper)                                       \
    case Trackable::OBJ_ ## capital:                                           \
        LOG_NRM("Obj %s is born with group lifetime", #proper);                \
        return SharedTrackablePtr(new proper());                               \
        break;
#define INSTANTIATE_OBJ_w_fd(capital, proper)                                  \
    case Trackable::OBJ_ ## capital:                                           \
        LOG_NRM("Obj %s is born with group lifetime", #proper);                \
        return SharedTrackablePtr(new proper(mFd));                            \
        break;
#define INSTANTIATE_OBJ_w_type(capital, proper)                                \
    case Trackable::OBJ_ ## capital:                                           \
        LOG_NRM("Obj %s is born with group lifetime", #proper);                \
        return SharedTrackablePtr(new proper(Trackable::OBJ_ ## capital));     \
        break;


typedef pair<string, SharedTrackablePtr> TrackablePair;


ObjRsrc::ObjRsrc()
{
    mFd = 0;
}


ObjRsrc::ObjRsrc(int fd)
{
    mFd = fd;
    if (mFd < 0)
        throw FrmwkEx("Object created with a bad FD=%d", fd);
}


ObjRsrc::~ObjRsrc()
{
}


SharedTrackablePtr
ObjRsrc::AllocWorker(Trackable::ObjType type)
{
    // List new objects here so that ObjRsrs may instantiate any resource
    switch (type) {
    // These objects are rare, require special treatment in constructors
    INSTANTIATE_OBJ_w_type(ADMINCMD, AdminCmd)
    INSTANTIATE_OBJ_w_type(NVMCMD, NVMCmd)

    // These objects require file descriptors in their constructors
    INSTANTIATE_OBJ_w_fd(ACQ, ACQ)
    INSTANTIATE_OBJ_w_fd(ASQ, ASQ)
    INSTANTIATE_OBJ_w_fd(IOCQ, IOCQ)
    INSTANTIATE_OBJ_w_fd(IOSQ, IOSQ)

    // These objects are the basic type of initialization we should see
    INSTANTIATE_OBJ(MEMBUFFER, MemBuffer)
    INSTANTIATE_OBJ(IDENTIFY, Identify)
    INSTANTIATE_OBJ(CREATEIOCQ, CreateIOCQ)
    INSTANTIATE_OBJ(CREATEIOSQ, CreateIOSQ)
    INSTANTIATE_OBJ(DELETEIOCQ, DeleteIOCQ)
    INSTANTIATE_OBJ(DELETEIOSQ, DeleteIOSQ)
    INSTANTIATE_OBJ(GETFEATURES, GetFeatures)
    INSTANTIATE_OBJ(SETFEATURES, SetFeatures)
    INSTANTIATE_OBJ(GETLOGPAGE, GetLogPage)
    INSTANTIATE_OBJ(FWACTIVATE, FWActivate)
    INSTANTIATE_OBJ(FWIMGDNLD, FWImgDnld)
    INSTANTIATE_OBJ(FORMATNVM, FormatNVM)
    INSTANTIATE_OBJ(ASYNCEVENTREQ, AsyncEventReq)
    INSTANTIATE_OBJ(SECURITYSEND, SecuritySend)
    INSTANTIATE_OBJ(SECURITYRCV, SecurityRcv)

    INSTANTIATE_OBJ(WRITE, Write)
    INSTANTIATE_OBJ(READ, Read)
    INSTANTIATE_OBJ(FLUSH, Flush)
    INSTANTIATE_OBJ(DATASETMGMT, DatasetMgmt)

    default:
        throw FrmwkEx("Unknown obj type specified: 0x%02X", type);
    }

    return Trackable::NullTrackablePtr;
}


SharedTrackablePtr
ObjRsrc::AllocObj(Trackable::ObjType type, string lookupName)
{
    if (lookupName.length() == 0) {
        LOG_ERR("Parameter lookupName has no value");
        return Trackable::NullTrackablePtr;
    }

    SharedTrackablePtr newObj = AllocWorker(type);
    if (newObj == Trackable::NullTrackablePtr) {
        LOG_ERR("System unable to create object from heap");
        return Trackable::NullTrackablePtr;
    }

    // Store this allocated object in a more permanent container
    pair<TrackableMap::iterator, bool> result;
    result = mObjGrpLife.insert(TrackablePair(lookupName, newObj));
    if (result.second == false) {
        LOG_ERR("Created object with collisions in lookupName: %s",
            lookupName.c_str());
        return Trackable::NullTrackablePtr;
    }

    return GetObj(lookupName);
}


SharedTrackablePtr
ObjRsrc::GetObj(string lookupName)
{
    TrackableMap::iterator item;

    item = mObjGrpLife.find(lookupName);
    if (item == mObjGrpLife.end()) {
        LOG_DBG("Object lookup name %s was not found", lookupName.c_str());
        return SharedTrackablePtr();  // (SharedTrackablePtr->expired() == true)
    }
    return (*item).second;
}


void
ObjRsrc::FreeAllObj()
{
    /** @note
     * By removing all pointers contained within the container it will destroy
     * the contained share_ptr's and thus the objects being pointed to. This, of
     * course, assumes that all other shared_ptr's to those objects have been
     * destroyed. This should be the case since the resource manager creates
     * objects on behalf of tests and all test objects within a group are
     * deleted after they complete, thus removing localized share_ptr's.
     */
    LOG_NRM("Group level resources are being freed: %ld", mObjGrpLife.size());
    mObjGrpLife.clear();
}


void
ObjRsrc::FreeAllObjNotASQACQ()
{
    /** @note
     * By removing all pointers contained within the container it will destroy
     * the contained share_ptr's and thus the objects being pointed to. This, of
     * course, assumes that all other shared_ptr's to those objects have been
     * destroyed. This should be the case since the resource manager creates
     * objects on behalf of tests and all test objects within a group are
     * deleted after they complete, thus removing localized share_ptr's.
     */
    TrackableMap::iterator item;
    size_t numB4 = mObjGrpLife.size();

    item = mObjGrpLife.begin();
    while (item != mObjGrpLife.end()) {
        SharedTrackablePtr tPtr = (*item).second;
        Trackable::ObjType obj = tPtr->GetObjType();
        if ((obj != Trackable::OBJ_ACQ) && (obj != Trackable::OBJ_ASQ))
            mObjGrpLife.erase(item);
        item++;
    }
    LOG_NRM("Group level resources are being freed: %ld",
        (numB4 - mObjGrpLife.size()));
    LOG_NRM("Group level resources remaining: %ld", mObjGrpLife.size());
}
