#ifndef _TNVMEHELPERS_H_
#define _TNVMEHELPERS_H_

#include "group.h"


bool ParseTargetCmdLine(TestTarget &target, const char *optarg);
bool ParseSkipTestCmdLine(vector<TestRef> &skipTest, const char *optarg);
bool ParseRmmapCmdLine(RmmapIo &rmmap, const char *optarg);
bool ParseWmmapCmdLine(WmmapIo &wmmap, const char *optarg);
bool ExecuteTests(struct CmdLine &cl, vector<Group *> &groups);


#endif
