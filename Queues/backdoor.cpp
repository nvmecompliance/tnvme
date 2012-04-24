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

#include "backdoor.h"
#include "../Exception/frmwkEx.h"


Backdoor::Backdoor()
{
    throw FrmwkEx(HERE, "Illegal constructor");
}


Backdoor::Backdoor(int fd)
{
    mFD = fd;
    if (mFD < 0)
        throw FrmwkEx(HERE, "Object created with a bad FD=%d", fd);
}


Backdoor::~Backdoor()
{
}


void
Backdoor::SetToxicCmdValue(struct backdoor_inject &injectReq)
{
    int ret;

    // This is volatile, see class level header comment.
    if ((ret = ioctl(mFD, NVME_IOCTL_TOXIC_64B_DWORD, &injectReq)) < 0)
        throw FrmwkEx(HERE, "Backdoor toxic injection failed: 0x%02X", ret);
}
