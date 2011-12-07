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

using namespace std;


FileSystem::FileSystem()
{
}


FileSystem::~FileSystem()
{
}


void
FileSystem::AssureDirectoryExists(string dir)
{
    try {
        if (boost::filesystem::exists(dir.c_str())) {

            if (boost::filesystem::is_directory(dir.c_str()) == false) {
                boost::filesystem::remove_all(dir.c_str());
                boost::filesystem::create_directory(dir.c_str());
            }
        } else {
            boost::filesystem::create_directory(dir.c_str());
        }
    } catch (...) {
        LOG_DBG("boost::filesystem exception");
        throw exception();
    }
}


string
FileSystem::PrepLogFile(string grpName, string className, string objName,
    string qualifier)
{
    string file;

    if (grpName.empty() || className.empty() || objName.empty()) {
        LOG_DBG("Mandatory params are empty");
        throw exception();
    }
    file += BASE_LOG_DIR;
    file += grpName + "/";
    file += className + ".";
    file += objName;
    if (qualifier.empty() == false)
        file += "." + qualifier;

    // All log file go into a specific sub-folder which is the name of the group
    AssureDirectoryExists(BASE_LOG_DIR + grpName);
    return file;
}
