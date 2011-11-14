#include "metaData.h"
#include "globals.h"

using namespace std;


MetaData::MetaData()
{
}


MetaData::~MetaData()
{
    gRsrcMngr->ReleaseMetaBuf(mMetaData);
}


void
MetaData::AllocMetaBuffer()
{
    if (gRsrcMngr->ReserveMetaBuf(mMetaData) == false) {
        LOG_ERR("Meta data alloc request denied");
        throw exception();
    }
}


send_64b_bitmask
MetaData::GetMetaBitmask()
{
    // If its still a default object then nothing has allocated a meta data buf
    if (mMetaData == MetaDataBuf())
        return (send_64b_bitmask)0;
    return MASK_MPTR;
}
