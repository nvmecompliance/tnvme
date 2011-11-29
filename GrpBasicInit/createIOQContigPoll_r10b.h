#ifndef _CREATEIOQCONTOGPOLL_r10b_H_
#define _CREATEIOQCONTOGPOLL_r10b_H_

#include "test.h"
#include "../Queues/asq.h"
#include "../Queues/acq.h"

#define IOCQ_CONTIG_POLL_GROUP_ID      "IOCQContigPoll"
#define IOSQ_CONTIG_POLL_GROUP_ID      "IOSQContigPoll"


/** \verbatim
 * -----------------------------------------------------------------------------
 * ----------------Mandatory rules for children to follow-----------------------
 * -----------------------------------------------------------------------------
 * 1) See notes in the header file of the Test base class
 * \endverbatim
 */
class CreateIOQContigPoll_r10b : public Test
{
public:
    CreateIOQContigPoll_r10b(int fd, string grpName, string testName);
    virtual ~CreateIOQContigPoll_r10b();

    /**
     * IMPORTANT: Read Test::Clone() header comment.
     */
    virtual CreateIOQContigPoll_r10b *Clone() const
        { return new CreateIOQContigPoll_r10b(*this); }
    CreateIOQContigPoll_r10b &operator=(const CreateIOQContigPoll_r10b &other);
    CreateIOQContigPoll_r10b(const CreateIOQContigPoll_r10b &other);


protected:
    virtual bool RunCoreTest();


private:
    ///////////////////////////////////////////////////////////////////////////
    // Adding a member variable? Then edit the copy constructor and operator().
    ///////////////////////////////////////////////////////////////////////////
    void CreateIOCQContigPoll(SharedASQPtr asq, SharedACQPtr acq);
    void CreateIOSQContigPoll(SharedASQPtr asq, SharedACQPtr acq);
};


#endif
