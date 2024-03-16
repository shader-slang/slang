// metal-sampler.cpp
#include "metal-sampler.h"

namespace gfx
{

using namespace Slang;

namespace metal
{

SamplerStateImpl::SamplerStateImpl(DeviceImpl* device)
    : m_device(device)
{}

SamplerStateImpl::~SamplerStateImpl()
{
}

Result SamplerStateImpl::getNativeHandle(InteropHandle* outHandle)
{
    return SLANG_OK;
}

} // namespace metal
} // namespace gfx
