#include "metaData.h"

using namespace std;


MetaData::MetaData()
{
}


MetaData::~MetaData()
{
    gRsrcMngr->ReleaseMetaBuf(mMetaData);
}


void
MetaData::AllocBuffer()
{
    if (gRsrcMngr->ReserveMetaBuf(mMetaData) == false) {
        LOG_ERR("Meta data alloc request denied");
        throw exception();
    }
}
