#ifndef _CTRLRCONFIG_H_
#define _CTRLRCONFIG_H_

#include "tnvme.h"
#include "dnvme.h"
#include "regDefs.h"
#include "subject.h"

/// Subject/Observer pattern for SetState() actions within CtrlrConfig
typedef StateObserver<bool> ObserverCtrlrStateDisable;
typedef StateSubject<bool>  SubjectCtrlrStateDisable;


/**
* This class is the access to the Controller Configuration (CC) register. It
* is true that direct access to the register is allowed/possible via the
* Register singleton via its Read() and Write members, however there are
* features of this class which envelope value added concepts. This class wraps
* the CC register to aid in providing the high layer logic needed by test case
* integration into the testing framework.
*
* @note Singleton's are not allowed to throw exceptions.
*/
class CtrlrConfig : public SubjectCtrlrStateDisable
{
public:
    /**
     * Enforce singleton design pattern.
     * @param fd Pass the opened file descriptor for the device under test
     * @param specRev Pass which compliance is needed to target
     */
    static CtrlrConfig* GetInstance(int fd, SpecRev specRev);
    static void KillInstance();
    ~CtrlrConfig();

    /**
     * Learns of the active IRQ scheme enabled in the device. It doesn't
     * indicate that IRQ's are being used, to use IRQ's CQ's must be created
     * to use IRQ's. The ACQ doesn't have a choice and always uses IRQ's if
     * there is a CQ created in the controller.
     * @param irq Pass reference to receive the active IRQ scheme
     * @return true upon success, otherwise ignore parameter irq.
     */
    bool GetIrqScheme(enum nvme_irq_type &irq);

    /**
     * Set the active IRQ scheme in the device. This only sets up the controller
     * to use this IRQ scheme, in order to use the scheme CQ's must be created.
     * The controller must be disable for this call to succeed.
     * @param newIrq Pass the new IRQ scheme which the controller should use.
     * @return true upon success, otherwise false.
     */
    bool SetIrqScheme(enum nvme_irq_type newIrq);

    /**
     * Is the controller enabled?
     * @return true if enabled, otherwise false
     */
    bool IsStateEnabled();

    /**
     * Set the state of the controller.
     * @param state Pass {ST_ENABLE | ST_DISABLE | ST_DISABLE_COMPLETELY}
     *      ST_ENABLE to enable the controller;
     *      ST_DISABLE to disable, free all kernel memory except what is
     *          needed by the ACQ/ASQ, because those entities remain intact.
     *          The ACQ/ASQ are also reset to the empty state. A re-enabling at
     *          this point would allow the immediate submission of admin cmds
     *          into ACQ.
     *      ST_DISABLE_COMPLETELY to disable and nothing is left intact. This
     *          is as close to a power up situation as one could achieve. The
     *          NVME device resets all registers to default values and dnvme
     *          writes admin Q base addresses and Q sizes to 0, nothing is truly
     *          enabled.
     * @return true if successful, otherwise false
     */
    bool SetState(enum nvme_state state);

    bool ReadRegCC(uint32_t &regVal);
    bool WriteRegCC(uint32_t regVal);

    bool GetIOCQES(uint8_t &value)
        { return GetRegValue(value, CC_IOCQES, 20); }
    bool SetIOCQES(uint8_t value)
        { return SetRegValue(value, 0x0f, CC_IOCQES, 20); }

    bool GetIOSQES(uint8_t &value)
        { return GetRegValue(value, CC_IOSQES, 16); }
    bool SetIOSQES(uint8_t value)
        { return SetRegValue(value, 0x0f, CC_IOSQES, 16); }

    bool GetSHN(uint8_t &value)
        { return GetRegValue(value, CC_SHN, 14); }
    bool SetSHN(uint8_t value)
        { return SetRegValue(value, 0x03, CC_SHN, 14); }

    bool GetAMS(uint8_t &value)
        { return GetRegValue(value, CC_AMS, 11); }
    bool SetAMS(uint8_t value)
        { return SetRegValue(value, 0x07, CC_AMS, 11); }

    bool GetMPS(uint8_t &value)
        { return GetRegValue(value, CC_MPS, 7); }
    bool SetMPS(uint8_t value)
        { return SetRegValue(value, 0x0f, CC_MPS, 7); }
    /// Set page size according to what sysconf(_SC_PAGESIZE) returns
    bool SetMPS();

    bool GetCSS(uint8_t &value)
        { return GetRegValue(value, CC_CSS, 4); }
    bool SetCSS(uint8_t value)
        { return SetRegValue(value, 0x07, CC_CSS, 4); }


private:
    // Implement singleton design pattern
    CtrlrConfig();
    CtrlrConfig(int fd, SpecRev specRev);
    static bool mInstanceFlag;
    static CtrlrConfig *mSingleton;

    /// which spec release is being targeted
    SpecRev mSpecRev;
    /// file descriptor to the device under test
    int mFd;

    /// Current value of controller capabilities register
    uint32_t mRegCAP;

    bool GetRegValue(uint8_t &value, uint64_t regMask, uint8_t bitShift);
    bool SetRegValue(uint8_t value, uint8_t valueMask, uint64_t regMask,
        uint8_t bitShift);
};


#endif
