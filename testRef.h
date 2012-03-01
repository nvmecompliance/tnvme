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

#ifndef _TESTREF_H_
#define _TESTREF_H_



/**
* This structure describes an individual test case. For details about this
* object see: https://github.com/nvmecompliance/tnvme/wiki/Test-Numbering
* @note This class will not throw exceptions.
*/
struct TestRef {
    size_t  group;
    size_t  xLev;
    size_t  yLev;
    size_t  zLev;

    TestRef() { group = 0; xLev = 0; yLev = 0; zLev = 0; }
    TestRef(size_t g, size_t x, size_t y, size_t z) { Init(g, x, y, z); }

    void Init(size_t g, size_t x, size_t y, size_t z) {
        group = g; xLev = x; yLev = y; zLev = z; }

    bool operator==(const TestRef &other) {
        if ((other.group == group) && (other.xLev == xLev) &&
            (other.yLev == yLev) && (other.zLev == zLev)) {
            return true;
        }
        return false;
    }
};


#endif
