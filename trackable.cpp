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
    LOG_DBG_DEEP("Destroying trackable obj: %s", GetObjName(mObjType).c_str());
}


string
Trackable::GetObjName(ObjType obj)
{
    string name;

    switch (obj) {
    case OBJ_MEMBUFFER:     name = "MemBuffer";          break;
    case OBJ_ACQ:           name = "ACQ";                break;
    case OBJ_ASQ:           name = "ASQ";                break;
    case OBJ_IOCQ:          name = "IOCQ";               break;
    case OBJ_IOSQ:          name = "IOSQ";               break;
    case OBJ_IDENTIFY:      name = "Identify";           break;
    case OBJ_CREATEIOCQ:    name = "CreateIOCQ";         break;
    case OBJ_CREATEIOSQ:    name = "CreateIOSQ";         break;
    case OBJ_DELETEIOCQ:    name = "DeleteIOCQ";         break;
    case OBJ_DELETEIOSQ:    name = "DeleteIOSQ";         break;
    case OBJ_GETFEATURES:   name = "GetFeatures";        break;
    case OBJ_SETFEATURES:   name = "SetFeatures";        break;
    case OBJ_GETLOGPAGE:    name = "GetLogPage";         break;
    case OBJ_WRITE:         name = "Write";              break;
    case OBJ_READ:          name = "Read";               break;
    default:
        LOG_ERR("Forgot to label this unknown obj");
        throw exception();
    }
    return name;
}
