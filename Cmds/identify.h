#ifndef _IDENTIFY_H_
#define _IDENTIFY_H_

#include "cmd.h"

class Identify;    // forward definition
typedef boost::shared_ptr<Identify>             SharedIdentifyPtr;
#define CAST_TO_Identify(shared_trackable_ptr)  \
        boost::shared_polymorphic_downcast<Identify>(shared_trackable_ptr);


/**
* This class implements the Identify admin cmd
*
* @note This class may throw exceptions.
*/
class Identify : public Cmd
{
public:
    /**
     * @param fd Pass the opened file descriptor for the device under test
     */
    Identify(int fd);
    virtual ~Identify();

    /// @param ctrlr Pass true for controller, otherwise false for namespace
    void SetCNS(bool ctrlr);
    /// @return true for controller data, false for namespace data
    bool GetCNS();

    /// The perfectly sized data buffer should be of this size
    static const uint16_t IDEAL_DATA_SIZE;


private:
    Identify();

    bool mCtrlr;
};


#endif
