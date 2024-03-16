// metal-buffer.cpp
#include "metal-buffer.h"

namespace gfx
{

using namespace Slang;

namespace metal
{

BufferResourceImpl::BufferResourceImpl(const IBufferResource::Desc& desc, DeviceImpl* renderer)
    : Parent(desc)
    , m_renderer(renderer)
{
    assert(renderer);
}

BufferResourceImpl::~BufferResourceImpl()
{
    if (sharedHandle.handleValue != 0)
    {
    }
}

DeviceAddress BufferResourceImpl::getDeviceAddress()
{
    return (DeviceAddress)0;
}

Result BufferResourceImpl::getNativeResourceHandle(InteropHandle* outHandle)
{
    return SLANG_OK;
}

Result BufferResourceImpl::getSharedHandle(InteropHandle* outHandle)
{
    return SLANG_OK;
}

Result BufferResourceImpl::map(MemoryRange* rangeToRead, void** outPointer)
{
    return SLANG_OK;
}

Result BufferResourceImpl::unmap(MemoryRange* writtenRange)
{
    return SLANG_OK;
}

Result BufferResourceImpl::setDebugName(const char* name)
{
    return SLANG_OK;
}

} // namespace metal
} // namespace gfx
