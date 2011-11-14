#ifndef _CREATEACQASQ_r10b_H_
#define _CREATEACQASQ_r10b_H_

#include "test.h"

#define ACQ_GROUP_ID      "ACQ"
#define ASQ_GROUP_ID      "ASQ"


/** \verbatim
 * -----------------------------------------------------------------------------
 * ----------------Mandatory rules for children to follow-----------------------
 * -----------------------------------------------------------------------------
 * 1) See notes in the header file of the Test base class
 * \endverbatim
 */
class CreateACQASQ_r10b : public Test
{
public:
    CreateACQASQ_r10b(int fd, string grpName, string testName);
    virtual ~CreateACQASQ_r10b();

    /**
     * IMPORTANT: Read Test::Clone() header comment.
     */
    virtual CreateACQASQ_r10b *Clone() const
        { return new CreateACQASQ_r10b(*this); }
    CreateACQASQ_r10b &operator=(const CreateACQASQ_r10b &other);
    CreateACQASQ_r10b(const CreateACQASQ_r10b &other);


protected:
    virtual bool RunCoreTest();


private:
    ///////////////////////////////////////////////////////////////////////////
    // Adding a member variable? Then edit the copy constructor and operator().
    ///////////////////////////////////////////////////////////////////////////
};


#endif
