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

#ifndef _TNVMEHELPERS_H_
#define _TNVMEHELPERS_H_

#include "group.h"


bool ParseTargetCmdLine(TestTarget &target, const char *optarg);
bool ParseSkipTestCmdLine(vector<TestRef> &skipTest, const char *optarg);
bool ParseRmmapCmdLine(RmmapIo &rmmap, const char *optarg);
bool ParseWmmapCmdLine(WmmapIo &wmmap, const char *optarg);
bool ExecuteTests(struct CmdLine &cl, vector<Group *> &groups);
bool ParseQueuesCmdLine(Queues &queues, const char *optarg);
bool ParseErrorCmdLine(ErrorRegs &errRegs, const char *optarg);
bool SetFeaturesNumberOfQueues(Queues &queues, int fd);


#endif
