#include "immediate-renderer-base.h"
#include "simple-render-pass-layout.h"
#include "simple-transient-resource-heap.h"
#include "command-writer.h"
#include "core/slang-basic.h"
#include "core/slang-blob.h"
#include "command-encoder-com-forward.h"

namespace gfx
{
using Slang::RefPtr;
using Slang::List;
using Slang::ShortList;
using Slang::ListBlob;
using Slang::Index;
using Slang::RefObject;
using Slang::ComPtr;
using Slang::Guid;

namespace
{

class CommandBufferImpl : public ICommandBuffer, public Slang::ComObject
{
public:
    SLANG_COM_OBJECT_IUNKNOWN_ALL
    ICommandBuffer* getInterface(const Guid& guid)
    {
        if (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_ICommandBuffer)
            return static_cast<ICommandBuffer*>(this);
        return nullptr;
    }

public:
    CommandWriter m_writer;
    bool m_hasWriteTimestamps = false;
    RefPtr<ImmediateRendererBase> m_renderer;
    RefPtr<ShaderObjectBase> m_rootShaderObject;
    TransientResourceHeapBase* m_transientHeap;

    void init(ImmediateRendererBase* renderer, TransientResourceHeapBase* transientHeap)
    {
        m_renderer = renderer;
        m_transientHeap = transientHeap;
    }

    void reset()
    {
        m_writer.clear();
    }

    class ResourceCommandEncoderImpl : public IResourceCommandEncoder
    {
    public:
        CommandWriter* m_writer;
        CommandBufferImpl* m_commandBuffer;
        void init(CommandBufferImpl* cmdBuffer)
        {
            m_writer = &cmdBuffer->m_writer;
            m_commandBuffer = cmdBuffer;
        }

        virtual SLANG_NO_THROW void SLANG_MCALL endEncoding() override {}
        virtual SLANG_NO_THROW void SLANG_MCALL copyBuffer(
            IBufferResource* dst,
            size_t dstOffset,
            IBufferResource* src,
            size_t srcOffset,
            size_t size) override
        {
            m_writer->copyBuffer(dst, dstOffset, src, srcOffset, size);
        }

        virtual SLANG_NO_THROW void SLANG_MCALL
            uploadBufferData(IBufferResource* dst, size_t offset, size_t size, void* data) override
        {
            m_writer->uploadBufferData(dst, offset, size, data);
        }

        virtual SLANG_NO_THROW void SLANG_MCALL
            writeTimestamp(IQueryPool* pool, SlangInt index) override
        {
            m_writer->writeTimestamp(pool, index);
        }

        virtual SLANG_NO_THROW void SLANG_MCALL textureBarrier(
            size_t count,
            ITextureResource* const* textures,
            ResourceState src,
            ResourceState dst) override
        {}

        virtual SLANG_NO_THROW void SLANG_MCALL bufferBarrier(
            size_t count,
            IBufferResource* const* buffers,
            ResourceState src,
            ResourceState dst) override
        {}

        virtual SLANG_NO_THROW void SLANG_MCALL copyTexture(
            ITextureResource* dst,
            ResourceState dstState,
            SubresourceRange dstSubresource,
            ITextureResource::Offset3D dstOffset,
            ITextureResource* src,
            ResourceState srcState,
            SubresourceRange srcSubresource,
            ITextureResource::Offset3D srcOffset,
            ITextureResource::Size extent) override
        {
            SLANG_UNUSED(dst);
            SLANG_UNUSED(dstState);
            SLANG_UNUSED(dstSubresource);
            SLANG_UNUSED(dstOffset);
            SLANG_UNUSED(src);
            SLANG_UNUSED(srcState);
            SLANG_UNUSED(srcSubresource);
            SLANG_UNUSED(srcOffset);
            SLANG_UNUSED(extent);
            SLANG_UNIMPLEMENTED_X("copyTexture");
        }

        virtual SLANG_NO_THROW void SLANG_MCALL uploadTextureData(
            ITextureResource* dst,
            SubresourceRange subResourceRange,
            ITextureResource::Offset3D offset,
            ITextureResource::Size extend,
            ITextureResource::SubresourceData* subResourceData,
            size_t subResourceDataCount) override
        {
            SLANG_UNUSED(dst);
            SLANG_UNUSED(subResourceRange);
            SLANG_UNUSED(offset);
            SLANG_UNUSED(extend);
            SLANG_UNUSED(subResourceData);
            SLANG_UNUSED(subResourceDataCount);
            SLANG_UNIMPLEMENTED_X("uploadTextureData");
        }

        virtual SLANG_NO_THROW void SLANG_MCALL clearResourceView(
            IResourceView* view,
            ClearValue* clearValue,
            ClearResourceViewFlags::Enum flags) override
        {
            SLANG_UNUSED(view);
            SLANG_UNUSED(clearValue);
            SLANG_UNUSED(flags);
            SLANG_UNIMPLEMENTED_X("clearResourceView");
        }

        virtual SLANG_NO_THROW void SLANG_MCALL resolveResource(
            ITextureResource* source,
            ResourceState sourceState,
            SubresourceRange sourceRange,
            ITextureResource* dest,
            ResourceState destState,
            SubresourceRange destRange) override
        {
            SLANG_UNUSED(source);
            SLANG_UNUSED(sourceState);
            SLANG_UNUSED(sourceRange);
            SLANG_UNUSED(dest);
            SLANG_UNUSED(destState);
            SLANG_UNUSED(destRange);
            SLANG_UNIMPLEMENTED_X("resolveResource");
        }

        virtual SLANG_NO_THROW void SLANG_MCALL resolveQuery(
            IQueryPool* queryPool,
            uint32_t index,
            uint32_t count,
            IBufferResource* buffer,
            uint64_t offset) override
        {
            SLANG_UNUSED(queryPool);
            SLANG_UNUSED(index);
            SLANG_UNUSED(count);
            SLANG_UNUSED(buffer);
            SLANG_UNUSED(offset);
            SLANG_UNIMPLEMENTED_X("resolveQuery");
        }

        virtual SLANG_NO_THROW void SLANG_MCALL copyTextureToBuffer(
            IBufferResource* dst,
            size_t dstOffset,
            size_t dstSize,
            size_t dstRowStride,
            ITextureResource* src,
            ResourceState srcState,
            SubresourceRange srcSubresource,
            ITextureResource::Offset3D srcOffset,
            ITextureResource::Size extent) override
        {
            SLANG_UNUSED(dst);
            SLANG_UNUSED(dstOffset);
            SLANG_UNUSED(dstSize);
            SLANG_UNUSED(dstRowStride);
            SLANG_UNUSED(src);
            SLANG_UNUSED(srcState);
            SLANG_UNUSED(srcSubresource);
            SLANG_UNUSED(srcOffset);
            SLANG_UNUSED(extent);
            SLANG_UNIMPLEMENTED_X("copyTextureToBuffer");
        }

        virtual SLANG_NO_THROW void SLANG_MCALL textureSubresourceBarrier(
            ITextureResource* texture,
            SubresourceRange subresourceRange,
            ResourceState src,
            ResourceState dst) override
        {
            SLANG_UNUSED(texture);
            SLANG_UNUSED(subresourceRange);
            SLANG_UNUSED(src);
            SLANG_UNUSED(dst);
            SLANG_UNIMPLEMENTED_X("textureSubresourceBarrier");
        }

        virtual SLANG_NO_THROW void SLANG_MCALL
            beginDebugEvent(const char* name, float rgbColor[3]) override
        {
            SLANG_UNUSED(name);
            SLANG_UNUSED(rgbColor);
        }
        virtual SLANG_NO_THROW void SLANG_MCALL endDebugEvent() override
        {
        }
    };

    ResourceCommandEncoderImpl m_resourceCommandEncoder;

    virtual SLANG_NO_THROW void SLANG_MCALL
        encodeResourceCommands(IResourceCommandEncoder** outEncoder) override
    {
        m_resourceCommandEncoder.init(this);
        *outEncoder = &m_resourceCommandEncoder;
    }

    class RenderCommandEncoderImpl
        : public IRenderCommandEncoder
        , public ResourceCommandEncoderImpl
    {
    public:
        SLANG_GFX_FORWARD_RESOURCE_COMMAND_ENCODER_IMPL(ResourceCommandEncoderImpl)
    public:
        virtual SLANG_NO_THROW void SLANG_MCALL endEncoding() override {}

        void init(CommandBufferImpl* cmdBuffer, SimpleRenderPassLayout* renderPass, IFramebuffer* framebuffer)
        {
            ResourceCommandEncoderImpl::init(cmdBuffer);

            // Encode clear commands.
            m_writer->setFramebuffer(framebuffer);
            uint32_t clearMask = 0;
            for (Index i = 0; i < renderPass->m_renderTargetAccesses.getCount(); i++)
            {
                auto& access = renderPass->m_renderTargetAccesses[i];
                // Clear.
                if (access.loadOp == IRenderPassLayout::AttachmentLoadOp::Clear)
                {
                    clearMask |= (1 << (uint32_t)i);
                }
            }
            bool clearDepth = false;
            bool clearStencil = false;
            if (renderPass->m_hasDepthStencil)
            {
                // Clear.
                if (renderPass->m_depthStencilAccess.loadOp ==
                    IRenderPassLayout::AttachmentLoadOp::Clear)
                {
                    clearDepth = true;
                }
                if (renderPass->m_depthStencilAccess.stencilLoadOp ==
                    IRenderPassLayout::AttachmentLoadOp::Clear)
                {
                    clearStencil = true;
                }
            }
            m_writer->clearFrame(clearMask, clearDepth, clearStencil);
        }

        virtual SLANG_NO_THROW Result SLANG_MCALL
            bindPipeline(IPipelineState* state, IShaderObject** outRootObject) override
        {
            m_writer->setPipelineState(state);
            auto stateImpl = static_cast<PipelineStateBase*>(state);
            SLANG_RETURN_ON_FAIL(m_commandBuffer->m_renderer->createRootShaderObject(
                stateImpl->m_program, m_commandBuffer->m_rootShaderObject.writeRef()));
            *outRootObject = m_commandBuffer->m_rootShaderObject.Ptr();
            return SLANG_OK;
        }

        virtual SLANG_NO_THROW Result SLANG_MCALL
            bindPipelineWithRootObject(IPipelineState* state, IShaderObject* rootObject) override
        {
            m_writer->setPipelineState(state);
            auto stateImpl = static_cast<PipelineStateBase*>(state);
            SLANG_RETURN_ON_FAIL(m_commandBuffer->m_renderer->createRootShaderObject(
                stateImpl->m_program, m_commandBuffer->m_rootShaderObject.writeRef()));
            m_commandBuffer->m_rootShaderObject->copyFrom(rootObject, m_commandBuffer->m_transientHeap);
            return SLANG_OK;
        }

        virtual SLANG_NO_THROW void SLANG_MCALL
            setViewports(uint32_t count, const Viewport* viewports) override
        {
            m_writer->setViewports(count, viewports);
        }
        virtual SLANG_NO_THROW void SLANG_MCALL
            setScissorRects(uint32_t count, const ScissorRect* scissors) override
        {
            m_writer->setScissorRects(count, scissors);
        }
        virtual SLANG_NO_THROW void SLANG_MCALL setPrimitiveTopology(PrimitiveTopology topology) override
        {
            m_writer->setPrimitiveTopology(topology);
        }
        virtual SLANG_NO_THROW void SLANG_MCALL setVertexBuffers(
            uint32_t startSlot,
            uint32_t slotCount,
            IBufferResource* const* buffers,
            const uint32_t* offsets) override
        {
            m_writer->setVertexBuffers(startSlot, slotCount, buffers, offsets);
        }

        virtual SLANG_NO_THROW void SLANG_MCALL
            setIndexBuffer(IBufferResource* buffer, Format indexFormat, uint32_t offset) override
        {
            m_writer->setIndexBuffer(buffer, indexFormat, offset);
        }

        virtual SLANG_NO_THROW void SLANG_MCALL
            draw(uint32_t vertexCount, uint32_t startVertex) override
        {
            m_writer->bindRootShaderObject(m_commandBuffer->m_rootShaderObject);
            m_writer->draw(vertexCount, startVertex);
        }

        virtual SLANG_NO_THROW void SLANG_MCALL
            drawIndexed(uint32_t indexCount, uint32_t startIndex, uint32_t baseVertex) override
        {
            m_writer->bindRootShaderObject(m_commandBuffer->m_rootShaderObject);
            m_writer->drawIndexed(indexCount, startIndex, baseVertex);
        }

        virtual SLANG_NO_THROW void SLANG_MCALL setStencilReference(uint32_t referenceValue) override
        {
            m_writer->setStencilReference(referenceValue);
        }

        virtual SLANG_NO_THROW void SLANG_MCALL drawIndirect(
            uint32_t maxDrawCount,
            IBufferResource* argBuffer,
            uint64_t argOffset,
            IBufferResource* countBuffer,
            uint64_t countOffset) override
        {
            SLANG_UNUSED(maxDrawCount);
            SLANG_UNUSED(argBuffer);
            SLANG_UNUSED(argOffset);
            SLANG_UNUSED(countBuffer);
            SLANG_UNUSED(countOffset);
            SLANG_UNIMPLEMENTED_X("ImmediateRenderBase::drawIndirect");
        }

        virtual SLANG_NO_THROW void SLANG_MCALL drawIndexedIndirect(
            uint32_t maxDrawCount,
            IBufferResource* argBuffer,
            uint64_t argOffset,
            IBufferResource* countBuffer,
            uint64_t countOffset) override
        {
            SLANG_UNUSED(maxDrawCount);
            SLANG_UNUSED(argBuffer);
            SLANG_UNUSED(argOffset);
            SLANG_UNUSED(countBuffer);
            SLANG_UNUSED(countOffset);
            SLANG_UNIMPLEMENTED_X("ImmediateRenderBase::drawIndirect");
        }

        virtual SLANG_NO_THROW Result SLANG_MCALL setSamplePositions(
            uint32_t samplesPerPixel,
            uint32_t pixelCount,
            const SamplePosition* samplePositions) override
        {
            SLANG_UNUSED(samplesPerPixel);
            SLANG_UNUSED(pixelCount);
            SLANG_UNUSED(samplePositions);
            return SLANG_E_NOT_AVAILABLE;
        }

        virtual SLANG_NO_THROW void SLANG_MCALL drawInstanced(
            uint32_t vertexCount,
            uint32_t instanceCount,
            uint32_t startVertex,
            uint32_t startInstanceLocation) override
        {
            m_writer->bindRootShaderObject(m_commandBuffer->m_rootShaderObject);
            m_writer->drawInstanced(vertexCount, instanceCount, startVertex, startInstanceLocation);
        }

        virtual SLANG_NO_THROW void SLANG_MCALL drawIndexedInstanced(
            uint32_t indexCount,
            uint32_t instanceCount,
            uint32_t startIndexLocation,
            int32_t baseVertexLocation,
            uint32_t startInstanceLocation) override
        {
            m_writer->bindRootShaderObject(m_commandBuffer->m_rootShaderObject);
            m_writer->drawIndexedInstanced(indexCount, instanceCount, startIndexLocation, baseVertexLocation, startInstanceLocation);
        }
    };

    RenderCommandEncoderImpl m_renderCommandEncoder;
    virtual SLANG_NO_THROW void SLANG_MCALL encodeRenderCommands(
        IRenderPassLayout* renderPass,
        IFramebuffer* framebuffer,
        IRenderCommandEncoder** outEncoder) override
    {
        m_renderCommandEncoder.init(
            this,
            static_cast<SimpleRenderPassLayout*>(renderPass),
            framebuffer);
        *outEncoder = &m_renderCommandEncoder;
    }

    class ComputeCommandEncoderImpl
        : public IComputeCommandEncoder
        , public ResourceCommandEncoderImpl
    {
    public:
        SLANG_GFX_FORWARD_RESOURCE_COMMAND_ENCODER_IMPL(ResourceCommandEncoderImpl)
    public:
        virtual SLANG_NO_THROW void SLANG_MCALL endEncoding() override
        {
        }

        virtual SLANG_NO_THROW Result SLANG_MCALL
            bindPipeline(IPipelineState* state, IShaderObject** outRootObject) override
        {
            m_writer->setPipelineState(state);
            auto stateImpl = static_cast<PipelineStateBase*>(state);
            SLANG_RETURN_ON_FAIL(m_commandBuffer->m_renderer->createRootShaderObject(
                stateImpl->m_program, m_commandBuffer->m_rootShaderObject.writeRef()));
            *outRootObject = m_commandBuffer->m_rootShaderObject.Ptr();
            return SLANG_OK;
        }

        virtual SLANG_NO_THROW Result SLANG_MCALL
            bindPipelineWithRootObject(IPipelineState* state, IShaderObject* rootObject) override
        {
            m_writer->setPipelineState(state);
            auto stateImpl = static_cast<PipelineStateBase*>(state);
            SLANG_RETURN_ON_FAIL(m_commandBuffer->m_renderer->createRootShaderObject(
                stateImpl->m_program, m_commandBuffer->m_rootShaderObject.writeRef()));
            m_commandBuffer->m_rootShaderObject->copyFrom(
                rootObject, m_commandBuffer->m_transientHeap);
            return SLANG_OK;
        }

        virtual SLANG_NO_THROW void SLANG_MCALL dispatchCompute(int x, int y, int z) override
        {
            m_writer->bindRootShaderObject(m_commandBuffer->m_rootShaderObject);
            m_writer->dispatchCompute(x, y, z);
        }

        virtual SLANG_NO_THROW void SLANG_MCALL dispatchComputeIndirect(IBufferResource* argBuffer, uint64_t offset) override
        {
            SLANG_UNIMPLEMENTED_X("ImmediateRenderBase::dispatchComputeIndirect");
        }
    };

    ComputeCommandEncoderImpl m_computeCommandEncoder;
    virtual SLANG_NO_THROW void SLANG_MCALL
        encodeComputeCommands(IComputeCommandEncoder** outEncoder) override
    {
        m_computeCommandEncoder.init(this);
        *outEncoder = &m_computeCommandEncoder;
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
        encodeRayTracingCommands(IRayTracingCommandEncoder** outEncoder) override
    {
        *outEncoder = nullptr;
    }

    virtual SLANG_NO_THROW void SLANG_MCALL close() override { }

    virtual SLANG_NO_THROW Result SLANG_MCALL getNativeHandle(InteropHandle* outHandle) override
    {
        return SLANG_FAIL;
    }

    void execute()
    {
        for (auto& cmd : m_writer.m_commands)
        {
            auto name = cmd.name;
            switch (name)
            {
            case CommandName::SetPipelineState:
                m_renderer->setPipelineState(m_writer.getObject<PipelineStateBase>(cmd.operands[0]));
                break;
            case CommandName::BindRootShaderObject:
                m_renderer->bindRootShaderObject(m_writer.getObject<ShaderObjectBase>(cmd.operands[0]));
                break;
            case CommandName::SetFramebuffer:
                m_renderer->setFramebuffer(m_writer.getObject<FramebufferBase>(cmd.operands[0]));
                break;
            case CommandName::ClearFrame:
                m_renderer->clearFrame(
                    cmd.operands[0], (cmd.operands[1] != 0), (cmd.operands[2] != 0));
                break;
            case CommandName::SetViewports:
                m_renderer->setViewports(
                    (UInt)cmd.operands[0], m_writer.getData<Viewport>(cmd.operands[1]));
                break;
            case CommandName::SetScissorRects:
                m_renderer->setScissorRects(
                    (UInt)cmd.operands[0], m_writer.getData<ScissorRect>(cmd.operands[1]));
                break;
            case CommandName::SetPrimitiveTopology:
                m_renderer->setPrimitiveTopology((PrimitiveTopology)cmd.operands[0]);
                break;
            case CommandName::SetVertexBuffers:
                {
                    ShortList<IBufferResource*> bufferResources;
                    for (uint32_t i = 0; i < cmd.operands[1]; i++)
                    {
                        bufferResources.add(
                            m_writer.getObject<BufferResource>(cmd.operands[2] + i));
                    }
                    m_renderer->setVertexBuffers(
                        cmd.operands[0],
                        cmd.operands[1],
                        bufferResources.getArrayView().getBuffer(),
                        m_writer.getData<uint32_t>(cmd.operands[3]));
                }
                break;
            case CommandName::SetIndexBuffer:
                m_renderer->setIndexBuffer(
                    m_writer.getObject<BufferResource>(cmd.operands[0]),
                    (Format)cmd.operands[1],
                    (UInt)cmd.operands[2]);
                break;
            case CommandName::Draw:
                m_renderer->draw(cmd.operands[0], cmd.operands[1]);
                break;
            case CommandName::DrawIndexed:
                m_renderer->drawIndexed(
                    cmd.operands[0], cmd.operands[1], cmd.operands[2]);
                break;
            case CommandName::DrawInstanced:
                m_renderer->drawInstanced(
                    cmd.operands[0], cmd.operands[1], cmd.operands[2], cmd.operands[3]);
                break;
            case CommandName::DrawIndexedInstanced:
                m_renderer->drawIndexedInstanced(
                    cmd.operands[0], cmd.operands[1], cmd.operands[2], cmd.operands[3], cmd.operands[4]);
                break;
            case CommandName::SetStencilReference:
                m_renderer->setStencilReference(cmd.operands[0]);
                break;
            case CommandName::DispatchCompute:
                m_renderer->dispatchCompute(
                    int(cmd.operands[0]), int(cmd.operands[1]), int(cmd.operands[2]));
                break;
            case CommandName::UploadBufferData:
                m_renderer->uploadBufferData(
                    m_writer.getObject<BufferResource>(cmd.operands[0]),
                    cmd.operands[1],
                    cmd.operands[2],
                    m_writer.getData<uint8_t>(cmd.operands[3]));
                break;
            case CommandName::CopyBuffer:
                m_renderer->copyBuffer(
                    m_writer.getObject<BufferResource>(cmd.operands[0]),
                    cmd.operands[1],
                    m_writer.getObject<BufferResource>(cmd.operands[2]),
                    cmd.operands[3],
                    cmd.operands[4]);
                break;
            case CommandName::WriteTimestamp:
                m_renderer->writeTimestamp(m_writer.getObject<QueryPoolBase>(cmd.operands[0]), (SlangInt)cmd.operands[1]);
                break;
            default:
                assert(!"unknown command");
                break;
            }
        }
        m_writer.clear();
    }
};

class CommandQueueImpl : public ImmediateCommandQueueBase
{
public:
    ICommandQueue::Desc m_desc;

    ImmediateRendererBase* getRenderer()
    {
        return static_cast<ImmediateRendererBase*>(m_renderer.get());
    }

    CommandQueueImpl(ImmediateRendererBase* renderer)
    {
        // Don't establish strong reference to `Device` at start, because
        // there will be only one instance of command queue and it will be
        // owned by `Device`. We should establish a strong reference only
        // when there are external references to the command queue.
        m_renderer.setWeakReference(renderer);
        m_desc.type = ICommandQueue::QueueType::Graphics;
    }

    ~CommandQueueImpl() { getRenderer()->m_queueCreateCount--; }

    virtual SLANG_NO_THROW const Desc& SLANG_MCALL getDesc() override { return m_desc; }

    virtual SLANG_NO_THROW void SLANG_MCALL executeCommandBuffers(
        uint32_t count, ICommandBuffer* const* commandBuffers, IFence* fence, uint64_t valueToSignal) override
    {
        // TODO: implement fence signal.
        assert(fence == nullptr);

        CommandBufferInfo info = {};
        for (uint32_t i = 0; i < count; i++)
        {
            info.hasWriteTimestamps |= static_cast<CommandBufferImpl*>(commandBuffers[i])->m_writer.m_hasWriteTimestamps;
        }
        static_cast<ImmediateRendererBase*>(m_renderer.get())->beginCommandBuffer(info);
        for (uint32_t i = 0; i < count; i++)
        {
            static_cast<CommandBufferImpl*>(commandBuffers[i])->execute();
        }
        static_cast<ImmediateRendererBase*>(m_renderer.get())->endCommandBuffer(info);
    }

    virtual SLANG_NO_THROW void SLANG_MCALL waitOnHost() override { getRenderer()->waitForGpu(); }

    virtual SLANG_NO_THROW Result SLANG_MCALL waitForFenceValuesOnDevice(
        uint32_t fenceCount, IFence** fences, uint64_t* waitValues) override
    {
        return SLANG_FAIL;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL getNativeHandle(InteropHandle* outHandle) override
    {
        return getRenderer()->m_queue->getNativeHandle(outHandle);
    }
};

using TransientResourceHeapImpl =
    SimpleTransientResourceHeap<ImmediateRendererBase, CommandBufferImpl>;

}

ImmediateRendererBase::ImmediateRendererBase()
{
    m_queue = new CommandQueueImpl(this);
}

SLANG_NO_THROW Result SLANG_MCALL ImmediateRendererBase::createTransientResourceHeap(
    const ITransientResourceHeap::Desc& desc,
    ITransientResourceHeap** outHeap)
{
    RefPtr<TransientResourceHeapImpl> result = new TransientResourceHeapImpl();
    SLANG_RETURN_ON_FAIL(result->init(this, desc));
    returnComPtr(outHeap, result);
    return SLANG_OK;
}

SLANG_NO_THROW Result SLANG_MCALL ImmediateRendererBase::createCommandQueue(
    const ICommandQueue::Desc& desc,
    ICommandQueue** outQueue)
{
    SLANG_UNUSED(desc);
    // Only one queue is supported.
    if (m_queueCreateCount != 0)
        return SLANG_FAIL;
    m_queue->establishStrongReferenceToDevice();
    returnComPtr(outQueue, m_queue);
    return SLANG_OK;
}

SLANG_NO_THROW Result SLANG_MCALL ImmediateRendererBase::createRenderPassLayout(
    const IRenderPassLayout::Desc& desc,
    IRenderPassLayout** outRenderPassLayout)
{
    RefPtr<SimpleRenderPassLayout> renderPass = new SimpleRenderPassLayout();
    renderPass->init(desc);
    returnComPtr(outRenderPassLayout, renderPass);
    return SLANG_OK;
}

void ImmediateRendererBase::uploadBufferData(
    IBufferResource* dst,
    size_t offset,
    size_t size,
    void* data)
{
    auto buffer = map(dst, gfx::MapFlavor::WriteDiscard);
    memcpy((uint8_t*)buffer + offset, data, size);
    unmap(dst, offset, size);
}

SLANG_NO_THROW SlangResult SLANG_MCALL ImmediateRendererBase::readBufferResource(
    IBufferResource* buffer,
    size_t offset,
    size_t size,
    ISlangBlob** outBlob)
{
    RefPtr<ListBlob> blob = new ListBlob();
    blob->m_data.setCount((Index)size);
    auto content = (uint8_t*)map(buffer, gfx::MapFlavor::HostRead);
    if (!content)
        return SLANG_FAIL;
    memcpy(blob->m_data.getBuffer(), content + offset, size);
    unmap(buffer, offset, size);
    returnComPtr(outBlob, blob);
    return SLANG_OK;
}

}
