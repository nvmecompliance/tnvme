#ifndef _NVMECMD_H_
#define _NVMECMD_H_

#include "cmd.h"


/**
* This class is the base class to NVME cmd set.
*
* @note This class may throw exceptions.
*/
class NVMECmd : public Cmd
{
public:
    /**
     * @param fd Pass the opened file descriptor for the device under test
     * @param objBeingCreated Pass the type of object this child class is
     */
    NVMECmd(int fd, Trackable::ObjType objBeingCreated);
    virtual ~NVMECmd();


protected:
    /**
     * Initialize this object.
     * @param opcode Pass the opcode defining this cmd, per NVME spec.
     * @param dataDir Pass the direction of data for this cmd
     */
    void Init(uint8_t opcode, DataDir dataDir);


private:
    NVMECmd();
};


#endif
