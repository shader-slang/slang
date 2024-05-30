// metal-command-buffer.h
#pragma once

#include "metal-base.h"
#include "metal-shader-object.h"
#include "metal-command-encoder.h"
#include "../simple-transient-resource-heap.h"

namespace gfx
{

using namespace Slang;

namespace metal
{

class CommandBufferImpl
    : public ICommandBuffer
    , public ComObject
{
public:
    SLANG_COM_OBJECT_IUNKNOWN_ALL
    ICommandBuffer* getInterface(const Guid& guid);
    virtual void comFree() override;

public:
    MTL::CommandBuffer* m_commandBuffer = nullptr;
    DeviceImpl* m_renderer;
    //bool m_isPreCommandBufferEmpty = true;
    RootShaderObjectImpl m_rootObject;

    ResourceCommandEncoder* m_resourceCommandEncoder = nullptr;
    ComputeCommandEncoder* m_computeCommandEncoder = nullptr;
    RenderCommandEncoder* m_renderCommandEncoder = nullptr;
    RayTracingCommandEncoder* m_rayTracingCommandEncoder = nullptr;

    // Command buffers are deallocated by its command pool,
    // so no need to free individually.
    ~CommandBufferImpl() = default;

    using TransientResourceHeapImpl = gfx::SimpleTransientResourceHeap<DeviceImpl, CommandBufferImpl>;
    Result init(DeviceImpl* renderer, TransientResourceHeapImpl* transientHeap);

    void beginCommandBuffer();

public:
    virtual SLANG_NO_THROW void SLANG_MCALL encodeRenderCommands(
        IRenderPassLayout* renderPass,
        IFramebuffer* framebuffer,
        IRenderCommandEncoder** outEncoder) override;
    virtual SLANG_NO_THROW void SLANG_MCALL
        encodeComputeCommands(IComputeCommandEncoder** outEncoder) override;
    virtual SLANG_NO_THROW void SLANG_MCALL
        encodeResourceCommands(IResourceCommandEncoder** outEncoder) override;
    virtual SLANG_NO_THROW void SLANG_MCALL
        encodeRayTracingCommands(IRayTracingCommandEncoder** outEncoder) override;
    virtual SLANG_NO_THROW void SLANG_MCALL close() override;
    virtual SLANG_NO_THROW Result SLANG_MCALL getNativeHandle(InteropHandle* outHandle) override;
};

} // namespace metal
} // namespace gfx
