#include "allPciRegs_r10b.h"
#include "../globals.h"


AllPciRegs_r10b::AllPciRegs_r10b(int fd) : Test(fd)
{
    // 66 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0a, section 3");
    mTestDesc.SetShort(     "Validate all PCI registers syntactically");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Validates the following; the RO fields which are not implementation "
        "specific contain default values; The RO fields cannot be written; All "
        "ASCII fields only contain chars 0x20 to 0x7e.");
}


AllPciRegs_r10b::~AllPciRegs_r10b()
{
}


bool
AllPciRegs_r10b::RunCoreTest()
{
    if (ValidateDefaultValues() == false)
        return false;
    else if (ValidateROBitsAfterWriting() == false)
        return false;
    return true;
}


bool
AllPciRegs_r10b::ValidateDefaultValues()
{
    const PciSpcType *pciMetrics = gRegisters->GetPciMetrics();
    const vector<PciCapabilities> *pciCap = gRegisters->GetPciCapabilities();


    // Traverse the PCI header registers
    for (int j = 0; j < PCISPC_FENCE; j++) {
        if (pciMetrics[j].specRev != SPECREV_10b)
            continue;

        // PCI hdr registers don't have an assoc capability
        if (pciMetrics[j].cap == PCICAP_FENCE) {
            if (ValidatePciHdrRegisterROAttribute((PciSpc)j) == false)
                return false;
        }
    }

    // Traverse all discovered capabilities
    for (size_t i = 0; i < pciCap->size(); i++) {
        // Read all registers assoc with the discovered capability
        for (int j = 0; j < PCISPC_FENCE; j++) {
            if (pciMetrics[j].specRev != SPECREV_10b)
                continue;
            else if (pciCap->at(i) == pciMetrics[j].cap) {
                if (ValidatePciCapRegisterROAttribute((PciSpc)j) == false)
                    return false;
            }
        }
    }

    return true;
}


bool
AllPciRegs_r10b::ValidateROBitsAfterWriting()
{
    ULONGLONG value;
    const PciSpcType *pciMetrics = gRegisters->GetPciMetrics();
    const vector<PciCapabilities> *pciCap = gRegisters->GetPciCapabilities();


    // Traverse the PCI header registers
    for (int j = 0; j < PCISPC_FENCE; j++) {
        if (pciMetrics[j].specRev != SPECREV_10b)
            continue;

        // Reserved areas at NOT suppose to be written
        if ((j == PCISPC_RES0) || (j == PCISPC_RES1))
            continue;

        // PCI hdr registers don't have an assoc capability
        if (pciMetrics[j].cap == PCICAP_FENCE) {
            LOG_NRM("Validate RO attribute after trying to write 1");
            value = 0xffffffffffffffffULL;
            if (gRegisters->Write((PciSpc)j, value) == false)
                return false;
            if (ValidatePciHdrRegisterROAttribute((PciSpc)j) == false)
                return false;

            LOG_NRM("Validate RO attribute after trying to write 0");
            value = 0x0ULL;
            if (gRegisters->Write((PciSpc)j, value) == false)
                return false;
            if (ValidatePciHdrRegisterROAttribute((PciSpc)j) == false)
                return false;
        }
    }

    // Traverse all discovered capabilities
    for (size_t i = 0; i < pciCap->size(); i++) {
        // Read all registers assoc with the discovered capability
        for (int j = 0; j < PCISPC_FENCE; j++) {
            if (pciMetrics[j].specRev != SPECREV_10b)
                continue;

            // Reserved areas at NOT suppose to be written
            if ((j == PCISPC_RES0) || (j == PCISPC_RES1))
                continue;

            if (pciCap->at(i) == pciMetrics[j].cap) {
                LOG_NRM("Validate RO attribute after trying to write 1");
                value = 0xffffffffffffffffULL;
                if (gRegisters->Write((PciSpc)j, value) == false)
                    return false;
                if (ValidatePciCapRegisterROAttribute((PciSpc)j) == false)
                    return false;

                LOG_NRM("Validate RO attribute after trying to write 0");
                value = 0x0ULL;
                if (gRegisters->Write((PciSpc)j, value) == false)
                    return false;
                if (ValidatePciCapRegisterROAttribute((PciSpc)j) == false)
                    return false;
            }
        }
    }

    return true;
}


int
AllPciRegs_r10b::ReportOffendingBitPos(ULONGLONG val, ULONGLONG expectedVal)
{
    ULONGLONG bitMask;

    for (int i = 0; i < (int)(sizeof(ULONGLONG)*8); i++) {
        bitMask = (1 << i);
        if ((val & bitMask) != (expectedVal & bitMask))
            return i;
    }
    return INT_MAX; // there is no mismatch
}


bool
AllPciRegs_r10b::ValidatePciCapRegisterROAttribute(PciSpc reg)
{
    ULONGLONG value;
    ULONGLONG expectedValue;
    const PciSpcType *pciMetrics = gRegisters->GetPciMetrics();


    if (pciMetrics[reg].size > MAX_SUPPORTED_REG_SIZE) {
        for (int k = 0; (k*sizeof(value)) < pciMetrics[reg].size; k++) {

            if (gRegisters->Read(NVMEIO_PCI_HDR, sizeof(value),
                pciMetrics[reg].offset + (k * sizeof(value)),
                (unsigned char *)&value) == false) {

                return false;
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
                    return false;
                }
            }
        }
    } else if (gRegisters->Read(reg, value) == false) {
        return false;
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
            return false;
        }
    }

    return true;
}


bool
AllPciRegs_r10b::ValidatePciHdrRegisterROAttribute(PciSpc reg)
{
    ULONGLONG value;
    ULONGLONG expectedValue;
    const PciSpcType *pciMetrics = gRegisters->GetPciMetrics();

    if (pciMetrics[reg].size > MAX_SUPPORTED_REG_SIZE) {
        for (int k = 0; (k*sizeof(value)) < pciMetrics[reg].size; k++) {

            if (gRegisters->Read(NVMEIO_PCI_HDR, sizeof(value),
                pciMetrics[reg].offset + (k * sizeof(value)),
                (unsigned char *)&value) == false) {

                return false;
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
                    ULONGLONG cmdReg;
                    if (gRegisters->Read(PCISPC_CMD, cmdReg) == false)
                        return false;

                    if (cmdReg & CMD_IOSE) {
                        if (value != expectedValue) {
                            LOG_ERR("%s RO bit #%d has incorrect value",
                                pciMetrics[reg].desc,
                                ReportOffendingBitPos(value, expectedValue));
                            return false;
                        }
                    } else {    // Optional index/Data pair register not supported
                        expectedValue = 0;
                        if (value != expectedValue) {
                            LOG_ERR("%s RO bit #%d has incorrect value",
                                pciMetrics[reg].desc,
                                ReportOffendingBitPos(value, expectedValue));
                            return false;
                        }
                    }
                } else {    // generically handled all other registers
                    if (value != expectedValue) {
                        LOG_ERR("%s RO bit #%d has incorrect value",
                            pciMetrics[reg].desc,
                            ReportOffendingBitPos(value, expectedValue));
                        return false;
                    }
                }
            }
        }
    } else if (gRegisters->Read(reg, value) == false) {
        return false;
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
            ULONGLONG cmdReg;
            if (gRegisters->Read(PCISPC_CMD, cmdReg) == false)
                return false;

            if (cmdReg & CMD_IOSE) {
                if (value != expectedValue) {
                    LOG_ERR("%s RO bit #%d has incorrect value",
                        pciMetrics[reg].desc,
                        ReportOffendingBitPos(value, expectedValue));
                    return false;
                }
            } else {    // Optional index/Data pair register not supported
                expectedValue = 0;
                if (value != expectedValue) {
                    LOG_ERR("%s RO bit #%d has incorrect value",
                        pciMetrics[reg].desc,
                        ReportOffendingBitPos(value, expectedValue));
                    return false;
                }
            }
        } else {    // generically handled all other registers
            if (value != expectedValue) {
                LOG_ERR("%s RO bit #%d has incorrect value",
                    pciMetrics[reg].desc,
                    ReportOffendingBitPos(value, expectedValue));
                return false;
            }
        }
    }

    return true;
}


