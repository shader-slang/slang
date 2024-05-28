// metal-command-buffer.cpp
#include "metal-command-buffer.h"

#include "metal-device.h"
#include "metal-command-encoder.h"
#include "metal-shader-object.h"
#include "metal-command-queue.h"

namespace gfx
{

using namespace Slang;

namespace metal
{

ICommandBuffer* CommandBufferImpl::getInterface(const Guid& guid)
{
    if (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_ICommandBuffer)
        return static_cast<ICommandBuffer*>(this);
    return nullptr;
}

void CommandBufferImpl::comFree() { }

Result CommandBufferImpl::init(DeviceImpl* renderer, TransientResourceHeapImpl* transientHeap)
{
    m_renderer = renderer;
    m_commandBuffer = m_renderer->m_commandQueue->commandBuffer();
    return SLANG_OK;
}

void CommandBufferImpl::encodeRenderCommands(
    IRenderPassLayout* renderPass, IFramebuffer* framebuffer, IRenderCommandEncoder** outEncoder)
{
    if (!m_renderCommandEncoder)
    {
        m_renderCommandEncoder = new RenderCommandEncoder;
        m_renderCommandEncoder->init(this);
    }
    m_renderCommandEncoder->beginPass(renderPass, framebuffer);
    *outEncoder = m_renderCommandEncoder;
}

void CommandBufferImpl::encodeComputeCommands(IComputeCommandEncoder** outEncoder)
{
    if (!m_computeCommandEncoder)
    {
        m_computeCommandEncoder = new ComputeCommandEncoder;
        m_computeCommandEncoder->init(this);
    }
    *outEncoder = m_computeCommandEncoder;
}

void CommandBufferImpl::encodeResourceCommands(IResourceCommandEncoder** outEncoder)
{
    if (!m_resourceCommandEncoder)
    {
        m_resourceCommandEncoder = new ResourceCommandEncoder;
        m_resourceCommandEncoder->init(this);
    }
    *outEncoder = m_resourceCommandEncoder;
}

void CommandBufferImpl::encodeRayTracingCommands(IRayTracingCommandEncoder** outEncoder)
{
    if (!m_rayTracingCommandEncoder)
    {
        m_rayTracingCommandEncoder = new RayTracingCommandEncoder;
        m_rayTracingCommandEncoder->init(this);
    }
    *outEncoder = m_rayTracingCommandEncoder;
}

void CommandBufferImpl::close()
{
    //m_commandBuffer->commit();
}

Result CommandBufferImpl::getNativeHandle(InteropHandle* outHandle)
{
    return SLANG_E_NOT_IMPLEMENTED;
}

} // namespace metal
} // namespace gfx
