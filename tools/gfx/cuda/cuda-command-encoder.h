// cuda-command-encoder.h
#pragma once
#include "cuda-base.h"

namespace gfx
{
#ifdef GFX_ENABLE_CUDA
using namespace Slang;

namespace cuda
{

class ResourceCommandEncoderImpl : public IResourceCommandEncoder
{
public:
    CommandWriter* m_writer;

    void init(CommandBufferImpl* cmdBuffer);

    virtual SLANG_NO_THROW void SLANG_MCALL endEncoding() override {}
    virtual SLANG_NO_THROW void SLANG_MCALL copyBuffer(
        IBufferResource* dst,
        Offset dstOffset,
        IBufferResource* src,
        Offset srcOffset,
        Size size) override;

    virtual SLANG_NO_THROW void SLANG_MCALL textureBarrier(
        GfxCount count,
        ITextureResource* const* textures,
        ResourceState src,
        ResourceState dst) override
    {}

    virtual SLANG_NO_THROW void SLANG_MCALL bufferBarrier(
        GfxCount count,
        IBufferResource* const* buffers,
        ResourceState src,
        ResourceState dst) override
    {}

    virtual SLANG_NO_THROW void SLANG_MCALL uploadBufferData(
        IBufferResource* dst, Offset offset, Size size, void* data) override;

    virtual SLANG_NO_THROW void SLANG_MCALL
        writeTimestamp(IQueryPool* pool, GfxIndex index) override;

    virtual SLANG_NO_THROW void SLANG_MCALL copyTexture(
        ITextureResource* dst,
        ResourceState dstState,
        SubresourceRange dstSubresource,
        ITextureResource::Offset3D dstOffset,
        ITextureResource* src,
        ResourceState srcState,
        SubresourceRange srcSubresource,
        ITextureResource::Offset3D srcOffset,
        ITextureResource::Extents extent) override;

    virtual SLANG_NO_THROW void SLANG_MCALL uploadTextureData(
        ITextureResource* dst,
        SubresourceRange subResourceRange,
        ITextureResource::Offset3D offset,
        ITextureResource::Extents extent,
        ITextureResource::SubresourceData* subResourceData,
        GfxCount subResourceDataCount) override;

    virtual SLANG_NO_THROW void SLANG_MCALL clearResourceView(
        IResourceView* view,
        ClearValue* clearValue,
        ClearResourceViewFlags::Enum flags) override;

    virtual SLANG_NO_THROW void SLANG_MCALL resolveResource(
        ITextureResource* source,
        ResourceState sourceState,
        SubresourceRange sourceRange,
        ITextureResource* dest,
        ResourceState destState,
        SubresourceRange destRange) override;

    virtual SLANG_NO_THROW void SLANG_MCALL resolveQuery(
        IQueryPool* queryPool,
        GfxIndex index,
        GfxCount count,
        IBufferResource* buffer,
        Offset offset) override;

    virtual SLANG_NO_THROW void SLANG_MCALL copyTextureToBuffer(
        IBufferResource* dst,
        Offset dstOffset,
        Size dstSize,
        Size dstRowStride,
        ITextureResource* src,
        ResourceState srcState,
        SubresourceRange srcSubresource,
        ITextureResource::Offset3D srcOffset,
        ITextureResource::Extents extent) override;

    virtual SLANG_NO_THROW void SLANG_MCALL textureSubresourceBarrier(
        ITextureResource* texture,
        SubresourceRange subresourceRange,
        ResourceState src,
        ResourceState dst) override;
    virtual SLANG_NO_THROW void SLANG_MCALL
        beginDebugEvent(const char* name, float rgbColor[3]) override;
    virtual SLANG_NO_THROW void SLANG_MCALL endDebugEvent() override {}
};

class ComputeCommandEncoderImpl
    : public IComputeCommandEncoder
    , public ResourceCommandEncoderImpl
{
public:
    SLANG_GFX_FORWARD_RESOURCE_COMMAND_ENCODER_IMPL(ResourceCommandEncoderImpl)
public:
    CommandWriter* m_writer;
    CommandBufferImpl* m_commandBuffer;
    RefPtr<ShaderObjectBase> m_rootObject;
    virtual SLANG_NO_THROW void SLANG_MCALL endEncoding() override {}
    void init(CommandBufferImpl* cmdBuffer);

    virtual SLANG_NO_THROW Result SLANG_MCALL
        bindPipeline(IPipelineState* state, IShaderObject** outRootObject) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL
        bindPipelineWithRootObject(IPipelineState* state, IShaderObject* rootObject) override;

    virtual SLANG_NO_THROW void SLANG_MCALL dispatchCompute(int x, int y, int z) override;

    virtual SLANG_NO_THROW void SLANG_MCALL
        dispatchComputeIndirect(IBufferResource* argBuffer, Offset offset) override;
};

} // namespace cuda
#endif
} // namespace gfx
