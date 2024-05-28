// metal-query.h
#pragma once

#include "metal-base.h"
#include "metal-device.h"

namespace gfx
{

using namespace Slang;

namespace metal
{

class QueryPoolImpl : public QueryPoolBase
{
public:
    Result init(const IQueryPool::Desc& desc, DeviceImpl* device);
    ~QueryPoolImpl();

public:
    virtual SLANG_NO_THROW Result SLANG_MCALL
        getResult(GfxIndex index, GfxCount count, uint64_t* data) override;

public:
    RefPtr<DeviceImpl> m_device;
};

} // namespace metal
} // namespace gfx
