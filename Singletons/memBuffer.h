/*
 * Copyright (c) 2011, Intel Corporation.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef _MEMBUFFER_H_
#define _MEMBUFFER_H_

#include <boost/shared_ptr.hpp>
#include "tnvme.h"
#include "trackable.h"
#include "limits.h"
#include "../Utils/fileSystem.h"

#define PRP_BUFFER_ALIGNMENT        128

class MemBuffer;    // forward definition
typedef boost::shared_ptr<MemBuffer>            SharedMemBufferPtr;
#define CAST_TO_MEMBUFFER(shared_trackable_ptr) \
        boost::shared_polymorphic_downcast<MemBuffer>(shared_trackable_ptr);


/**
* This class is a wrapper for a raw heap allocated memory buffer. It may
* be created and destroyed by the RsrcMngr, however that is not strictly
* necessary. These buffers can be specified to have certain alignment criteria
* to be used for CQ/SQ memory and user data buffers. After instantiation the
* Initxxxxxx() methods must be called to attain something useful.
*
* @note This class may throw exceptions.
*/
class MemBuffer : public Trackable
{
public:
    MemBuffer();
    /**
     * @note Uses member Init() to perform the alignment of the buffer
     * @param initData Pass the data to perform initialization of contents.
     */
    MemBuffer(const vector<uint8_t> &initData);
    virtual ~MemBuffer();

    /// Used to compare for NULL pointers being returned by allocations
    static SharedMemBufferPtr NullMemBufferPtr;

    /**
     * Allocates memory allowing to specify an offset into the 1st page of
     * allocated memory, and thus also the alignment of that buffer. More memory
     * may be allocated than requested to satisfy the request. If a specific
     * offset into a page is not strictly necessary then calling
     * InitAlignment() should be more efficient at allocations, because entire
     * pages of memory may not be required to satisfy the request.
     * @param bufSize Pass the minimum number of bytes for buffer creation
     * @param offset1stPg Pass the byte offset into the 1st page of allocated
     *        memory which the buffer is intended to start. offset == 0 implies
     *        page aligned memory. This value is enforced to a multiple of 32
     *        bit alignment.
     * @param initMem Pass true to initialize all elements, otherwise don't init
     * @param initVal Pass the init value if suppose to init the buffer
     */
    void InitOffset1stPage(uint32_t bufSize, uint32_t offset1stPg = 0,
        bool initMem = false, uint8_t initVal = 0);

    /**
     * Allocates memory allowing to specify the alignment of that buffer, but
     * not the offset into the 1st page of the allocation. Residual memory
     * may be consumed to satisfy this request since entire pages of memory
     * may not be necessary, and therefore the allocation of > what was
     * requested will not be necessary.
     * @param bufSize Pass the number of bytes for buffer creation
     * @param align Pass the alignment requirements of the buffer. This value
     *        is enforced to a multiple of sizeof(void *) alignment at min.
     * @param initMem Pass true to initialize all elements, otherwise don't init
     * @param initVal Pass the init value if suppose to init the buffer
     */
    void InitAlignment(uint32_t bufSize, uint32_t align = PRP_BUFFER_ALIGNMENT,
        bool initMem = false, uint8_t initVal = 0);

    /**
     * Allocates memory not allowed to specify anything, heap memory is taken
     * and alignment and offset into the 1st page is not guaranteed. Residual
     * memory will most likely be consumed.
     * @param bufSize Pass the number of bytes for buffer creation
     * @param initMem Pass true to initialize all elements, otherwise don't init
     * @param initVal Pass the init value if suppose to init the buffer
     */
    void Init(uint32_t bufSize, bool initMem = false, uint8_t initVal = 0);

    /**
     * Get the buffer's byte value at the provided offset from beginning of
     * the buffer.
     * @param offset Pass the offset of the byte value to return
     * @return The requested byte value
     */
    uint8_t GetAt(size_t offset);

    uint8_t *GetBuffer() { return mVirBaseAddr; }
    uint32_t GetBufSize() { return mVirBufSize; }
    uint32_t GetAlignment() { return mAlignment; }

    /**
     * Write a data pattern to a segment of the data buffer. This segment
     * is defined by the offset from the start of the data buffer and
     * continues 'patLength' bytes. This segment is the only portion affected.
     * This segment starts with the stated initial value and progress according
     * to the desired pattern/series.
     * @param dataPat Pass the desired data pattern/series to calc next value
     * @param initVal Pass the 1st value of the pattern/series
     * @param offset Pass offset into the meta buf which is start of the segment
     * @param length Pass the number of bytes of the segment length, value
     *        of UINT32_MAX implies infinite length.
     */
    void SetDataPattern(DataPattern dataPat, uint64_t initVal = 0,
        uint32_t offset = 0, uint32_t length = UINT32_MAX);

    /// Zero out all memory bytes
    void Zero() { SetDataPattern(DATAPAT_CONST_8BIT, 0); }

    /**
     * Compare a specified MemBuffer to this one.
     * @param compTo Pass a reference to the memory to compare against
     * @return true upon all data exactly identical, false is miscompare, and
     *      throws when buffers are not of same size or other serious error
     *      which causes the inability to compare data.
     */
    bool Compare(const SharedMemBufferPtr compTo);
    bool Compare(const vector<uint8_t> &compTo);

    /**
     * Send the entire contents of this buffer to the logging endpoint
     * @param bufOffset Pass the offset byte for which to start dumping
     * @param length Pass the number of bytes to dump, ULONG_MAX implies all
     */
    void Log(uint32_t bufOffset = 0, unsigned long length = ULONG_MAX);

    /**
     * Send the entire contents of this buffer to the named file.
     * @param filename Pass the filename as generated by macro
     *      FileSystem::PrepLogFile().
     * @param fileHdr Pass a custom file header description to dump
     */
    void Dump(DumpFilename filename, string fileHdr);


private:
    bool mAllocByNewOperator;
    uint8_t *mRealBaseAddr;     // System address returned by posix_memalign()
    uint8_t *mVirBaseAddr;      // User buffer address to satisfy mOffset1stPg
    uint32_t mVirBufSize;       // User request buffer size
    uint32_t mAlignment;

    void InitMemberVariables();
    void DeallocateResources();
};


#endif
