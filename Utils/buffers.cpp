#include "buffers.h"
#include "globals.h"


Buffers::Buffers()
{
}


Buffers::~Buffers()
{
}


void
Buffers::Log(const uint8_t *buf, uint32_t bufOffset, unsigned long length,
    uint32_t totalBufSize, string objName)
{
    const uint8_t *data;
    const int BUF_SIZE = 20;
    char work[BUF_SIZE];
    string output;
    unsigned long dumpLen = length;


    LOG_NRM("Logging %s obj contents....", objName.c_str());
    if (bufOffset >= totalBufSize) {
        LOG_DBG("Offset into buffer 0x%08X >= to buffer size 0x%08X",
            bufOffset, totalBufSize);
        throw exception();
    }
    data = &(buf[bufOffset]);
    if (length == ULONG_MAX)
        dumpLen = (totalBufSize - bufOffset);
    else if ((length + bufOffset) >= totalBufSize)
        dumpLen = (totalBufSize - bufOffset);

    for (unsigned long i = 0; i < dumpLen; i++) {
        if ((i % 16) == 15) {
            snprintf(work, BUF_SIZE, " %02X", *data++);
            output += work;
            LOG_NRM("%s", output.c_str());
            output.clear();
        } else if ((i % 16) == 0) {
            snprintf(work, BUF_SIZE, "0x%08X: %02X", (uint32_t)i, *data++);
            output += work;
        } else {
            snprintf(work, BUF_SIZE, " %02X", *data++);
            output += work;
        }
    }
    if (output.length() != 0)
        LOG_NRM("%s", output.c_str());
}


void
Buffers::Dump(LogFilename filename, const uint8_t *buf, uint32_t bufOffset,
    unsigned long length, uint32_t totalBufSize, string fileHdr)
{
    const uint8_t *data;
    const int BUF_SIZE = 20;
    char work[BUF_SIZE];
    string output;
    FILE *fp;
    unsigned long dumpLen = length;


    LOG_NRM("Dump a test obj to filename: %s", filename.c_str());
    LOG_NRM("%s", fileHdr.c_str());
    if ((fp = fopen(filename.c_str(), "w+")) == NULL) {
        LOG_DBG("Failed to open file: %s", filename.c_str());
        throw exception();
    }
    fprintf(fp, "This file: %s\n", filename.c_str());
    fprintf(fp, "%s\n", fileHdr.c_str());

    if (bufOffset >= totalBufSize) {
        LOG_DBG("Offset into buffer 0x%08X >= to buffer size 0x%08X",
            bufOffset, totalBufSize);
        throw exception();
    }

    data = &(buf[bufOffset]);
    if (length == ULONG_MAX)
        dumpLen = (totalBufSize - bufOffset);
    else if ((length + bufOffset) >= totalBufSize)
        dumpLen = (totalBufSize - bufOffset);

    for (unsigned long i = 0; i < dumpLen; i++) {
        if ((i % 16) == 15) {
            snprintf(work, BUF_SIZE, " %02X\n", *data++);
            output += work;
            fprintf(fp, "%s", output.c_str());
            output.clear();
        } else if ((i % 16) == 0) {
            snprintf(work, BUF_SIZE, "0x%08X: %02X", (uint32_t)i, *data++);
            output += work;
        } else {
            snprintf(work, BUF_SIZE, " %02X", *data++);
            output += work;
        }
    }
    if (output.length() != 0)
        fprintf(fp, "%s\n", output.c_str());

    fclose(fp);
}
