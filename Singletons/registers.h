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

#ifndef _REGISTERS_H_
#define _REGISTERS_H_

#include <string>
#include <vector>
#include "regDefs.h"
#include "dnvme.h"

/**
 * @param regVal Pass the 64 bit register value to mask down to size.
 * @param bytes Pass the number of bytes to keep of the 8 byte regVal
 */
#define REGMASK(regval, bytes)  \
        (regval & (0xffffffffffffffffULL >> (64 - (bytes * 8))))


/**
* This class is meant to interface with PCI and/or ctrl'r registers.
*/
class Registers
{
public:
    /**
     * Enforce singleton design pattern.
     * @param fd Pass the opened file descriptor for the device under test
     * @param specRev Pass which compliance is needed to target
     * @return NULL upon error, otherwise a pointer to the singleton
     */
    static Registers *GetInstance(int fd, SpecRev specRev);
    static void KillInstance();
    ~Registers();

    /**
     * No reason to protect the specification revision being targeted, each
     * instance of tnvme can only ever target a single revision of the spec.
     */
    SpecRev GetSpecRev() { return mSpecRev; }

    /**
     * Read a register value from the appropriate address space.
     * @param reg Pass which register to read.
     * @param value Returns the value read, if and only if successful. The
     *          lowest order nibbles are populated 1st if the register is
     *          smaller than sizeof(value)
     * @param verbose Pass true to log action, false to be silent
     * @return true upon success, otherwise false
     */
    bool Read(PciSpc reg, uint64_t &value, bool verbose = true);
    bool Read(CtlSpc reg, uint64_t &value, bool verbose = true);

    /**
     * Generic read function, you supply ALL the necessary data to read at
     * specified offset, length of register, which address space to read, etc.
     * @param regSpc Pass which register space to read
     * @param rsize Pass the length in bytes of the register
     * @param roffset Pass the offset from start of spec'd address space to
     *        start reading.
     * @param value Returns the value read, if and only if successful.
     * @param verbose Pass true to log action, false to be silent
     * @return true upon success, otherwise false
     */
    bool Read(nvme_io_space regSpc, uint16_t rsize, uint16_t roffset,
        uint8_t *value, bool verbose = true);

    /**
     * Generic read function, you supply ALL the necessary data to read at
     * specified offset, length of register, which address space to read, etc.
     * @param regSpc Pass which register space to read
     * @param rsize Pass the length in bytes of the register
     * @param roffset Pass the offset from start of spec'd address space to
     *        start reading.
     * @param racc Pass the register access width to read.
     * @param value Returns the value read, if and only if successful.
     * @param verbose Pass true to log action, false to be silent
     * @return true upon success, otherwise false
     */
    bool Read(nvme_io_space regSpc, uint16_t rsize, uint16_t roffset,
        nvme_acc_type racc, uint8_t *value, bool verbose = true);

    /**
     * Write a register value to the appropriate address space.
     * @param reg Pass which register to write.
     * @param value Pass the value to write.
     * @param verbose Pass true to log action, false to be silent
     * @return true upon success, otherwise false
     */
    bool Write(PciSpc reg, uint64_t value, bool verbose = true);
    bool Write(CtlSpc reg, uint64_t value, bool verbose = true);

    /**
     * Generic write function, you supply ALL the necessary data to write at
     * specified offset, length of register, which address space to write, etc.
     * @param regSpc Pass which register space to write
     * @param rsize Pass the length in bytes of the register
     * @param roffset Pass the offset from start of spec'd address space to
     *        start writing.
     * @param value Pass the array of value(s) to write, must be of rsize size
     * @param verbose Pass true to log action, false to be silent
     * @return true upon success, otherwise false
     */
    bool Write(nvme_io_space regSpc, uint16_t rsize, uint16_t roffset,
        uint8_t *value, bool verbose = true);

    /**
     * Generic write function, you supply ALL the necessary data to write at
     * specified offset, length of register, which address space to write, etc.
     * @param regSpc Pass which register space to write
     * @param rsize Pass the length in bytes of the register
     * @param roffset Pass the offset from start of spec'd address space to
     *        start writing.
     * @param racc Pass which register access width to write
     * @param value Pass the array of value(s) to write, must be of rsize size
     * @param verbose Pass true to log action, false to be silent
     * @return true upon success, otherwise false
     */
    bool Write(nvme_io_space regSpc, uint16_t rsize, uint16_t roffset,
        nvme_acc_type racc, uint8_t *value, bool verbose = true);

    /**
     * Returns the list of capabilities discovered by parsing PCI address
     * space. This is is ordered in the fashion those capabilities were
     * discovered, i.e. idx=0 is the 1st capability discovered.
     * @return The discovered list of capabilities
     */
    const vector<PciCapabilities> *GetPciCapabilities() { return &mPciCap; }

    /**
     * Returns the known metrics (meta data) for each and every register.
     * Metrics are discovered during class instantiation to learn of the
     * capabilities of the device.
     */
    const PciSpcType *GetPciMetrics() { return mPciSpcMetrics; }
    const CtlSpcType *GetCtlMetrics() { return mCtlSpcMetrics; }

    /**
     * Format, beautify for printing, a register's value based upon its size.
     * @param regSize Pass the number of bytes of the register
     * @param regDesc Pass the description of this register
     * @param regValue Pass the value of the register, the regSize could be
     *          smaller than the uint64_t variable size.
     * @return the formatted resulting string
     */
    string FormatRegister(uint16_t regSize, const char *regDesc,
        uint64_t regValue);
    string FormatRegister(nvme_io_space regSpc, uint16_t rsize,
        uint16_t roffset, uint8_t *value);


private:
    // Implement singleton design pattern
    Registers();
    Registers(int fd, SpecRev specRev);
    static bool mInstanceFlag;
    static Registers *mSingleton;


    /// which spec release is being targeted
    SpecRev mSpecRev;
    /// file descriptor to the device under test
    int mFd;
    /// Keeps track of discovered PCI capabilities
    vector<PciCapabilities> mPciCap;

    /// Contains details about every register residing in PCI space
    static PciSpcType mPciSpcMetrics[];
    /// Contains details about every register residing in ctrlr space
    static CtlSpcType mCtlSpcMetrics[];

    /**
     * The PCI addr space is a bit convoluted in that the capabilities are not
     * at predetermined offsets within PCI addr space like the PCI header regs.
     * Thus we need to traverse the capabilities discovering those offsets.
     */
    void DiscoverPciCapabilities();

    bool Read(nvme_io_space regSpc, uint16_t rsize, uint16_t roffset,
        uint64_t &value, const char *rdesc, bool verbose);
    bool Write(nvme_io_space regSpc, uint16_t rsize, uint16_t roffset,
        uint64_t &value, const char *rdesc, bool verbose);
};


#endif
