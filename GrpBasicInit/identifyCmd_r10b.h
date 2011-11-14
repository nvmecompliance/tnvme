#ifndef _IDENTIFYCMD_r10b_H_
#define _IDENTIFYCMD_r10b_H_

#include "test.h"
#include "../Queues/asq.h"
#include "../Queues/acq.h"

#define IDENTIFY_CTRLR_STRUCT_GROUP_ID      "IdentifyCmdCap"
#define IDENTIFY_NAMESPACE_STRUCT_GROUP_ID  "IdentifyCmdNamSpc"


/** \verbatim
 * -----------------------------------------------------------------------------
 * ----------------Mandatory rules for children to follow-----------------------
 * -----------------------------------------------------------------------------
 * 1) See notes in the header file of the Test base class
 * \endverbatim
 */
class IdentifyCmd_r10b : public Test
{
public:
    IdentifyCmd_r10b(int fd, string grpName, string testName);
    virtual ~IdentifyCmd_r10b();

    /**
     * IMPORTANT: Read Test::Clone() header comment.
     */
    virtual IdentifyCmd_r10b *Clone() const
        { return new IdentifyCmd_r10b(*this); }
    IdentifyCmd_r10b &operator=(const IdentifyCmd_r10b &other);
    IdentifyCmd_r10b(const IdentifyCmd_r10b &other);


protected:
    virtual bool RunCoreTest();


private:
    ///////////////////////////////////////////////////////////////////////////
    // Adding a member variable? Then edit the copy constructor and operator().
    ///////////////////////////////////////////////////////////////////////////
    void SendIdentifyCtrlrStruct(SharedASQPtr asq, SharedACQPtr acq);
    void SendIdentifyNamespaceStruct(SharedASQPtr asq, SharedACQPtr acq);
};


#endif
