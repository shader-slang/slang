// metal-fence.cpp
#include "metal-fence.h"

#include "metal-device.h"

namespace gfx
{

using namespace Slang;

namespace metal
{

FenceImpl::~FenceImpl() {}

Result FenceImpl::init(DeviceImpl* device, const IFence::Desc& desc)
{
    m_device = device;
    m_event = NS::TransferPtr(m_device->m_device->newSharedEvent());
    if (!m_event)
    {
        return SLANG_FAIL;
    }
    m_event->setSignaledValue(desc.initialValue);
    return SLANG_OK;
}

Result FenceImpl::getCurrentValue(uint64_t* outValue)
{
    *outValue = m_event->signaledValue();
    return SLANG_OK;
}

Result FenceImpl::setCurrentValue(uint64_t value)
{
    m_event->setSignaledValue(value);
    return SLANG_OK;
}

Result FenceImpl::getSharedHandle(InteropHandle* outHandle)
{
    return SLANG_E_NOT_AVAILABLE;
}

Result FenceImpl::getNativeHandle(InteropHandle* outNativeHandle)
{
    outNativeHandle->api = InteropHandleAPI::Metal;
    outNativeHandle->handleValue = reinterpret_cast<intptr_t>(m_event.get());
    return SLANG_OK;
}

} // namespace metal
} // namespace gfx
