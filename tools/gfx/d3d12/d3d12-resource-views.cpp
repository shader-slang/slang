// d3d12-resource-views.cpp
#include "d3d12-resource-views.h"

namespace gfx
{
namespace d3d12
{

using namespace Slang;

ResourceViewInternalImpl::~ResourceViewInternalImpl()
{
    if (m_descriptor.cpuHandle.ptr)
        m_allocator->free(m_descriptor);
}

Result ResourceViewImpl::getNativeHandle(InteropHandle* outHandle)
{
    outHandle->api = InteropHandleAPI::D3D12CpuDescriptorHandle;
    outHandle->handleValue = m_descriptor.cpuHandle.ptr;
    return SLANG_OK;
}

#if SLANG_GFX_HAS_DXR_SUPPORT

DeviceAddress AccelerationStructureImpl::getDeviceAddress()
{
    return m_buffer->getDeviceAddress() + m_offset;
}

Result AccelerationStructureImpl::getNativeHandle(InteropHandle* outHandle)
{
    outHandle->api = InteropHandleAPI::DeviceAddress;
    outHandle->handleValue = getDeviceAddress();
    return SLANG_OK;
}

#endif // SLANG_GFX_HAS_DXR_SUPPORT

} // namespace d3d12
} // namespace gfx
