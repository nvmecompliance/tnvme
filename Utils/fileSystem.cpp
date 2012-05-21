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
#include <boost/filesystem.hpp>
#include "fileSystem.h"
#include "../Exception/frmwkEx.h"

#define BASE_NAME_DIR_INFO      "/Informative/"
#define BASE_NAME_PENDING       "/GrpPending/"

using namespace std;

bool FileSystem::mUseDirInfo = true;
string FileSystem::mDumpDirInfo;
string FileSystem::mDumpDirPending;


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

    mDumpDirPending = (dir + BASE_NAME_PENDING);
    mDumpDirInfo = (dir + BASE_NAME_DIR_INFO);

    try {
        if (boost::filesystem::exists(dir.c_str())) {
            SetBaseDumpDir(false);
            snprintf(work, sizeof(work), "mkdir -p -m 777 %s",
                mDumpDirPending.c_str());
            system(work);
            if (CleanDumpDir() == false)
                return false;

            SetBaseDumpDir(true);    // this is the default
            snprintf(work, sizeof(work), "mkdir -p -m 777 %s",
                mDumpDirInfo.c_str());
            system(work);
            if (CleanDumpDir() == false)
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
FileSystem::CleanDumpDir()
{
    char work[256];
    string dumpDir = (mUseDirInfo) ? mDumpDirInfo : mDumpDirPending;

    if (dumpDir.empty()) {
        LOG_ERR("cmd line option --dump=<empty> is destructive, not allowing");
        return false;
    }

    // Remove everything in the dir, not the dir itself
    snprintf(work, sizeof(work), "rm -rf %s*", dumpDir.c_str());
    system(work);
    if (boost::filesystem::exists(work)) {
        LOG_ERR("Unable to remove files within: %s", dumpDir.c_str());
        return false;
    }
    return true;
}


bool
FileSystem::RotateDumpDir()
{
    char work[256];
    string dumpDir = (mUseDirInfo) ? mDumpDirInfo : mDumpDirPending;

    if (dumpDir.empty())
        return true;

    // Remove everything in the dir of the form "*.prev"
    snprintf(work, sizeof(work), "rm %s*.prev", dumpDir.c_str());
    system(work);

    // Get a vector of filenames of all other files within dir
    boost::filesystem::path targDir(dumpDir);
    vector<boost::filesystem::path> allFiles;
    copy(boost::filesystem::directory_iterator(targDir),
        boost::filesystem::directory_iterator(), back_inserter(allFiles));

    // Rename all files to "*.prev"
    for (size_t i = 0; i < allFiles.size(); i++) {
        string newName = (allFiles[i].filename() + ".prev");
        snprintf(work, sizeof(work), "mv %s%s %s%s.prev",
            dumpDir.c_str(), allFiles[i].filename().c_str(),
            dumpDir.c_str(), allFiles[i].filename().c_str());
        system(work);
    }
    return true;
}


string
FileSystem::PrepDumpFile(string grpName, string className, string objName,
    string qualifier)
{
    string file;

    if (grpName.empty() || className.empty() || objName.empty())
        throw FrmwkEx(HERE, "Mandatory params are empty");

    file += (mUseDirInfo) ? mDumpDirInfo : mDumpDirPending;
    file += grpName + ".";
    file += className + ".";
    file += objName;
    if (qualifier.empty() == false)
        file += "." + qualifier;
    return file;
}
