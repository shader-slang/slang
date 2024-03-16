// metal-gfence.cpp
#include "metal-fence.h"
#include "metal-device.h"

namespace gfx
{

using namespace Slang;

namespace metal
{

FenceImpl::FenceImpl(DeviceImpl* device)
    : m_device(device)
{}

FenceImpl::~FenceImpl()
{
}

Result FenceImpl::init(const IFence::Desc& desc)
{
    return SLANG_OK;
}

Result FenceImpl::getCurrentValue(uint64_t* outValue)
{
    return SLANG_OK;
}

Result FenceImpl::setCurrentValue(uint64_t value)
{
    return SLANG_OK;
}

Result FenceImpl::getSharedHandle(InteropHandle* outHandle)
{
    return SLANG_OK;
}

Result FenceImpl::getNativeHandle(InteropHandle* outNativeHandle)
{
    outNativeHandle->handleValue = 0;
    return SLANG_FAIL;
}

} // namespace metal
} // namespace gfx
