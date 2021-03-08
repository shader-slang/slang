#include "immediate-renderer-base.h"
#include "simple-render-pass-layout.h"
#include "command-writer.h"
#include "core/slang-basic.h"
#include "core/slang-blob.h"

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

class CommandBufferImpl : public ICommandBuffer, public RefObject
{
public:
    SLANG_REF_OBJECT_IUNKNOWN_ALL
    ICommandBuffer* getInterface(const Guid& guid)
    {
        if (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_ICommandBuffer)
            return static_cast<ICommandBuffer*>(this);
        return nullptr;
    }

public:
    CommandWriter m_writer;
    ImmediateRendererBase* m_renderer;

    void init(ImmediateRendererBase* renderer)
    {
        m_renderer = renderer;
    }

    void reset()
    { m_writer.clear();
    }

    class RenderCommandEncoderImpl
        : public IRenderCommandEncoder
    {
    public:
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL
            queryInterface(SlangUUID const& uuid, void** outObject) override
        {
            if (uuid == GfxGUID::IID_ISlangUnknown || uuid == GfxGUID::IID_IRenderCommandEncoder)
            {
                *outObject = static_cast<IRenderCommandEncoder*>(this);
                return SLANG_OK;
            }
            *outObject = nullptr;
            return SLANG_E_NO_INTERFACE;
        }
        virtual SLANG_NO_THROW uint32_t SLANG_MCALL addRef() { return 1; }
        virtual SLANG_NO_THROW uint32_t SLANG_MCALL release() { return 1; }

    public:
        CommandWriter* m_writer;
        virtual SLANG_NO_THROW void SLANG_MCALL endEncoding() override {}

        void init(CommandBufferImpl* cmdBuffer, SimpleRenderPassLayout* renderPass, IFramebuffer* framebuffer)
        {
            m_writer = &cmdBuffer->m_writer;

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

        virtual SLANG_NO_THROW void SLANG_MCALL setPipelineState(IPipelineState* state)
        {
            m_writer->setPipelineState(state);
        }

        virtual SLANG_NO_THROW void SLANG_MCALL
            bindRootShaderObject(IShaderObject* object)
        {
            m_writer->bindRootShaderObject(PipelineType::Graphics, object);
        }

        virtual SLANG_NO_THROW void SLANG_MCALL setDescriptorSet(
            IPipelineLayout* layout,
            UInt index,
            IDescriptorSet* descriptorSet)
        {
            m_writer->setDescriptorSet(PipelineType::Graphics, layout, index, descriptorSet);
        }

        virtual SLANG_NO_THROW void SLANG_MCALL
            setViewports(uint32_t count, const Viewport* viewports)
        {
            m_writer->setViewports(count, viewports);
        }
        virtual SLANG_NO_THROW void SLANG_MCALL
            setScissorRects(uint32_t count, const ScissorRect* scissors)
        {
            m_writer->setScissorRects(count, scissors);
        }
        virtual SLANG_NO_THROW void SLANG_MCALL setPrimitiveTopology(PrimitiveTopology topology)
        {
            m_writer->setPrimitiveTopology(topology);
        }
        virtual SLANG_NO_THROW void SLANG_MCALL setVertexBuffers(
            UInt startSlot,
            UInt slotCount,
            IBufferResource* const* buffers,
            const UInt* strides,
            const UInt* offsets)
        {
            m_writer->setVertexBuffers(startSlot, slotCount, buffers, strides, offsets);
        }

        virtual SLANG_NO_THROW void SLANG_MCALL
            setIndexBuffer(IBufferResource* buffer, Format indexFormat, UInt offset)
        {
            m_writer->setIndexBuffer(buffer, indexFormat, offset);
        }

        virtual SLANG_NO_THROW void SLANG_MCALL draw(UInt vertexCount, UInt startVertex)
        {
            m_writer->draw(vertexCount, startVertex);
        }

        virtual SLANG_NO_THROW void SLANG_MCALL
            drawIndexed(UInt indexCount, UInt startIndex, UInt baseVertex)
        {
            m_writer->drawIndexed(indexCount, startIndex, baseVertex);
        }

        virtual SLANG_NO_THROW void SLANG_MCALL setStencilReference(uint32_t referenceValue)
        {
            m_writer->setStencilReference(referenceValue);
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
    {
    public:
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL
            queryInterface(SlangUUID const& uuid, void** outObject) override
        {
            if (uuid == GfxGUID::IID_ISlangUnknown || uuid == GfxGUID::IID_IComputeCommandEncoder)
            {
                *outObject = static_cast<IComputeCommandEncoder*>(this);
                return SLANG_OK;
            }
            *outObject = nullptr;
            return SLANG_E_NO_INTERFACE;
        }
        virtual SLANG_NO_THROW uint32_t SLANG_MCALL addRef() { return 1; }
        virtual SLANG_NO_THROW uint32_t SLANG_MCALL release() { return 1; }

    public:
        CommandWriter* m_writer;

        virtual SLANG_NO_THROW void SLANG_MCALL endEncoding() override
        {
        }

        void init(CommandBufferImpl* cmdBuffer)
        {
            m_writer = &cmdBuffer->m_writer;
        }

        virtual SLANG_NO_THROW void SLANG_MCALL setPipelineState(IPipelineState* state) override
        {
            m_writer->setPipelineState(state);
        }
        virtual SLANG_NO_THROW void SLANG_MCALL
            bindRootShaderObject(IShaderObject* object) override
        {
            m_writer->bindRootShaderObject(PipelineType::Compute, object);
        }

        virtual SLANG_NO_THROW void SLANG_MCALL setDescriptorSet(
            IPipelineLayout* layout,
            UInt index,
            IDescriptorSet* descriptorSet) override
        {
            m_writer->setDescriptorSet(PipelineType::Compute, layout, index, descriptorSet);
        }

        virtual SLANG_NO_THROW void SLANG_MCALL dispatchCompute(int x, int y, int z) override
        {
            m_writer->dispatchCompute(x, y, z);
        }
    };

    ComputeCommandEncoderImpl m_computeCommandEncoder;
    virtual SLANG_NO_THROW void SLANG_MCALL
        encodeComputeCommands(IComputeCommandEncoder** outEncoder) override
    {
        m_computeCommandEncoder.init(this);
        *outEncoder = &m_computeCommandEncoder;
    }

    class ResourceCommandEncoderImpl
        : public IResourceCommandEncoder
    {
    public:
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL
            queryInterface(SlangUUID const& uuid, void** outObject) override
        {
            if (uuid == GfxGUID::IID_ISlangUnknown || uuid == GfxGUID::IID_IResourceCommandEncoder)
            {
                *outObject = static_cast<IResourceCommandEncoder*>(this);
                return SLANG_OK;
            }
            *outObject = nullptr;
            return SLANG_E_NO_INTERFACE;
        }
        virtual SLANG_NO_THROW uint32_t SLANG_MCALL addRef() { return 1; }
        virtual SLANG_NO_THROW uint32_t SLANG_MCALL release() { return 1; }

    public:
        CommandWriter* m_writer;

        void init(CommandBufferImpl* cmdBuffer)
        {
            m_writer = &cmdBuffer->m_writer;
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
            uploadBufferData(IBufferResource* dst, size_t offset, size_t size, void* data)
        {
            m_writer->uploadBufferData(dst, offset, size, data);
        }
    };

    ResourceCommandEncoderImpl m_resourceCommandEncoder;

    virtual SLANG_NO_THROW void SLANG_MCALL
        encodeResourceCommands(IResourceCommandEncoder** outEncoder) override
    {
        m_resourceCommandEncoder.init(this);
        *outEncoder = &m_resourceCommandEncoder;
    }

    virtual SLANG_NO_THROW void SLANG_MCALL close() override { }

    void execute()
    {
        for (auto& cmd : m_writer.m_commands)
        {
            auto name = cmd.name;
            switch (name)
            {
            case CommandName::SetPipelineState:
                m_renderer->_setPipelineState(m_writer.getObject<IPipelineState>(cmd.operands[0]));
                break;
            case CommandName::BindRootShaderObject:
                m_renderer->bindRootShaderObject(
                    (PipelineType)cmd.operands[0],
                    m_writer.getObject<IShaderObject>(cmd.operands[1]));
                break;
            case CommandName::SetDescriptorSet:
                m_renderer->setDescriptorSet(
                    (gfx::PipelineType)cmd.operands[0],
                    m_writer.getObject<IPipelineLayout>(cmd.operands[1]),
                    (UInt)cmd.operands[2],
                    m_writer.getObject<IDescriptorSet>(cmd.operands[3]));
                break;
            case CommandName::SetFramebuffer:
                m_renderer->setFramebuffer(m_writer.getObject<IFramebuffer>(cmd.operands[0]));
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
                            m_writer.getObject<IBufferResource>(cmd.operands[2] + i));
                    }
                    m_renderer->setVertexBuffers(
                        (UInt)cmd.operands[0],
                        (UInt)cmd.operands[1],
                        bufferResources.getArrayView().getBuffer(),
                        m_writer.getData<UInt>(cmd.operands[3]),
                        m_writer.getData<UInt>(cmd.operands[4]));
                }
                break;
            case CommandName::SetIndexBuffer:
                m_renderer->setIndexBuffer(
                    m_writer.getObject<IBufferResource>(cmd.operands[0]),
                    (Format)cmd.operands[1],
                    (UInt)cmd.operands[2]);
                break;
            case CommandName::Draw:
                m_renderer->draw((UInt)cmd.operands[0], (UInt)cmd.operands[1]);
                break;
            case CommandName::DrawIndexed:
                m_renderer->drawIndexed(
                    (UInt)cmd.operands[0], (UInt)cmd.operands[1], (UInt)cmd.operands[2]);
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
                    m_writer.getObject<IBufferResource>(cmd.operands[0]),
                    cmd.operands[1],
                    cmd.operands[2],
                    m_writer.getData<uint8_t>(cmd.operands[3]));
                break;
            case CommandName::CopyBuffer:
                m_renderer->copyBuffer(
                    m_writer.getObject<IBufferResource>(cmd.operands[0]),
                    cmd.operands[1],
                    m_writer.getObject<IBufferResource>(cmd.operands[2]),
                    cmd.operands[3],
                    cmd.operands[4]);
                break;
            default:
                assert(!"unknown command");
                break;
            }
        }
        m_writer.clear();
    }
};

class CommandQueueImpl
    : public ICommandQueue
    , public RefObject
{
public:
    SLANG_REF_OBJECT_IUNKNOWN_ALL
    ICommandQueue* getInterface(const Guid& guid)
    {
        if (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_ICommandQueue)
            return static_cast<ICommandQueue*>(this);
        return nullptr;
    }

public:
    ImmediateRendererBase* m_renderer;
    ICommandQueue::Desc m_desc;

    CommandQueueImpl(ImmediateRendererBase* renderer)
        : m_renderer(renderer)
    {
        m_desc.type = ICommandQueue::QueueType::Graphics;
    }

    ~CommandQueueImpl()
    {
        m_renderer->m_queueCreateCount--;
    }

    virtual SLANG_NO_THROW const Desc& SLANG_MCALL getDesc() override { return m_desc; }

    virtual SLANG_NO_THROW Result SLANG_MCALL
        createCommandBuffer(ICommandBuffer** outCommandBuffer) override
    {
        RefPtr<CommandBufferImpl> newCmdBuffer = new CommandBufferImpl();
        newCmdBuffer->init(m_renderer);
        *outCommandBuffer = newCmdBuffer.detach();
        return SLANG_OK;
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
        executeCommandBuffers(uint32_t count, ICommandBuffer* const* commandBuffers) override
    {
        for (uint32_t i = 0; i < count; i++)
        {
            static_cast<CommandBufferImpl*>(commandBuffers[i])->execute();
        }
    }

    virtual SLANG_NO_THROW void SLANG_MCALL wait() override
    {
        m_renderer->waitForGpu();
    }
};
}


ImmediateRendererBase::ImmediateRendererBase() {
    m_queue = new CommandQueueImpl(this);
}

void ImmediateRendererBase::bindRootShaderObject(PipelineType pipelineType, IShaderObject* shaderObject)
{
    class ImmediateCommandEncoder : public GraphicsComputeCommandEncoderBase
    {
    public:
        virtual SLANG_NO_THROW void SLANG_MCALL setDescriptorSetImpl(
            PipelineType pipelineType,
            IPipelineLayout* layout,
            UInt index,
            IDescriptorSet* descriptorSet) override
        {
            auto renderer = static_cast<ImmediateRendererBase*>(m_rendererBase);
            renderer->setDescriptorSet(pipelineType, layout, index, descriptorSet);
        }

        virtual SLANG_NO_THROW void SLANG_MCALL uploadBufferDataImpl(
            IBufferResource* buffer,
            size_t offset,
            size_t size,
            void* data) override
        {
            auto renderer = static_cast<ImmediateRendererBase*>(m_rendererBase);
            renderer->uploadBufferData(buffer, offset, size, data);
        }
    };
    ImmediateCommandEncoder encoder;
    encoder.m_rendererBase = this;
    encoder.m_currentPipeline = static_cast<PipelineStateBase*>(m_currentPipelineState.get());
    encoder.bindRootShaderObjectImpl(pipelineType, shaderObject);
    _setPipelineState(encoder.m_currentPipeline);
}

SLANG_NO_THROW Result SLANG_MCALL ImmediateRendererBase::createCommandQueue(
    const ICommandQueue::Desc& desc,
    ICommandQueue** outQueue)
{
    SLANG_UNUSED(desc);
    // Only one queue is supported.
    if (m_queueCreateCount != 0)
        return SLANG_FAIL;
    *outQueue = m_queue.get();
    m_queue->addRef();
    return SLANG_OK;
}

SLANG_NO_THROW Result SLANG_MCALL ImmediateRendererBase::createRenderPassLayout(
    const IRenderPassLayout::Desc& desc,
    IRenderPassLayout** outRenderPassLayout)
{
    RefPtr<SimpleRenderPassLayout> renderPass = new SimpleRenderPassLayout();
    renderPass->init(desc);
    *outRenderPassLayout = renderPass.detach();
    return SLANG_OK;
}

void ImmediateRendererBase::_setPipelineState(IPipelineState* state)
{
    PipelineStateBase* pipelineImpl = static_cast<PipelineStateBase*>(state);
    if (!pipelineImpl->isSpecializable)
    {
        setPipelineState(state);
    }
    m_currentPipelineState = state;
}

void ImmediateRendererBase::uploadBufferData(
    IBufferResource* dst,
    size_t offset,
    size_t size,
    void* data)
{
    auto buffer = map(dst, gfx::MapFlavor::WriteDiscard);
    memcpy((uint8_t*)buffer + offset, data, size);
    unmap(dst);
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
    unmap(buffer);
    *outBlob = blob.detach();
    return SLANG_OK;
}

}
