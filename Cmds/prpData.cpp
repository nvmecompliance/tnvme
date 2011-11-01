#include "prpData.h"

using namespace std;


PrpData::PrpData()
{
    throw exception();
}


PrpData::PrpData(Trackable::Lifetime life, bool ownByRsrcMngr)
{
    mLifetime = life;
    mOwnByRsrcMngr = ownByRsrcMngr;
    mBuf = NULL;
    mPrpFields = (send_64b_bitmask)0;
}


PrpData::~PrpData()
{
    if (mOwnTheBuffer && (mBuf != NULL))
        delete mBuf;
}


void
PrpData::SetBuffer(MemBuffer &memBuffer, send_64b_bitmask prpFields)
{
    if (memBuffer.GetOwnByRsrcMngr() != mOwnByRsrcMngr) {
        LOG_DBG("MemBuffer wasn't created via same means as Q object");
        throw exception();
    } else if (memBuffer.GetObjLife() != this->GetObjLife()) {
        // Can't have the memory of the Q live shorter than the Q itself
        LOG_DBG("MemBuffer doesn't have same life span as Q object");
        throw exception();
    }

    mBuf = memBuf;
    mOwnTheBuffer = ownIt;
    mPrpFields = prpFields;
}
