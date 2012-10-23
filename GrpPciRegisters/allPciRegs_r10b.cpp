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

namespace GrpPciRegisters {


AllPciRegs_r10b::AllPciRegs_r10b(
    string grpName, string testName) :
    Test(grpName, testName, SPECREV_10b)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 2");
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


Test::RunType
AllPciRegs_r10b::RunnableCoreTest(bool preserve)
{
    ///////////////////////////////////////////////////////////////////////////
    // All code contained herein must never permanently modify the state or
    // configuration of the DUT. Permanence is defined as state or configuration
    // changes that will not be restored after a cold hard reset.
    ///////////////////////////////////////////////////////////////////////////

    preserve = preserve;    // Suppress compiler error/warning
    return RUN_TRUE;        // This test is never destructive
}


void
AllPciRegs_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) none
     *  \endverbatim
     */
    bool result = true;

    if (gCtrlrConfig->SetState(ST_DISABLE_COMPLETELY) == false)
        throw FrmwkEx(HERE);

    result &= ValidateDefaultValues();
    result &= ValidateROBitsAfterWriting();
    if (!result) 
    	throw FrmwkEx(HERE, "Test failed, check log for specific bit errors.");
}


bool
AllPciRegs_r10b::ValidateDefaultValues()
{
    const PciSpcType *pciMetrics = gRegisters->GetPciMetrics();
    const vector<PciCapabilities> *pciCap = gRegisters->GetPciCapabilities();
    bool result = true;

    LOG_NRM("Validating default register values");

    // Traverse the PCI header registers
    for (int j = 0; j < PCISPC_FENCE; j++) {
        if (pciMetrics[j].specRev != mSpecRev)
            continue;

        // PCI hdr registers don't have an assoc capability
        if (pciMetrics[j].cap == PCICAP_FENCE)
            result &= ValidatePciHdrRegisterROAttribute((PciSpc)j);
    }
    // Traverse all discovered capabilities
    for (size_t i = 0; i < pciCap->size(); i++) {
        // Read all registers assoc with the discovered capability
        for (int j = 0; j < PCISPC_FENCE; j++) {
            if (pciMetrics[j].specRev != SPECREV_10b)
                continue;
            else if (pciCap->at(i) == pciMetrics[j].cap)
                result &= ValidatePciCapRegisterROAttribute((PciSpc)j);
        }
    }
    return result;
}


bool
AllPciRegs_r10b::ValidateROBitsAfterWriting()
{
    uint32_t tmpValue;
    uint64_t value;
    uint64_t origValue;
    const PciSpcType *pciMetrics = gRegisters->GetPciMetrics();
    const vector<PciCapabilities> *pciCap = gRegisters->GetPciCapabilities();
    bool result = true;

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
                throw FrmwkEx(HERE);
            value = (origValue | pciMetrics[j].maskRO);
            if (gRegisters->Write((PciSpc)j, value) == false)
                throw FrmwkEx(HERE);
            result &= ValidatePciHdrRegisterROAttribute((PciSpc)j);

            LOG_NRM("Validate RO attribute after trying to write 0");
            value = (origValue & ~pciMetrics[j].maskRO);
            if (gRegisters->Write((PciSpc)j, value) == false)
                throw FrmwkEx(HERE);
            result &= ValidatePciHdrRegisterROAttribute((PciSpc)j);
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
                if (pciMetrics[j].size > MAX_SUPPORTED_REG_SIZE) {
                    origValue = 0;
                    LOG_NRM("Reading %s",pciMetrics[j].desc);
                    for (uint32_t k = 0; (k*DWORD_LEN) < pciMetrics[j].size;
                        k++) {
                        if (gRegisters->Read(NVMEIO_PCI_HDR, DWORD_LEN,
                            pciMetrics[j].offset + (k * DWORD_LEN),
                            (uint8_t *)&tmpValue) == false) {
                            throw FrmwkEx(HERE);
                        }
                        origValue |= (tmpValue << (k * DWORD_LEN * 8));
                    }
                    LOG_NRM("Value = 0x%016lX",(long unsigned int)origValue);
                } else {
                    if (gRegisters->Read((PciSpc)j, origValue) == false)
                        throw FrmwkEx(HERE);
                }

                value = (origValue | pciMetrics[j].maskRO);
                if (pciMetrics[j].size > MAX_SUPPORTED_REG_SIZE) {
                    tmpValue = 0;
                    LOG_NRM("Writing %s",pciMetrics[j].desc);
                    for (uint32_t k=0; (k*DWORD_LEN) < pciMetrics[j].size;
                        k++) {
                        tmpValue = (value >> ( k*DWORD_LEN*8 )) & 0xFFFFFFFF;
                        if (gRegisters->Write
                            (NVMEIO_PCI_HDR, DWORD_LEN, pciMetrics[j].offset +
                            (k * DWORD_LEN), (uint8_t *)&tmpValue) == false) {
                            throw FrmwkEx(HERE);
                        }
                    }
                    LOG_NRM("Value = 0x%016lX",(long unsigned int)value);
                } else {
                    if (gRegisters->Write((PciSpc)j, value) == false)
                        throw FrmwkEx(HERE);
                }

                result &= ValidatePciCapRegisterROAttribute((PciSpc)j);

                LOG_NRM("Validate RO attribute after trying to write 0");

                if (pciMetrics[j].size > MAX_SUPPORTED_REG_SIZE) {
                    origValue = 0;
                    LOG_NRM("Reading %s",pciMetrics[j].desc);
                    for (uint32_t k = 0; (k*DWORD_LEN) < pciMetrics[j].size;
                        k++) {
                        if (gRegisters->Read
                            (NVMEIO_PCI_HDR, DWORD_LEN, pciMetrics[j].offset +
                            (k * DWORD_LEN), (uint8_t *)&tmpValue) == false) {
                            throw FrmwkEx(HERE);
                        }
                        origValue |= (tmpValue << (k * DWORD_LEN * 8));
                    }
                    LOG_NRM("Value = 0x%016lX",(long unsigned int)origValue);
                } else {
                    if (gRegisters->Read((PciSpc)j, origValue) == false)
                        throw FrmwkEx(HERE);
                }

                value = (origValue & ~pciMetrics[j].maskRO);
                if (pciMetrics[j].size > MAX_SUPPORTED_REG_SIZE) {
                    tmpValue = 0;
                    LOG_NRM("Writing %s",pciMetrics[j].desc);
                    for (uint32_t k=0; (k*DWORD_LEN) < pciMetrics[j].size;
                        k++) {
                        tmpValue = (value >> ( k*DWORD_LEN*8 )) & 0xFFFFFFFF;
                        if (gRegisters->Write(NVMEIO_PCI_HDR, DWORD_LEN,
                            pciMetrics[j].offset + (k * DWORD_LEN),
                            (uint8_t *)&tmpValue) == false) {
                            throw FrmwkEx(HERE);
                        }
                    }
                    LOG_NRM("Value = 0x%016lX",(long unsigned int)value);
                } else {
                    if (gRegisters->Write((PciSpc)j, value) == false)
                        throw FrmwkEx(HERE);
                }
		        result &= ValidatePciCapRegisterROAttribute((PciSpc)j);
            }
        }
    }
    return result;
}


bool
AllPciRegs_r10b::ValidatePciCapRegisterROAttribute(PciSpc reg)
{
    uint64_t value;
    uint64_t expectedValue;
    const PciSpcType *pciMetrics = gRegisters->GetPciMetrics();
    bool result = true;

    if (pciMetrics[reg].size > MAX_SUPPORTED_REG_SIZE) {
        for (uint32_t k = 0; (k*DWORD_LEN) < pciMetrics[reg].size; k++) {
            if (gRegisters->Read(NVMEIO_PCI_HDR, DWORD_LEN,
                pciMetrics[reg].offset + (k * DWORD_LEN),
                (uint8_t *)&value) == false) {

                throw FrmwkEx(HERE);
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
                    result = false;
                }
            }
        }
    } else if (gRegisters->Read(reg, value) == false) {
        throw FrmwkEx(HERE);
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
            result = false;
        }
    }
    return result;
}


bool
AllPciRegs_r10b::ValidatePciHdrRegisterROAttribute(PciSpc reg)
{
    uint64_t value;
    uint64_t expectedValue;
    const PciSpcType *pciMetrics = gRegisters->GetPciMetrics();
    bool result = true;

    if (pciMetrics[reg].size > MAX_SUPPORTED_REG_SIZE) {
        for (int k = 0; (k*sizeof(value)) < pciMetrics[reg].size; k++) {

            if (gRegisters->Read(NVMEIO_PCI_HDR, sizeof(value),
                pciMetrics[reg].offset + (k * sizeof(value)),
                (uint8_t *)&value, true) == false) {

                throw FrmwkEx(HERE);
            } else {
                // Ignore the implementation specific bits, and bits that the
                // manufacturer can make a decision as to their type of access
                value &= ~pciMetrics[reg].impSpec;

                // Verify that the RO bits are set to correct default values, no
                // reset needed to achieve this because there's no way to change
                value &= pciMetrics[reg].maskRO;
                expectedValue =
                    (pciMetrics[reg].dfltValue & pciMetrics[reg].maskRO);

                // Take care of special cases 1st
                if (reg == PCISPC_BAR2) {
                    // Learn the behavior of this register, supported or not?
                    uint64_t cmdReg;
                    if (gRegisters->Read(PCISPC_CMD, cmdReg) == false)
                        throw FrmwkEx(HERE);

                    if (cmdReg & CMD_IOSE) {
                        if (value != expectedValue) {
                            LOG_ERR("%s RO bit #%d has incorrect value",
                                pciMetrics[reg].desc,
                                ReportOffendingBitPos(value, expectedValue));
                            result = false;
                        }
                    } else {  // Optional index/Data pair register not supported
                        expectedValue = 0;
                        if (value != expectedValue) {
                            LOG_ERR("%s RO bit #%d has incorrect value",
                                pciMetrics[reg].desc,
                                ReportOffendingBitPos(value, expectedValue));
                            result = false;
                        }
                    }
                } else {    // generically handled all other registers
                    if (value != expectedValue) {
                        LOG_ERR("%s RO bit #%d has incorrect value",
                            pciMetrics[reg].desc,
                            ReportOffendingBitPos(value, expectedValue));
                        result = false;
                    }
                }
            }
        }
    } else if (gRegisters->Read(reg, value) == false) {
        throw FrmwkEx(HERE);
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
                throw FrmwkEx(HERE);

            if (cmdReg & CMD_IOSE) {
                if (value != expectedValue) {
                    LOG_ERR("%s RO bit #%d has incorrect value",
                        pciMetrics[reg].desc,
                        ReportOffendingBitPos(value, expectedValue));
                    result = false;
                }
            } else {    // Optional index/Data pair register not supported
                expectedValue = 0;
                if (value != expectedValue) {
                    LOG_ERR("%s RO bit #%d has incorrect value",
                        pciMetrics[reg].desc,
                        ReportOffendingBitPos(value, expectedValue));
                    result = false;
                }
            }
        } else {    // generically handled all other registers
            if (value != expectedValue) {
                LOG_ERR("%s RO bit #%d has incorrect value",
                    pciMetrics[reg].desc,
                    ReportOffendingBitPos(value, expectedValue));
                result = false;
            }
        }
    }
    return result;
}

}   // namespace

