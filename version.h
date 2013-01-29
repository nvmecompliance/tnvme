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

/*
 * Specify the software release version numbers on their own line for use with
 * awk and the creation of RPM's while also being compatible with building
 * the binaries via the Makefile with *.cpp source code.
 * If the line numbers within this file change by the result of editing, then
 * you must modify both the Makefile and build.sh for awk parsing. Additionally
 * test this modification by running the Makefile rpm target.
 */

#define VER_MAJOR	\
2

#define VER_MINOR	\
12
