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

#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "dumpCtrlrAddrSpace_r10b.h"
#include "grpInformative.h"
#include "globals.h"
#include "../Utils/fileSystem.h"


DumpCtrlrAddrSpace_r10b::DumpCtrlrAddrSpace_r10b(int fd, string grpName,
    string testName) :
    Test(fd, grpName, testName, SPECREV_10b)
{
    // 66 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 3");
    mTestDesc.SetShort(     "Dump all controller address space registers");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Dumps the values of every controller address space register, offset "
        "from PCI BAR0/BAR1 address, to a file");
}


DumpCtrlrAddrSpace_r10b::~DumpCtrlrAddrSpace_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


DumpCtrlrAddrSpace_r10b::
DumpCtrlrAddrSpace_r10b(const DumpCtrlrAddrSpace_r10b &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


DumpCtrlrAddrSpace_r10b &
DumpCtrlrAddrSpace_r10b::operator=(const DumpCtrlrAddrSpace_r10b &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


bool
DumpCtrlrAddrSpace_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) none
     *  \endverbatim
     */

    int fd;
    string work;
    uint64_t value = 0;
    string outFile;
    const CtlSpcType *pciMetrics = gRegisters->GetCtlMetrics();


    // Dumping all register values to well known file
    outFile = FileSystem::PrepLogFile(mGrpName, mTestName, "ctrl", "registers");
    if ((fd = open(outFile.c_str(), FILENAME_FLAGS, FILENAME_MODE)) == -1) {
        LOG_ERR("file=%s: %s", outFile.c_str(), strerror(errno));
        throw exception();
    }

    // Read all registers in ctrlr space
    for (int i = 0; i < CTLSPC_FENCE; i++) {
        if (pciMetrics[i].specRev != mSpecRev)
            continue;

        if (pciMetrics[i].size > MAX_SUPPORTED_REG_SIZE) {
            uint8_t *buffer;
            buffer = new uint8_t[pciMetrics[i].size];
            if (gRegisters->Read(NVMEIO_BAR01, pciMetrics[i].size,
                pciMetrics[i].offset, buffer) == false) {
                goto ERROR_OUT;
            } else {
                string work = "  ";
                work += gRegisters->FormatRegister(NVMEIO_BAR01,
                    pciMetrics[i].size, pciMetrics[i].offset, buffer);
                work += "\n";
                write(fd, work.c_str(), work.size());
            }
            delete [] buffer;
        } else if (pciMetrics[i].size > MAX_SUPPORTED_REG_SIZE) {
            continue;   // Don't care about really large areas, their reserved
        } else if (gRegisters->Read((CtlSpc)i, value) == false) {
            break;
        } else {
            work = "  ";    // indent reg values within each capability
            work += gRegisters->FormatRegister(pciMetrics[i].size,
                pciMetrics[i].desc, value);
            work += "\n";
            write(fd, work.c_str(), work.size());
        }
    }

    close(fd);
    return true;

ERROR_OUT:
    close(fd);
    throw exception();
}


