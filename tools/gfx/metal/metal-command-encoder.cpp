// metal-command-encoder.cpp
#include "metal-command-encoder.h"

#include "metal-buffer.h"
#include "metal-command-buffer.h"
#include "metal-query.h"
#include "metal-render-pass.h"
#include "metal-resource-views.h"
#include "metal-shader-object.h"
#include "metal-shader-program.h"
#include "metal-shader-table.h"
#include "metal-texture.h"
#include "metal-util.h"

#include "metal-helper-functions.h"

namespace gfx
{

using namespace Slang;

namespace metal
{

void PipelineCommandEncoder::init(CommandBufferImpl* commandBuffer)
{
    m_commandBuffer = commandBuffer;
    m_metalCommandBuffer = m_commandBuffer->m_commandBuffer.get();
}

void PipelineCommandEncoder::endEncodingImpl()
{
    m_commandBuffer->endMetalCommandEncoder();
}

Result PipelineCommandEncoder::setPipelineStateImpl(IPipelineState* state, IShaderObject** outRootObject)
{
    m_currentPipeline = static_cast<PipelineStateImpl*>(state);
    // m_commandBuffer->m_mutableRootShaderObject = nullptr;
    SLANG_RETURN_ON_FAIL(m_commandBuffer->m_rootObject.init(
        m_commandBuffer->m_device,
        m_currentPipeline->getProgram<ShaderProgramImpl>()->m_rootObjectLayout));
    *outRootObject = &m_commandBuffer->m_rootObject;
    return SLANG_OK;
}

void ResourceCommandEncoder::endEncoding()
{
    PipelineCommandEncoder::endEncodingImpl();
}

void ResourceCommandEncoder::writeTimestamp(IQueryPool* queryPool, GfxIndex index)
{
    auto encoder = m_commandBuffer->getMetalBlitCommandEncoder();
    encoder->sampleCountersInBuffer(static_cast<QueryPoolImpl*>(queryPool)->m_counterSampleBuffer.get(), index, true);
}

void ResourceCommandEncoder::copyBuffer(
    IBufferResource* dst, Offset dstOffset, IBufferResource* src, Offset srcOffset, Size size)
{
    auto encoder = m_commandBuffer->getMetalBlitCommandEncoder();
    encoder->copyFromBuffer(
        static_cast<BufferResourceImpl*>(src)->m_buffer.get(),
        srcOffset,
        static_cast<BufferResourceImpl*>(dst)->m_buffer.get(),
        dstOffset,
        size);
}

void ResourceCommandEncoder::copyTexture(
    ITextureResource* dst,
    ResourceState dstState,
    SubresourceRange dstSubresource,
    ITextureResource::Offset3D dstOffset,
    ITextureResource* src,
    ResourceState srcState,
    SubresourceRange srcSubresource,
    ITextureResource::Offset3D srcOffset,
    ITextureResource::Extents extent)
{
    auto encoder = m_commandBuffer->getMetalBlitCommandEncoder();

    if (dstSubresource.layerCount == 0 && dstSubresource.mipLevelCount == 0 && srcSubresource.layerCount == 0 && srcSubresource.mipLevelCount == 0)
    {
        encoder->copyFromTexture(
            static_cast<TextureResourceImpl*>(src)->m_texture.get(),
            static_cast<TextureResourceImpl*>(dst)->m_texture.get());
    }
    else
    {
        for (GfxIndex layer = 0; layer < dstSubresource.layerCount; layer++)
        {
            encoder->copyFromTexture(
                static_cast<TextureResourceImpl*>(src)->m_texture.get(),
                srcSubresource.baseArrayLayer + layer,
                srcSubresource.mipLevel,
                MTL::Origin(srcOffset.x, srcOffset.y, srcOffset.z),
                MTL::Size(extent.width, extent.height, extent.depth),
                static_cast<TextureResourceImpl*>(dst)->m_texture.get(),
                dstSubresource.baseArrayLayer + layer,
                dstSubresource.mipLevel,
                MTL::Origin(dstOffset.x, dstOffset.y, dstOffset.z));
        }
    }
}

void ResourceCommandEncoder::copyTextureToBuffer(
    IBufferResource* dst,
    Offset dstOffset,
    Size dstSize,
    Size dstRowStride,
    ITextureResource* src,
    ResourceState srcState,
    SubresourceRange srcSubresource,
    ITextureResource::Offset3D srcOffset,
    ITextureResource::Extents extent)
{
    assert(srcSubresource.mipLevelCount <= 1);

    auto encoder = m_commandBuffer->getMetalBlitCommandEncoder();
    encoder->copyFromTexture(
        static_cast<TextureResourceImpl*>(src)->m_texture.get(),
        srcSubresource.baseArrayLayer,
        srcSubresource.mipLevel,
        MTL::Origin(srcOffset.x, srcOffset.y, srcOffset.z),
        MTL::Size(extent.width, extent.height, extent.depth),
        static_cast<BufferResourceImpl*>(dst)->m_buffer.get(),
        dstOffset,
        dstRowStride,
        dstSize);
}

void ResourceCommandEncoder::uploadBufferData(
    IBufferResource* buffer, Offset offset, Size size, void* data)
{
    SLANG_UNIMPLEMENTED_X("uploadBufferData");
}

void ResourceCommandEncoder::uploadTextureData(
    ITextureResource* dst,
    SubresourceRange subResourceRange,
    ITextureResource::Offset3D offset,
    ITextureResource::Extents extend,
    ITextureResource::SubresourceData* subResourceData,
    GfxCount subResourceDataCount)
{
    SLANG_UNIMPLEMENTED_X("uploadTextureData");
}

void ResourceCommandEncoder::bufferBarrier(
    GfxCount count, IBufferResource* const* buffers, ResourceState src, ResourceState dst)
{
    // We use automatic hazard tracking for now, no need for barriers.
}

void ResourceCommandEncoder::textureBarrier(
    GfxCount count, ITextureResource* const* textures, ResourceState src, ResourceState dst)
{
    // We use automatic hazard tracking for now, no need for barriers.
}

void ResourceCommandEncoder::textureSubresourceBarrier(
    ITextureResource* texture,
    SubresourceRange subresourceRange,
    ResourceState src,
    ResourceState dst)
{
    // We use automatic hazard tracking for now, no need for barriers.
}

void ResourceCommandEncoder::clearResourceView(
    IResourceView* view, ClearValue* clearValue, ClearResourceViewFlags::Enum flags)
{
    SLANG_UNIMPLEMENTED_X("clearResourceView");
}

void ResourceCommandEncoder::resolveResource(
    ITextureResource* source,
    ResourceState sourceState,
    SubresourceRange sourceRange,
    ITextureResource* dest,
    ResourceState destState,
    SubresourceRange destRange)
{
    SLANG_UNIMPLEMENTED_X("resolveResource");
}

void ResourceCommandEncoder::resolveQuery(
    IQueryPool* queryPool, GfxIndex index, GfxCount count, IBufferResource* buffer, Offset offset)
{
    auto encoder = m_commandBuffer->getMetalBlitCommandEncoder();
    encoder->resolveCounters(
        static_cast<QueryPoolImpl*>(queryPool)->m_counterSampleBuffer.get(),
        NS::Range(index, count),
        static_cast<BufferResourceImpl*>(buffer)->m_buffer.get(),
        offset);
}

void ResourceCommandEncoder::beginDebugEvent(const char* name, float rgbColor[3])
{
    NS::SharedPtr<NS::String> string = MetalUtil::createString(name);
    m_commandBuffer->m_commandBuffer->pushDebugGroup(string.get());
}

void ResourceCommandEncoder::endDebugEvent()
{
    m_commandBuffer->m_commandBuffer->popDebugGroup();
}

void RenderCommandEncoder::beginPass(IRenderPassLayout* renderPass, IFramebuffer* framebuffer)
{
    FramebufferImpl* fb = static_cast<FramebufferImpl*>(framebuffer);
    if (fb == nullptr)
    {
        return;
    }
    RenderPassLayoutImpl* renderPassLayoutImpl = static_cast<RenderPassLayoutImpl*>(renderPass);

    MTL::RenderPassDescriptor* rpd = renderPassLayoutImpl->m_renderPassDesc->copy();

    if (rpd->depthAttachment() && false)
    {
        TextureResourceViewImpl* depthView = static_cast<TextureResourceViewImpl*>(fb->depthStencilView.get());
        rpd->depthAttachment()->setTexture(depthView->m_texture->m_texture.get());
    }
    const int colorTargetCount = fb->renderTargetViews.getCount();
    for (int i = 0; i < colorTargetCount; ++i)
    {
        TextureResourceViewImpl* texView = static_cast<TextureResourceViewImpl*>(fb->renderTargetViews[i].get());
        MTL::Texture* tex = nullptr;
        assert(texView->m_texture);
        tex = texView->m_texture->m_texture.get();
        rpd->colorAttachments()->object(i)->setTexture(tex);
        rpd->colorAttachments()->object(i)->setClearColor(MTL::ClearColor(0.2, 0.4, 0.9, 1.0));
    }
    rpd->setRenderTargetWidth(fb->m_width);
    rpd->setRenderTargetHeight(fb->m_height);

    m_encoder = m_metalCommandBuffer->renderCommandEncoder(rpd);
}

void RenderCommandEncoder::endEncoding()
{
    m_encoder->endEncoding();
}

Result RenderCommandEncoder::bindPipeline(
    IPipelineState* pipelineState, IShaderObject** outRootObject)
{
    m_currentPipeline = static_cast<PipelineStateImpl*>(pipelineState);
    // Initialize the root object
    SLANG_RETURN_ON_FAIL(m_commandBuffer->m_rootObject.init(m_commandBuffer->m_device,
        m_currentPipeline->getProgram<ShaderProgramImpl>()->m_rootObjectLayout));
    *outRootObject = &m_commandBuffer->m_rootObject;
    //if (pPipelineState->m_renderState == nullptr) return SLANG_ERROR_INVALID_PARAMETER;
    //m_encoder->setRenderPipelineState(pPipelineState->m_renderState);
    return SLANG_OK;
}

Result RenderCommandEncoder::bindPipelineWithRootObject(
    IPipelineState* pipelineState, IShaderObject* rootObject)
{
    return SLANG_E_NOT_IMPLEMENTED;
}

void RenderCommandEncoder::setViewports(GfxCount count, const Viewport* viewports)
{
    static const int kMaxViewports = 8; // TODO: base on device caps
    assert(count <= kMaxViewports);

    m_viewports.setCount(count);
    for (GfxIndex i = 0; i < count; ++i)
    {
        const auto& inViewport = viewports[i];
        auto& metalViewport = m_viewports[i];
        metalViewport.height = inViewport.extentY;
        metalViewport.width = inViewport.extentX;
        metalViewport.originX = inViewport.originX;
        metalViewport.originY = inViewport.originY;
        metalViewport.znear = inViewport.minZ;
        metalViewport.zfar = inViewport.maxZ;
    }
    m_encoder->setViewports(m_viewports.begin(), count);
}

void RenderCommandEncoder::setScissorRects(GfxCount count, const ScissorRect* rects)
{
    static const int kMaxScissorRects = 9; // TODO:
    assert(count < kMaxScissorRects);

    m_scissorRects.setCount(count);
    for (GfxIndex i = 0; i < count; ++i)
    {
        const auto& inRect = rects[i];
        auto& metalRect = m_scissorRects[i];
        metalRect.height = inRect.maxX - inRect.minX;
        metalRect.width = inRect.maxY - inRect.minY;
        metalRect.x = inRect.minX;
        metalRect.y = inRect.minY;
    }
    m_encoder->setScissorRects(m_scissorRects.begin(), count);
}

void RenderCommandEncoder::setPrimitiveTopology(PrimitiveTopology topology)
{
}

void RenderCommandEncoder::setVertexBuffers(
    GfxIndex startSlot,
    GfxCount slotCount,
    IBufferResource* const* buffers,
    const Offset* offsets)
{
    for (GfxIndex i = 0; i < GfxIndex(slotCount); i++)
    {
        GfxIndex slotIndex = startSlot + i;
        BufferResourceImpl* buffer = static_cast<BufferResourceImpl*>(buffers[i]);
        if (buffer)
        {
            MTL::Buffer* vertexBuffers = {buffer->m_buffer.get()};
            m_encoder->setVertexBuffer(buffer->m_buffer.get(), offsets[i], slotIndex);
            // ...
        }
    }
}

void RenderCommandEncoder::setIndexBuffer(
    IBufferResource* buffer, Format indexFormat, Offset offset)
{
}

Result RenderCommandEncoder::prepareDraw()
{
    // Bind render state, including JIT pipeline state object creation
    auto pipeline = static_cast<PipelineStateImpl*>(m_currentPipeline.Ptr());
    if (!pipeline)
    {
        return SLANG_FAIL;
    }
    // TODO: specialization, binding, ...
    SLANG_RETURN_ON_FAIL(pipeline->ensureAPIPipelineStateCreated());
    return SLANG_OK;
}

static Result translatePrimitiveType(gfx::PrimitiveType primType, MTL::PrimitiveType& mtlType)
{
    switch (primType) 
    {
    case PrimitiveType::Triangle:
        mtlType = MTL::PrimitiveTypeTriangle;
        break;
    case PrimitiveType::Line:
        mtlType = MTL::PrimitiveTypeLine;
        break;
    case PrimitiveType::Point:
        mtlType = MTL::PrimitiveTypePoint;
        break;
    case PrimitiveType::Patch:
    default:
        return SLANG_E_INVALID_ARG;
    }
    return SLANG_OK;
}

Result RenderCommandEncoder::draw(GfxCount vertexCount, GfxIndex startVertex)
{
    SLANG_RETURN_ON_FAIL(prepareDraw());

    MTL::PrimitiveType primType;
    Result res = translatePrimitiveType(m_currentPipeline->desc.graphics.primitiveType, primType);
    if (res != SLANG_OK)
        return res;
    m_encoder->drawPrimitives(primType, startVertex, vertexCount);
    return SLANG_OK;
}

Result RenderCommandEncoder::drawIndexed(
    GfxCount indexCount, GfxIndex startIndex, GfxIndex baseVertex)
{
    return SLANG_E_NOT_IMPLEMENTED;
}

void RenderCommandEncoder::setStencilReference(uint32_t referenceValue)
{
}

Result RenderCommandEncoder::drawIndirect(
    GfxCount maxDrawCount,
    IBufferResource* argBuffer,
    Offset argOffset,
    IBufferResource* countBuffer,
    Offset countOffset)
{
    return SLANG_E_NOT_IMPLEMENTED;
}

Result RenderCommandEncoder::drawIndexedIndirect(
    GfxCount maxDrawCount,
    IBufferResource* argBuffer,
    Offset argOffset,
    IBufferResource* countBuffer,
    Offset countOffset)
{
    return SLANG_E_NOT_IMPLEMENTED;
}

Result RenderCommandEncoder::setSamplePositions(
    GfxCount samplesPerPixel, GfxCount pixelCount, const SamplePosition* samplePositions)
{
    return SLANG_E_NOT_AVAILABLE;
}

Result RenderCommandEncoder::drawInstanced(
    GfxCount vertexCount,
    GfxCount instanceCount,
    GfxIndex startVertex,
    GfxIndex startInstanceLocation)
{
    SLANG_RETURN_ON_FAIL(prepareDraw());
    return SLANG_E_NOT_IMPLEMENTED;
}

Result RenderCommandEncoder::drawIndexedInstanced(
    GfxCount indexCount,
    GfxCount instanceCount,
    GfxIndex startIndexLocation,
    GfxIndex baseVertexLocation,
    GfxIndex startInstanceLocation)
{
    SLANG_RETURN_ON_FAIL(prepareDraw());
    return SLANG_E_NOT_IMPLEMENTED;
}

Result RenderCommandEncoder::drawMeshTasks(int x, int y, int z)
{
    SLANG_RETURN_ON_FAIL(prepareDraw());
    return SLANG_E_NOT_IMPLEMENTED;
}

void ComputeCommandEncoder::endEncoding()
{
    ResourceCommandEncoder::endEncoding();
}

Result ComputeCommandEncoder::bindPipeline(
    IPipelineState* pipelineState, IShaderObject** outRootObject)
{
    return setPipelineStateImpl(pipelineState, outRootObject);
}

Result ComputeCommandEncoder::bindPipelineWithRootObject(
    IPipelineState* pipelineState, IShaderObject* rootObject)
{
    return SLANG_E_NOT_IMPLEMENTED;
}

Result ComputeCommandEncoder::dispatchCompute(int x, int y, int z)
{
    auto pipeline = static_cast<PipelineStateImpl*>(m_currentPipeline.Ptr());
    pipeline->ensureAPIPipelineStateCreated();

    auto metalComputeCommandEncoder = m_commandBuffer->getMetalComputeCommandEncoder();
    metalComputeCommandEncoder->setComputePipelineState(pipeline->m_computePipelineState.get());
    ComputeBindingContext bindingContext;
    bindingContext.init(m_commandBuffer->m_device, metalComputeCommandEncoder);
    auto program = static_cast<ShaderProgramImpl*>(m_currentPipeline->m_program.get());
    m_commandBuffer->m_rootObject.bindAsRoot(&bindingContext, program->m_rootObjectLayout);
    metalComputeCommandEncoder->dispatchThreadgroups(MTL::Size(x, y, z), pipeline->m_threadGroupSize);

    // Also create descriptor sets based on the given pipeline layout
    return SLANG_E_NOT_IMPLEMENTED;
}

Result ComputeCommandEncoder::dispatchComputeIndirect(IBufferResource* argBuffer, Offset offset)
{
    SLANG_UNIMPLEMENTED_X("dispatchComputeIndirect");
}

void RayTracingCommandEncoder::_memoryBarrier(
    int count,
    IAccelerationStructure* const* structures,
    AccessFlag srcAccess,
    AccessFlag destAccess)
{
}

void RayTracingCommandEncoder::_queryAccelerationStructureProperties(
    GfxCount accelerationStructureCount,
    IAccelerationStructure* const* accelerationStructures,
    GfxCount queryCount,
    AccelerationStructureQueryDesc* queryDescs)
{
}

void RayTracingCommandEncoder::buildAccelerationStructure(
    const IAccelerationStructure::BuildDesc& desc,
    GfxCount propertyQueryCount,
    AccelerationStructureQueryDesc* queryDescs)
{
}

void RayTracingCommandEncoder::copyAccelerationStructure(
    IAccelerationStructure* dest, IAccelerationStructure* src, AccelerationStructureCopyMode mode)
{
}

void RayTracingCommandEncoder::queryAccelerationStructureProperties(
    GfxCount accelerationStructureCount,
    IAccelerationStructure* const* accelerationStructures,
    GfxCount queryCount,
    AccelerationStructureQueryDesc* queryDescs)
{
    _queryAccelerationStructureProperties(
        accelerationStructureCount, accelerationStructures, queryCount, queryDescs);
}

void RayTracingCommandEncoder::serializeAccelerationStructure(
    DeviceAddress dest, IAccelerationStructure* source)
{
}

void RayTracingCommandEncoder::deserializeAccelerationStructure(
    IAccelerationStructure* dest, DeviceAddress source)
{
}

Result RayTracingCommandEncoder::bindPipeline(IPipelineState* pipeline, IShaderObject** outRootObject)
{
    return SLANG_E_NOT_IMPLEMENTED;
}

Result RayTracingCommandEncoder::bindPipelineWithRootObject(
    IPipelineState* pipelineState, IShaderObject* rootObject)
{
    return SLANG_E_NOT_IMPLEMENTED;
}

Result RayTracingCommandEncoder::dispatchRays(
    GfxIndex raygenShaderIndex,
    IShaderTable* shaderTable,
    GfxCount width,
    GfxCount height,
    GfxCount depth)
{
    return SLANG_E_NOT_IMPLEMENTED;
}

void RayTracingCommandEncoder::endEncoding() { }

} // namespace metal
} // namespace gfx
