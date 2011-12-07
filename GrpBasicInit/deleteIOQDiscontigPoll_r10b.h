#ifndef _DELETEIOQDISCONTIGPOLL_r10b_H_
#define _DELETEIOQDISCONTIGPOLL_r10b_H_

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
class DeleteIOQDiscontigPoll_r10b : public Test
{
public:
    DeleteIOQDiscontigPoll_r10b(int fd, string grpName, string testName);
    virtual ~DeleteIOQDiscontigPoll_r10b();

    /**
     * IMPORTANT: Read Test::Clone() header comment.
     */
    virtual DeleteIOQDiscontigPoll_r10b *Clone() const
        { return new DeleteIOQDiscontigPoll_r10b(*this); }
    DeleteIOQDiscontigPoll_r10b &operator=(
        const DeleteIOQDiscontigPoll_r10b &other);
    DeleteIOQDiscontigPoll_r10b(const DeleteIOQDiscontigPoll_r10b &other);


protected:
    virtual bool RunCoreTest();


private:
    ///////////////////////////////////////////////////////////////////////////
    // Adding a member variable? Then edit the copy constructor and operator().
    ///////////////////////////////////////////////////////////////////////////
    void DeleteIOCQDiscontigPoll(SharedASQPtr asq, SharedACQPtr acq);
    void DeleteIOSQDiscontigPoll(SharedASQPtr asq, SharedACQPtr acq);
};


#endif
