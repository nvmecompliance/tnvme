#ifndef _REGISTERS_H_
#define _REGISTERS_H_

#include <string>
#include <vector>
#include "regDefs.h"
#include "dnvme/dnvme_interface.h"

/**
 * @param regVal Pass the 64 bit register value to mask down to size.
 * @param bytes Pass the number of bytes to keep of the 8 byte regVal
 */
#define REGMASK(regval, bytes)  \
        (regval & (0xffffffffffffffffULL >> (64 - (bytes * 8))))

using namespace std;


/**
* This class is meant to interface with PCI and/or ctrl'r registers
*/
class Registers
{
public:
    /**
     * @param fd Pass the opened file descriptor for the device under test
     * @param specRev Pass which compliance is needed to target
     */
    Registers(int fd, SpecRev specRev);
    virtual ~Registers();

    /**
     * Read a register value from the appropriate address space.
     * @param reg Pass which register to read.
     * @param value Returns the value read, if and only if successful. The
     *          lowest order nibbles are populated 1st if the register is
     *          smaller than sizeof(value)
     * @return true upon success, otherwise false
     */
    bool Read(PciSpc reg, unsigned long long &value);
    bool Read(CtlSpc reg, unsigned long long &value);

    /**
     * Generic read function, you supply ALL the necessary data to read at
     * specified offset, length of register, which address space to read, etc.
     * @param regSpc Pass which register space to read from
     * @param rsize Pass the length in bytes of the register
     * @param roffset Pass the offset from start of spec'd address space to
     *        start reading.
     * @param value Returns the value read, if and only if successful.
     * @return true upon success, otherwise false
     */
    bool Read(nvme_io_space regSpc, unsigned int rsize, unsigned int roffset,
        unsigned char *value);

    /**
     * Write a register value to the appropriate address space.
     * @param reg Pass which register to write.
     * @param value Pass the value to write.
     * @return true upon success, otherwise false
     */
    bool Write(PciSpc reg, unsigned long long value);
    bool Write(CtlSpc reg, unsigned long long value);

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
     *          smaller than the unsigned long long variable size.
     * @return the formatted resulting string
     */
    string FormatRegister(unsigned int regSize, const char *regDesc,
        unsigned long long regValue);
    string FormatRegister(nvme_io_space regSpc, unsigned int rsize,
        unsigned int roffset, unsigned char *value);


private:
    Registers();

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

    bool Read(nvme_io_space regSpc, unsigned int rsize, unsigned int roffset,
        unsigned long long &value, const char *rdesc);
    bool Write(nvme_io_space regSpc, unsigned int rsize, unsigned int roffset,
        unsigned long long &value, const char *rdesc);
};


#endif
