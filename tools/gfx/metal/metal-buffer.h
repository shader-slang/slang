// metal-buffer.h
#pragma once

#include "metal-base.h"
#include "metal-device.h"

namespace gfx
{

using namespace Slang;

namespace metal
{

class BufferResourceImpl : public BufferResource
{
public:
    typedef BufferResource Parent;

    BufferResourceImpl(const IBufferResource::Desc& desc, DeviceImpl* renderer);

    ~BufferResourceImpl();

    DeviceImpl* m_renderer;
    MTL::Buffer* m_buffer = nullptr;

    virtual SLANG_NO_THROW DeviceAddress SLANG_MCALL getDeviceAddress() override;

    virtual SLANG_NO_THROW Result SLANG_MCALL
        getNativeResourceHandle(InteropHandle* outHandle) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL getSharedHandle(InteropHandle* outHandle) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL
        map(MemoryRange* rangeToRead, void** outPointer) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL unmap(MemoryRange* writtenRange) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL setDebugName(const char* name) override;
};

} // namespace metal
} // namespace gfx
