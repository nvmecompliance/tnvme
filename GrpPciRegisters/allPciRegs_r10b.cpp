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

#include "allPciRegs_r10b.h"
#include "globals.h"


AllPciRegs_r10b::AllPciRegs_r10b(int fd, string grpName, string testName) :
    Test(fd, grpName, testName, SPECREV_10b)
{
    // 66 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 3");
    mTestDesc.SetShort(     "Validate all PCI registers syntactically");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Validates the following; the RO fields which are not implementation "
        "specific contain default values; The RO fields cannot be written; All "
        "ASCII fields only contain chars 0x20 to 0x7e.");
}


AllPciRegs_r10b::~AllPciRegs_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


AllPciRegs_r10b::
AllPciRegs_r10b(const AllPciRegs_r10b &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


AllPciRegs_r10b &
AllPciRegs_r10b::operator=(const AllPciRegs_r10b &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


bool
AllPciRegs_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) none
     *  \endverbatim
     */

    ValidateDefaultValues();
    ValidateROBitsAfterWriting();
    return true;
}


void
AllPciRegs_r10b::ValidateDefaultValues()
{
    const PciSpcType *pciMetrics = gRegisters->GetPciMetrics();
    const vector<PciCapabilities> *pciCap = gRegisters->GetPciCapabilities();

    LOG_NRM("Validating default register values");

    // Traverse the PCI header registers
    for (int j = 0; j < PCISPC_FENCE; j++) {
        if (pciMetrics[j].specRev != mSpecRev)
            continue;

        // PCI hdr registers don't have an assoc capability
        if (pciMetrics[j].cap == PCICAP_FENCE)
            ValidatePciHdrRegisterROAttribute((PciSpc)j);
    }

    // Traverse all discovered capabilities
    for (size_t i = 0; i < pciCap->size(); i++) {
        // Read all registers assoc with the discovered capability
        for (int j = 0; j < PCISPC_FENCE; j++) {
            if (pciMetrics[j].specRev != SPECREV_10b)
                continue;
            else if (pciCap->at(i) == pciMetrics[j].cap)
                ValidatePciCapRegisterROAttribute((PciSpc)j);
        }
    }
}


void
AllPciRegs_r10b::ValidateROBitsAfterWriting()
{
    uint64_t value;
    uint64_t origValue;
    const PciSpcType *pciMetrics = gRegisters->GetPciMetrics();
    const vector<PciCapabilities> *pciCap = gRegisters->GetPciCapabilities();

    LOG_NRM("Validating RO bits after writing");

    // Traverse the PCI header registers
    for (int j = 0; j < PCISPC_FENCE; j++) {
        if (pciMetrics[j].specRev != mSpecRev)
            continue;

        // Reserved areas at NOT suppose to be written
        if ((j == PCISPC_RES0) || (j == PCISPC_RES1))
            continue;

        // PCI hdr registers don't have an assoc capability
        if (pciMetrics[j].cap == PCICAP_FENCE) {
            LOG_NRM("Validate RO attribute after trying to write 1");
            if (gRegisters->Read((PciSpc)j, origValue) == false)
                throw exception();
            value = (origValue | pciMetrics[j].maskRO);
            if (gRegisters->Write((PciSpc)j, value) == false)
                throw exception();
            ValidatePciHdrRegisterROAttribute((PciSpc)j);

            LOG_NRM("Validate RO attribute after trying to write 0");
            value = (origValue & ~pciMetrics[j].maskRO);
            if (gRegisters->Write((PciSpc)j, value) == false)
                throw exception();
            ValidatePciHdrRegisterROAttribute((PciSpc)j);
        }
    }

    // Traverse all discovered capabilities
    for (size_t i = 0; i < pciCap->size(); i++) {
        // Read all registers assoc with the discovered capability
        for (int j = 0; j < PCISPC_FENCE; j++) {
            if (pciMetrics[j].specRev != mSpecRev)
                continue;

            // Reserved areas at NOT suppose to be written
            if ((j == PCISPC_RES0) || (j == PCISPC_RES1))
                continue;

            if (pciCap->at(i) == pciMetrics[j].cap) {
                LOG_NRM("Validate RO attribute after trying to write 1");
                if (gRegisters->Read((PciSpc)j, origValue) == false)
                    throw exception();
                value = (origValue | pciMetrics[j].maskRO);
                if (gRegisters->Write((PciSpc)j, value) == false)
                    throw exception();
                ValidatePciCapRegisterROAttribute((PciSpc)j);

                LOG_NRM("Validate RO attribute after trying to write 0");
                value = (origValue & ~pciMetrics[j].maskRO);
                if (gRegisters->Write((PciSpc)j, value) == false)
                    throw exception();
                ValidatePciCapRegisterROAttribute((PciSpc)j);
            }
        }
    }
}


int
AllPciRegs_r10b::ReportOffendingBitPos(uint64_t val, uint64_t expectedVal)
{
    uint64_t bitMask;

    for (int i = 0; i < (int)(sizeof(uint64_t)*8); i++) {
        bitMask = (1 << i);
        if ((val & bitMask) != (expectedVal & bitMask))
            return i;
    }
    return INT_MAX; // there is no mismatch
}


void
AllPciRegs_r10b::ValidatePciCapRegisterROAttribute(PciSpc reg)
{
    uint64_t value;
    uint64_t expectedValue;
    const PciSpcType *pciMetrics = gRegisters->GetPciMetrics();


    if (pciMetrics[reg].size > MAX_SUPPORTED_REG_SIZE) {
        for (uint16_t k = 0; (k*DWORD_LEN) < pciMetrics[reg].size; k++) {

            if (gRegisters->Read(NVMEIO_PCI_HDR, DWORD_LEN,
                pciMetrics[reg].offset + (k * DWORD_LEN),
                (uint8_t *)&value) == false) {

                throw exception();
            } else {
                // Ignore the implementation specific bits, and bits that
                // the manufacturer can make a decision as to their type of
                // access RW,RO
                value &= ~pciMetrics[reg].impSpec;

                // Verify that the RO bits are set to correct default
                // values, no reset needed to achieve this because there's
                // no way to change.
                value &= pciMetrics[reg].maskRO;
                expectedValue = (pciMetrics[reg].dfltValue &
                    pciMetrics[reg].maskRO);

                if (value != expectedValue) {
                    LOG_ERR("%s RO bit #%d has incorrect value",
                        pciMetrics[reg].desc,
                        ReportOffendingBitPos(value, expectedValue));
                    throw exception();
                }
            }
        }
    } else if (gRegisters->Read(reg, value) == false) {
        throw exception();
    } else {
        // Ignore the implementation specific bits, and bits that
        // the manufacturer can make a decision as to their type of
        // access RW,RO
        value &= ~pciMetrics[reg].impSpec;

        // Verify that the RO bits are set to correct default
        // values, no reset needed to achieve this because there's
        // no way to change.
        value &= pciMetrics[reg].maskRO;
        expectedValue = (pciMetrics[reg].dfltValue &
            pciMetrics[reg].maskRO);

        if (value != expectedValue) {
            LOG_ERR("%s RO bit #%d has incorrect value", pciMetrics[reg].desc,
                ReportOffendingBitPos(value, expectedValue));
            throw exception();
        }
    }
}


void
AllPciRegs_r10b::ValidatePciHdrRegisterROAttribute(PciSpc reg)
{
    uint64_t value;
    uint64_t expectedValue;
    const PciSpcType *pciMetrics = gRegisters->GetPciMetrics();

    if (pciMetrics[reg].size > MAX_SUPPORTED_REG_SIZE) {
        for (int k = 0; (k*sizeof(value)) < pciMetrics[reg].size; k++) {

            if (gRegisters->Read(NVMEIO_PCI_HDR, sizeof(value),
                pciMetrics[reg].offset + (k * sizeof(value)),
                (uint8_t *)&value) == false) {

                throw exception();
            } else {
                // Ignore the implementation specific bits, and bits that the
                // manufacturer can make a decision as to their type of access
                value &= ~pciMetrics[reg].impSpec;

                // Verify that the RO bits are set to correct default values, no
                // reset needed to achieve this because there's no way to change.
                value &= pciMetrics[reg].maskRO;
                expectedValue =
                    (pciMetrics[reg].dfltValue & pciMetrics[reg].maskRO);

                // Take care of special cases 1st
                if (reg == PCISPC_BAR2) {
                    // Learn the behavior of this register, supported or not?
                    uint64_t cmdReg;
                    if (gRegisters->Read(PCISPC_CMD, cmdReg) == false)
                        throw exception();

                    if (cmdReg & CMD_IOSE) {
                        if (value != expectedValue) {
                            LOG_ERR("%s RO bit #%d has incorrect value",
                                pciMetrics[reg].desc,
                                ReportOffendingBitPos(value, expectedValue));
                            throw exception();
                        }
                    } else {  // Optional index/Data pair register not supported
                        expectedValue = 0;
                        if (value != expectedValue) {
                            LOG_ERR("%s RO bit #%d has incorrect value",
                                pciMetrics[reg].desc,
                                ReportOffendingBitPos(value, expectedValue));
                            throw exception();
                        }
                    }
                } else {    // generically handled all other registers
                    if (value != expectedValue) {
                        LOG_ERR("%s RO bit #%d has incorrect value",
                            pciMetrics[reg].desc,
                            ReportOffendingBitPos(value, expectedValue));
                        throw exception();
                    }
                }
            }
        }
    } else if (gRegisters->Read(reg, value) == false) {
        throw exception();
    } else {
        // Ignore the implementation specific bits, and bits that the
        // manufacturer can make a decision as to their type of access RW/RO
        value &= ~pciMetrics[reg].impSpec;

        // Verify that the RO bits are set to correct default values, no
        // reset needed to achieve this because there's no way to change.
        value &= pciMetrics[reg].maskRO;
        expectedValue = (pciMetrics[reg].dfltValue & pciMetrics[reg].maskRO);

        // Take care of special cases 1st
        if (reg == PCISPC_BAR2) {
            // Learn the behavior of this register, supported or not?
            uint64_t cmdReg;
            if (gRegisters->Read(PCISPC_CMD, cmdReg) == false)
                throw exception();

            if (cmdReg & CMD_IOSE) {
                if (value != expectedValue) {
                    LOG_ERR("%s RO bit #%d has incorrect value",
                        pciMetrics[reg].desc,
                        ReportOffendingBitPos(value, expectedValue));
                    throw exception();
                }
            } else {    // Optional index/Data pair register not supported
                expectedValue = 0;
                if (value != expectedValue) {
                    LOG_ERR("%s RO bit #%d has incorrect value",
                        pciMetrics[reg].desc,
                        ReportOffendingBitPos(value, expectedValue));
                    throw exception();
                }
            }
        } else {    // generically handled all other registers
            if (value != expectedValue) {
                LOG_ERR("%s RO bit #%d has incorrect value",
                    pciMetrics[reg].desc,
                    ReportOffendingBitPos(value, expectedValue));
                throw exception();
            }
        }
    }
}


