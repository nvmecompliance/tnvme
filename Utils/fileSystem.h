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

#ifndef _FILESYSTEM_H_
#define _FILESYSTEM_H_

#include <limits.h>
#include "tnvme.h"


/**
 * Enforce type checking to ensure dumps end up the proper location. Use
 * method FileSystem::PrepDumpFile() to create this name
 */
typedef string                      DumpFilename;


/**
* This class is meant not be instantiated because it should only ever contain
* static members. These utility functions can be viewed as wrappers to
* perform common, repetitious tasks which avoids coping the same chunks of
* code throughout the framework.
*
* @note This class may throw exceptions, please see comment within specific
*       methods.
*/
class FileSystem
{
public:
    FileSystem();
    virtual ~FileSystem();

    /**
     * Initializes the file system to use the specific root dump directory by
     * verifying it exists. It must exists and will not be created. Under the
     * root dump directory 2 base dump directories are setup and cleaned. The
     * default base dump directory will be SetBaseDumpDir(true). The dump
     * directory structure is as follows:
     *     1) \<root_dump\>/Informative or
     *     2) \<root_dump\>/GrpPending
     * @note This method will not throw
     * @param dir Pass the name of the root dump directory to certify
     * @return true if successful, otherwise false;
     */
    static bool SetRootDumpDir(string dir);

    /**
     * Sets 1 of 2 possible base dump directories. GrpInformative gets its
     * own dump directory to remember the constant data which is always
     * extracted before all test(s) execute. Selects one of:
     *     1) \<root_dump\>/Informative or
     *     2) \<root_dump\>/GrpPending
     * @note This method will not throw
     * @param useDirInfo Pass true for mDumpDirInfo, false for mDumpDirPending
     */
    static void SetBaseDumpDir(bool useDirInfo) { mUseDirInfo = useDirInfo; }

    /**
     * Cleans all files from the base dump directory. Each new group which
     * executes should start dumping to an empty directory. This approach keeps
     * only the last group's dumps and attempts to prevent the file system from
     * breaching a maximum limit.
     * @note This method will not throw
     * @return true if successful, otherwise false;
     */
    static bool CleanDumpDir();

    /**
     * All the files from the base dump directory will be rotated such that
     * filenames of the format "*.prev" will be deleted, and all other files
     * currently within will be renamed to "*.prev".
     * @note This method will not throw
     * @return true if successful, otherwise false;
     */
    static bool RotateDumpDir();

    /**
     * Creates a filename from the parameters within the base dump directory.
     * @note This method may throw
     * @param grpName Pass the name of the group, i.e. Test::mGrpName, required
     * @param className Pass the test cast class name, required
     * @param objName Pass the name of the object being dumped, required
     * @param qualifier (optional) Pass any extra qualifications
     * @return A properly formated filename for dumping purposes
     */
    static DumpFilename PrepDumpFile(string grpName, string className,
        string objName, string qualifier = "");


private:
    /// true uses mDumpDirGrpInfo; false uses mDumpDirPending
    static bool mUseDirInfo;
    static string mDumpDirInfo;
    static string mDumpDirPending;
};


#endif
