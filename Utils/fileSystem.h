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
 * Enforce type checking to ensure logs end up the proper location. Use
 * method FileSystem::PrepLogFile() to create this name
 */
typedef string                      LogFilename;


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
     * Initializes the file system to use the specific root logging directory by
     * verifying it exists. It must exists and will not be created. Under the
     * root log directory 2 base logging directories are setup and cleaned. The
     * default base log directory will be SetBaseLogDir(true). The logging
     * directory structure is as follows:
     *     1) root_log_dir/GrpInformative or
     *     2) root_log_dir/GrpPending
     * @note This method will not throw
     * @param dir Pass the name of the root log directory to certify
     * @return true if successful, otherwise false;
     */
    static bool SetRootLogDir(string dir);

    /**
     * Sets 1 of 2 possible base logging directories. GrpInformative gets its
     * own logging directory to remember the constant data which is always
     * extracted before all test(s) execute. Selects one of:
     *     1) root_log_dir/GrpInformative or
     *     2) root_log_dir/GrpPending
     * @note This method will not throw
     * @param useGrpInfo Pass true for mLogDirPending, false for mLogDirPending
     */
    static void SetBaseLogDir(bool useGrpInfo) { mUseGrpInfo = useGrpInfo; }

    /**
     * Cleans all files from the base logging directory. Each new group which
     * executes should start logging to an empty directory. This approach keeps
     * only the last group's logs and attempts to prevent the file system from
     * breaching a maximum limit.
     * @note This method will not throw
     * @return true if successful, otherwise false;
     */
    static bool PrepLogDir();

    /**
     * Creates a filename from the parameters within the base logging directory.
     * @note This method may throw
     * @param grpName Pass the name of the group, i.e. Test::mGrpName, required
     * @param className Pass the test cast class name, required
     * @param objName Pass the name of the object being dumped, required
     * @param qualifier (optional) Pass any extra qualifications
     * @return A properly formated filename for logging purposes
     */
    static LogFilename PrepLogFile(string grpName, string className,
        string objName, string qualifier = "");


private:
    /// true uses mLogDirGrpInfo; false uses mLogDirPending
    static bool mUseGrpInfo;
    static string mLogDirPending;
    static string mLogDirGrpInfo;
};


#endif
