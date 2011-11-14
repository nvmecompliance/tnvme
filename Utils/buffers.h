#ifndef _BUFFERS_H_
#define _BUFFERS_H_

#include <limits.h>
#include "tnvme.h"
#include "fileSystem.h"


/**
* This class is meant not be instantiated because it should only ever contain
* static members. These utility functions can be viewed as wrappers to
* perform common, repetitous tasks which avoids coping the same chunks of
* code throughout the framework.
*
* @note This class may throw exceptions, please see comment within specific
*       methods.
*/
class Buffers
{
public:
    Buffers();
    virtual ~Buffers();

    /**
     * Send the entire contents of this buf starting a bufOffset and continue
     * for length bytes to the logging endpoint.
     * @note This method may throw
     * @param buf Pass a pointer to the buffer to dump
     * @param bufOffset Pass the offset byte for which to start dumping
     * @param length Pass the number of bytes to dump, ULONG_MAX implies all
     * @param totalBufSize Pass the total number of bytes within the buffer
     * @param objName Pass an identifying string for name of the buffer
     */
    static void Log(const uint8_t *buf, uint32_t bufOffset,
        unsigned long length, uint32_t totalBufSize, string objName);

    /**
     * Send the entire contents of this buf starting a bufOffset and continue
     * for length bytes to the file named by filename.
     * @note This method may throw
     * @param filename Pass the name of a file to open for dumping buffer
     * @param buf Pass a pointer to the buffer to dump
     * @param bufOffset Pass the offset byte for which to start dumping
     * @param length Pass the number of bytes to dump, ULONG_MAX implies all
     * @param totalBufSize Pass the total number of bytes within the buffer
     * @param fileHdr Pass a custom file header description to dump
     */
    static void Dump(LogFilename filename, const uint8_t *buf, uint32_t bufOffset,
     unsigned long length, uint32_t totalBufSize, string fileHdr);
};


#endif
