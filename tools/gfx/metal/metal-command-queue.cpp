// metal-command-queue.cpp
#include "metal-command-queue.h"

#include "metal-command-buffer.h"
#include "metal-fence.h"

namespace gfx
{

using namespace Slang;

namespace metal 
{

ICommandQueue* CommandQueueImpl::getInterface(const Guid& guid)
{
    if (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_ICommandQueue)
        return static_cast<ICommandQueue*>(this);
    return nullptr;
}

CommandQueueImpl::~CommandQueueImpl()
{
}

void CommandQueueImpl::init(DeviceImpl* renderer)
{
    m_renderer = renderer;

    MTL::Device* device = m_renderer->m_device;
    m_commandQueue = device->newCommandQueue(8);
}

void CommandQueueImpl::waitOnHost()
{
}

Result CommandQueueImpl::getNativeHandle(InteropHandle* outHandle)
{
    outHandle->api = InteropHandleAPI::Metal;
    outHandle->handleValue = reinterpret_cast<intptr_t>(m_commandQueue);
    return SLANG_OK;
}

const CommandQueueImpl::Desc& CommandQueueImpl::getDesc() { return m_desc; }

Result CommandQueueImpl::waitForFenceValuesOnDevice(
    GfxCount fenceCount, IFence** fences, uint64_t* waitValues)
{
    return SLANG_OK;
}

void CommandQueueImpl::queueSubmitImpl(
    uint32_t count, ICommandBuffer* const* commandBuffers, IFence* fence, uint64_t valueToSignal)
{
    for (uint32_t i = 0; i < count; ++i)
    {
        CommandBufferImpl* cmdBufImpl = static_cast<CommandBufferImpl*>(commandBuffers[i]);
        cmdBufImpl->m_commandBuffer->presentDrawable(m_renderer->m_drawable);
        cmdBufImpl->m_commandBuffer->commit();
    }
}

void CommandQueueImpl::executeCommandBuffers(
    GfxCount count, ICommandBuffer* const* commandBuffers, IFence* fence, uint64_t valueToSignal)
{
    if (count == 0 && fence == nullptr)
    {
        return;
    }
    queueSubmitImpl(count, commandBuffers, fence, valueToSignal);
}

} // namespace metal
} // namespace gfx
