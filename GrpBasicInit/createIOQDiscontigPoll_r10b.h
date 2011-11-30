#ifndef _CREATEIOQDISCONTOGPOLL_r10b_H_
#define _CREATEIOQDISCONTOGPOLL_r10b_H_

#include "test.h"
#include "../Queues/asq.h"
#include "../Queues/acq.h"

#define IOCQ_DISCONTIG_POLL_GROUP_ID      "IOCQDiscontigPoll"
#define IOSQ_DISCONTIG_POLL_GROUP_ID      "IOSQDiscontigPoll"


/** \verbatim
 * -----------------------------------------------------------------------------
 * ----------------Mandatory rules for children to follow-----------------------
 * -----------------------------------------------------------------------------
 * 1) See notes in the header file of the Test base class
 * \endverbatim
 */
class CreateIOQDiscontigPoll_r10b : public Test
{
public:
    CreateIOQDiscontigPoll_r10b(int fd, string grpName, string testName);
    virtual ~CreateIOQDiscontigPoll_r10b();

    /**
     * IMPORTANT: Read Test::Clone() header comment.
     */
    virtual CreateIOQDiscontigPoll_r10b *Clone() const
        { return new CreateIOQDiscontigPoll_r10b(*this); }
    CreateIOQDiscontigPoll_r10b &operator=(
        const CreateIOQDiscontigPoll_r10b &other);
    CreateIOQDiscontigPoll_r10b(const CreateIOQDiscontigPoll_r10b &other);


protected:
    virtual bool RunCoreTest();


private:
    ///////////////////////////////////////////////////////////////////////////
    // Adding a member variable? Then edit the copy constructor and operator=().
    ///////////////////////////////////////////////////////////////////////////
    void CreateIOCQDiscontigPoll(SharedASQPtr asq, SharedACQPtr acq);
    void CreateIOSQDiscontigPoll(SharedASQPtr asq, SharedACQPtr acq);
};


#endif
