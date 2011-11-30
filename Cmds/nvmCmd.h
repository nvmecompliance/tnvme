#ifndef _NVMCMD_H_
#define _NVMCMD_H_

#include "cmd.h"


/**
* This class is the base class to NVM cmd set.
*
* @note This class may throw exceptions.
*/
class NVMCmd : public Cmd
{
public:
    /**
     * @param fd Pass the opened file descriptor for the device under test
     * @param objBeingCreated Pass the type of object this child class is
     */
    NVMCmd(int fd, Trackable::ObjType objBeingCreated);
    virtual ~NVMCmd();


protected:
    /**
     * Initialize this object.
     * @param opcode Pass the opcode defining this cmd, per NVME spec.
     * @param dataDir Pass the direction of data for this cmd
     */
    void Init(uint8_t opcode, DataDir dataDir);


private:
    NVMCmd();
};


#endif
