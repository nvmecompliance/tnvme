#ifndef _TNVMEHELPERS_H_
#define _TNVMEHELPERS_H_

#include "group.h"


bool ParseTargetCmdLine(TestTarget &target, const char *optarg);
bool ParseSkipTestCmdLine(vector<TestRef> &skipTest, const char *optarg);
bool ParseMmapCmdLine(MmapIo &mmap, const char *optarg);
bool ExecuteTests(struct CmdLine &cl, vector<Group *> &groups);
void ResetStickyErrors();


#endif
