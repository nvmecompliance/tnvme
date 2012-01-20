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

#include "tnvme.h"
#include "test.h"
#include "globals.h"


Test::Test(int fd, string grpName, string testName, SpecRev specRev,
    ErrorRegs errRegs)
{
    mFd = fd;
    if (mFd < 0)
        LOG_DBG("Object created with a bad fd=%d", fd);

    mSpecRev = specRev;
    mGrpName = grpName;
    mTestName = testName;
    mErrRegs = errRegs;
}


Test::~Test()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


Test::Test(const Test &other) :
    mFd(other.mFd), mSpecRev(other.mSpecRev), mTestDesc(other.mTestDesc),
    mGrpName(other.mGrpName), mTestName(other.mTestName)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


Test &
Test::operator=(const Test &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    mFd = other.mFd;
    mSpecRev = other.mSpecRev;
    mTestDesc = other.mTestDesc;
    mGrpName = other.mGrpName;
    mTestName = other.mTestName;
    return *this;
}


bool
Test::Run()
{
    try {
        ResetStatusRegErrors();
        if (RunCoreTest()) {
            if (GetStatusRegErrors()) {
                LOG_NRM("SUCCESSFUL test case run");
                return true;
            }
        }
    } catch (...) {
        ;   // Don't let exceptions propagate, converting to boolean error
    }

    LOG_NRM("FAILED test case run");
    return false;
}


void
Test::ResetStatusRegErrors()
{
    const vector<PciCapabilities> *cap = gRegisters->GetPciCapabilities();

    // The following algo is taking advantage of the fact that writing
    // RO register bits have no effect, but will have effect on RWC bits
    LOG_NRM("Resetting sticky PCI errors");
    gRegisters->Write(PCISPC_STS, 0xffff);

    for (uint16_t i = 0; i < cap->size(); i++) {
        if (cap->at(i) == PCICAP_PXCAP)
            gRegisters->Write(PCISPC_PXDS, 0xffff);
        else if (cap->at(i) == PCICAP_AERCAP)
            gRegisters->Write(PCISPC_AERUCES, 0xffffffff);
    }
}


bool
Test::GetStatusRegErrors()
{
    uint64_t value = 0;
    uint64_t expectedValue = 0;
    const PciSpcType *pciMetrics = gRegisters->GetPciMetrics();
    const CtlSpcType *ctlMetrics = gRegisters->GetCtlMetrics();
    const vector<PciCapabilities> *cap = gRegisters->GetPciCapabilities();


    // PCI STS register may indicate some error
    if (gRegisters->Read(PCISPC_STS, value) == false)
        return false;
    expectedValue = (value & ~((uint64_t)mErrRegs.sts));
    if (value != expectedValue) {
        LOG_ERR("%s error bit #%d indicates test failure",
            pciMetrics[PCISPC_STS].desc,
            ReportOffendingBitPos(value, expectedValue));
        return false;
    }


    // Other optional PCI errors
    for (uint16_t i = 0; i < cap->size(); i++) {
        if (cap->at(i) == PCICAP_PXCAP) {
            if (gRegisters->Read(PCISPC_PXDS, value) == false)
                return false;
            expectedValue = (value & ~((uint64_t)mErrRegs.pxds));
            if (value != expectedValue) {
                LOG_ERR("%s error bit #%d indicates test failure",
                    pciMetrics[PCISPC_PXDS].desc,
                    ReportOffendingBitPos(value, expectedValue));
                return false;
            }
        } else if (cap->at(i) == PCICAP_AERCAP) {
            if (gRegisters->Read(PCISPC_AERUCES, value) == false)
                return false;
            expectedValue = (value & ~((uint64_t)mErrRegs.aeruces));
            if (value != expectedValue) {
                LOG_ERR("%s error bit #%d indicates test failure",
                    pciMetrics[PCISPC_PXDS].desc,
                    ReportOffendingBitPos(value, expectedValue));
                return false;
            }
        }
    }


    // Ctrl'r STS register may indicate some error
    if (gRegisters->Read(CTLSPC_CSTS, value) == false)
        return false;
    expectedValue = (value & ~((uint64_t)mErrRegs.csts));
    if (value != expectedValue) {
        LOG_ERR("%s error bit #%d indicates test failure",
            ctlMetrics[CTLSPC_CSTS].desc,
            ReportOffendingBitPos(value, expectedValue));
        return false;
    }

    return true;
}


int
Test::ReportOffendingBitPos(uint64_t val, uint64_t expectedVal)
{
    uint64_t bitMask;

    for (int i = 0; i < (int)(sizeof(uint64_t)*8); i++) {
        bitMask = (1 << i);
        if ((val & bitMask) != (expectedVal & bitMask))
            return i;
    }
    return INT_MAX; // there is no mismatch
}


bool
Test::RunCoreTest()
{
    LOG_ERR("Children must over ride to provide functionality");
    return false;
}

