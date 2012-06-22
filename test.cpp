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
#include "./Utils/kernelAPI.h"


Test::Test(string grpName, string testName, SpecRev specRev)
{
    mSpecRev = specRev;
    mGrpName = grpName;
    mTestName = testName;
}


Test::~Test()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


Test::Test(const Test &other) :
    mSpecRev(other.mSpecRev), mGrpName(other.mGrpName),
    mTestName(other.mTestName), mTestDesc(other.mTestDesc)

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
    mSpecRev = other.mSpecRev;
    mGrpName = other.mGrpName;
    mTestName = other.mTestName;
    mTestDesc = other.mTestDesc;
    return *this;
}


bool
Test::Run()
{
    try {
        ResetStatusRegErrors();
        KernelAPI::DumpKernelMetrics(FileSystem::PrepDumpFile(mGrpName,
            mTestName, "kmetrics", "preTestRun"));

        RunCoreTest();  // Throws upon errors, returns upon success

        // What do the PCI registers say about errors that may have occurred?
        if (GetStatusRegErrors() == false)
            return false;
    } catch (FrmwkEx &ex) {
        return false;
    } catch (...) {
        // If this exception is thrown from some library which tnvme links
        // with then there is nothing that can be done about this. However,
        // If the source of this exception is source within the compliance
        // suite, then remove it, see class note in file Exception/frmwkEx.h
        LOG_ERR("******************************************************");
        LOG_ERR("******************************************************");
        LOG_ERR("*  Unsupp'd exception, replace with \"throw FrmwkEx\"  *");
        LOG_ERR("*     see class note in file Exception/frmwkEx.h     *");
        LOG_ERR("******************************************************");
        LOG_ERR("******************************************************");
        return false;
    }
    return true;
}


Test::RunType
Test::Runnable(bool preserve)
{
    try {
        return RunnableCoreTest(preserve);  // Throws upon errors
    } catch (FrmwkEx &ex) {
        return RUN_FAIL;
    } catch (...) {
        // If this exception is thrown from some library which tnvme links
        // with then there is nothing that can be done about this. However,
        // If the source of this exception is source within the compliance
        // suite, then remove it, see class note in file Exception/frmwkEx.h
        LOG_ERR("******************************************************");
        LOG_ERR("******************************************************");
        LOG_ERR("*  Unsupp'd exception, replace with \"throw FrmwkEx\"  *");
        LOG_ERR("*     see class note in file Exception/frmwkEx.h     *");
        LOG_ERR("******************************************************");
        LOG_ERR("******************************************************");
        return RUN_FAIL;
    }
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
    expectedValue = (value & ~((uint64_t)gCmdLine.errRegs.sts));
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
            expectedValue = (value & ~((uint64_t)gCmdLine.errRegs.pxds));
            if (value != expectedValue) {
                LOG_ERR("%s error bit #%d indicates test failure",
                    pciMetrics[PCISPC_PXDS].desc,
                    ReportOffendingBitPos(value, expectedValue));
                return false;
            }
        } else if (cap->at(i) == PCICAP_AERCAP) {
            if (gRegisters->Read(PCISPC_AERUCES, value) == false)
                return false;
            expectedValue = (value & ~((uint64_t)gCmdLine.errRegs.aeruces));
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
    expectedValue = (value & ~((uint64_t)gCmdLine.errRegs.csts));
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
        if ((val & bitMask) != (expectedVal & bitMask)) {
            LOG_NRM("Reg val(0x%016lX) expect val(0x%016lX)", val, expectedVal);
            return i;
        }
    }
    return INT_MAX; // there is no mismatch
}


void
Test::RunCoreTest()
{
    throw FrmwkEx(HERE, "Children must over ride to provide functionality");
}


Test::RunType
Test::RunnableCoreTest(bool preserve)
{
    preserve = preserve;    // Suppress compiler error/warning
    throw FrmwkEx(HERE, "Children must over ride to provide functionality");
}

