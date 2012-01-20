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

#define BASE_LOG_DIR                "./Logs/"

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
     * Certifies that a directory is located in the current working directory,
     * otherwise it is created.
     * @note This method will not throw
     * @param dir Pass the name of the directory to certify
     */
    static void AssureDirectoryExists(string dir);

    /**
     * Creates a filename from the parameters, and assures the
     * BASE_LOG_DIR/grpName directory exists to contain the file.
     * @note This method may throw
     * @param grpName Pass the name of the group, i.e. Test::mGrpName, required
     * @param className Pass the test cast class name, required
     * @param objName Pass the name of the object being dumped, required
     * @param qualifier (optional) Pass any extra qualifications
     * @return A properly formated filename for logging purposes
     */
    static LogFilename PrepLogFile(string grpName, string className,
        string objName, string qualifier = "");
};


#endif
