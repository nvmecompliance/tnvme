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

#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include "dnvme.h"
#include "Singletons/registers.h"
#include "Singletons/rsrcMngr.h"
#include "Singletons/ctrlrConfig.h"
#include "Singletons/ctrlrCap.h"
#include "Singletons/informative.h"

// NOTE: To make it easier to decipher objects which are global, prepend 'g'

/// The sole targeted DUT's file descriptor
extern int gDutFd;

/// The appliation's cmd line args
extern struct CmdLine gCmdLine;

/// Tests are encouraged to use this instance for all register access
extern Registers *gRegisters;

/// Tests are encouraged to use this instance to allocate test resources
extern RsrcMngr *gRsrcMngr;

/// Tests are encouraged to use this instance to interface with ctrlr config
extern CtrlrConfig *gCtrlrConfig;

/// Tests are encouraged to use this instance to interface with ctrlr cap
extern CtrlrCap *gCtrlrCap;

/// Tests are encouraged to use this instance to learn common DUT parameters
extern Informative *gInformative;


#endif
