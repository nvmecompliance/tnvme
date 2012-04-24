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

#ifndef _FRMWKEX_H_
#define _FRMWKEX_H_

#include <string>

using namespace std;


/**
* This class is the base class for all exceptions thrown from the source
* originating from the compliance suite. Throwing anything not derived from
* this class could cause a segmentation fault as hardware could keep writing
* to resources hdw thought were there after a test decided an error against
* some criteria. Immediately, after throwing, test lifetime objects go out of
* scope and so hdw must be immediately disabled to prevent access to freed
* resources.
*/
class FrmwkEx
{
public:
    FrmwkEx(string filename, int lineNum);
    FrmwkEx(string filename, int lineNum, string &msg);
    FrmwkEx(string filename, int lineNum, const char *fmt, ...);
    virtual ~FrmwkEx();

    string GetMessage() const { return mMsg; }


protected:
    /**
     * Provide child class to place custom processing duties before the
     * DUT is disabled.
     */
    virtual void PreliminaryProcessing();


private:
    FrmwkEx();

    string mMsg;
    static bool mPrelimProcessingInProgress;

    void DumpStateOfTheSystem();
};


#endif
