// metal-resource-views.cpp
#include "metal-resource-views.h"

namespace gfx
{

using namespace Slang;

namespace metal
{

TextureResourceViewImpl::~TextureResourceViewImpl()
{
}

Result TextureResourceViewImpl::getNativeHandle(InteropHandle* outHandle)
{
    return SLANG_E_NOT_IMPLEMENTED;
}

BufferResourceViewImpl::~BufferResourceViewImpl()
{
}

Result BufferResourceViewImpl::getNativeHandle(InteropHandle* outHandle)
{
    return SLANG_E_NOT_IMPLEMENTED;
}

TexelBufferResourceViewImpl::TexelBufferResourceViewImpl(DeviceImpl* device)
    : ResourceViewImpl(ViewType::TexelBuffer, device)
{}

TexelBufferResourceViewImpl::~TexelBufferResourceViewImpl()
{
}

Result TexelBufferResourceViewImpl::getNativeHandle(InteropHandle* outHandle)
{
    return SLANG_E_NOT_IMPLEMENTED;
}

DeviceAddress AccelerationStructureImpl::getDeviceAddress()
{
    return 0;
}

Result AccelerationStructureImpl::getNativeHandle(InteropHandle* outHandle)
{
    return SLANG_E_NOT_IMPLEMENTED;
}

AccelerationStructureImpl::~AccelerationStructureImpl()
{
}

} // namespace metal
} // namespace gfx
