// immediate-renderer-base.h
#pragma once

// Provides shared implementation of public API objects for targets with
// an immediate mode execution context.

#include "render-graphics-common.h"

namespace gfx
{

enum class MapFlavor
{
    Unknown, ///< Unknown mapping type
    HostRead,
    HostWrite,
    WriteDiscard,
};

class ImmediateRendererBase : public GraphicsAPIRenderer
{
private:
    ComPtr<IPipelineState> m_currentPipelineState;

public:
    // Immediate commands to be implemented by each target.
    virtual SLANG_NO_THROW void SLANG_MCALL setPipelineState(IPipelineState* state) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL setDescriptorSet(
        PipelineType pipelineType,
        IPipelineLayout* layout,
        UInt index,
        IDescriptorSet* descriptorSet) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL setFramebuffer(IFramebuffer* frameBuffer) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL clearFrame(uint32_t colorBufferMask, bool clearDepth, bool clearStencil) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL setViewports(UInt count, const Viewport* viewports) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL
        setScissorRects(UInt count, const ScissorRect* scissors) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL setPrimitiveTopology(PrimitiveTopology topology) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL setVertexBuffers(
        UInt startSlot,
        UInt slotCount,
        IBufferResource* const* buffers,
        const UInt* strides,
        const UInt* offsets) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL
        setIndexBuffer(IBufferResource* buffer, Format indexFormat, UInt offset = 0) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL draw(UInt vertexCount, UInt startVertex = 0) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL
        drawIndexed(UInt indexCount, UInt startIndex = 0, UInt baseVertex = 0) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL setStencilReference(uint32_t referenceValue) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL dispatchCompute(int x, int y, int z) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL copyBuffer(
        IBufferResource* dst,
        size_t dstOffset,
        IBufferResource* src,
        size_t srcOffset,
        size_t size) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL submitGpuWork() = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL waitForGpu() = 0;
    virtual void* map(IBufferResource* buffer, MapFlavor flavor) = 0;
    virtual void unmap(IBufferResource* buffer) = 0;
    void bindRootShaderObject(PipelineType pipelineType, IShaderObject* shaderObject);

public:
    Slang::ComPtr<ICommandQueue> m_queue;
    uint32_t m_queueCreateCount = 0;

    ImmediateRendererBase();

    virtual SLANG_NO_THROW Result SLANG_MCALL
        createCommandQueue(const ICommandQueue::Desc& desc, ICommandQueue** outQueue) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createRenderPassLayout(
        const IRenderPassLayout::Desc& desc,
        IRenderPassLayout** outRenderPassLayout) override;

    void _setPipelineState(IPipelineState* state);

    void uploadBufferData(
        IBufferResource* dst,
        size_t offset,
        size_t size, void* data);

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL readBufferResource(
        IBufferResource* buffer,
        size_t offset,
        size_t size,
        ISlangBlob** outBlob) override;
};
}
