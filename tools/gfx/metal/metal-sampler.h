// metal-sampler.h
#pragma once

#include "metal-base.h"
#include "metal-device.h"

namespace gfx
{

using namespace Slang;

namespace metal
{

class SamplerStateImpl : public SamplerStateBase
{
public:
    RefPtr<DeviceImpl> m_device;
    SamplerStateImpl(DeviceImpl* device);
    ~SamplerStateImpl();
    virtual SLANG_NO_THROW Result SLANG_MCALL getNativeHandle(InteropHandle* outHandle) override;
};

} // namespace metal
} // namespace gfx
