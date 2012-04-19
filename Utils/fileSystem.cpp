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

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <boost/filesystem/operations.hpp>
#include "fileSystem.h"
#include "../Exception/frmwkEx.h"

using namespace std;

bool FileSystem::mUseGrpInfo = true;
string FileSystem::mDumpDirPending;
string FileSystem::mDumpDirGrpInfo;


FileSystem::FileSystem()
{
}


FileSystem::~FileSystem()
{
}


bool
FileSystem::SetRootDumpDir(string dir)
{
    char work[256];

    mDumpDirPending = (dir + "/GrpPending/");
    mDumpDirGrpInfo = (dir + "/GrpInformative/");
    try {
        if (boost::filesystem::exists(dir.c_str())) {
            SetBaseDumpDir(false);
            snprintf(work, sizeof(work), "mkdir -p -m 777 %s",
                mDumpDirPending.c_str());
            system(work);
            if (PrepDumpDir() == false)
                return false;

            SetBaseDumpDir(true);    // this is the default
            snprintf(work, sizeof(work), "mkdir -p -m 777 %s",
                mDumpDirGrpInfo.c_str());
            system(work);
            if (PrepDumpDir() == false)
                return false;

            return true;
        } else {
            LOG_ERR("Root dump directory is missing: %s", dir.c_str());
            return false;
        }
    } catch (exception &exc) {
        LOG_ERR("boost::filesystem exception");
        return false;
    }
}


bool
FileSystem::PrepDumpDir()
{
    char work[256];
    string dumpDir = (mUseGrpInfo) ? mDumpDirGrpInfo : mDumpDirPending;


    if (dumpDir.empty()) {
        LOG_ERR("cmd line option --dump=<empty> is dangerous, not allowing");
        return false;
    }
    snprintf(work, sizeof(work), "rm -rf %s*", dumpDir.c_str());
    system(work);
    if (boost::filesystem::exists(work)) {
        LOG_ERR("Unable to remove files within: %s", dumpDir.c_str());
        return false;
    }
    return true;
}


string
FileSystem::PrepDumpFile(string grpName, string className, string objName,
    string qualifier)
{
    string file;

    if (grpName.empty() || className.empty() || objName.empty())
        throw FrmwkEx("Mandatory params are empty");

    file += (mUseGrpInfo) ? mDumpDirGrpInfo : mDumpDirPending;
    file += grpName + ".";
    file += className + ".";
    file += objName;
    if (qualifier.empty() == false)
        file += "." + qualifier;
    return file;
}
