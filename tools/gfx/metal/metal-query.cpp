// metal-query.cpp
#include "metal-query.h"

//#include "metal-util.h"

namespace gfx
{

using namespace Slang;

namespace metal
{
Result QueryPoolImpl::init(const IQueryPool::Desc& desc, DeviceImpl* device)
{
    return SLANG_OK;
}

QueryPoolImpl::~QueryPoolImpl()
{
}

Result QueryPoolImpl::getResult(GfxIndex index, GfxCount count, uint64_t* data)
{
    return SLANG_OK;
}

} // namespace metal
} // namespace gfx
