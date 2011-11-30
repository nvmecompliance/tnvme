#ifndef _DELETEIOQCONTOGPOLL_r10b_H_
#define _DELETEIOQCONTOGPOLL_r10b_H_

#include "test.h"
#include "../Queues/asq.h"
#include "../Queues/acq.h"


/** \verbatim
 * -----------------------------------------------------------------------------
 * ----------------Mandatory rules for children to follow-----------------------
 * -----------------------------------------------------------------------------
 * 1) See notes in the header file of the Test base class
 * \endverbatim
 */
class DeleteIOQContigPoll_r10b : public Test
{
public:
    DeleteIOQContigPoll_r10b(int fd, string grpName, string testName);
    virtual ~DeleteIOQContigPoll_r10b();

    /**
     * IMPORTANT: Read Test::Clone() header comment.
     */
    virtual DeleteIOQContigPoll_r10b *Clone() const
        { return new DeleteIOQContigPoll_r10b(*this); }
    DeleteIOQContigPoll_r10b &operator=(const DeleteIOQContigPoll_r10b &other);
    DeleteIOQContigPoll_r10b(const DeleteIOQContigPoll_r10b &other);


protected:
    virtual bool RunCoreTest();


private:
    ///////////////////////////////////////////////////////////////////////////
    // Adding a member variable? Then edit the copy constructor and operator=().
    ///////////////////////////////////////////////////////////////////////////
    void DeleteIOCQContigPoll(SharedASQPtr asq, SharedACQPtr acq);
    void DeleteIOSQContigPoll(SharedASQPtr asq, SharedACQPtr acq);
};


#endif
