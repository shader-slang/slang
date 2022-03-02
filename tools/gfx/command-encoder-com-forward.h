#pragma once

#define SLANG_GFX_FORWARD_RESOURCE_COMMAND_ENCODER_IMPL(ResourceCommandEncoderBase)                     \
    virtual SLANG_NO_THROW void SLANG_MCALL copyBuffer(                                                 \
        IBufferResource* dst,                                                                           \
        size_t dstOffset,                                                                               \
        IBufferResource* src,                                                                           \
        size_t srcOffset,                                                                               \
        size_t size) override                                                                           \
    {                                                                                                   \
        ResourceCommandEncoderBase::copyBuffer(dst, dstOffset, src, srcOffset, size);                   \
    }                                                                                                   \
    virtual SLANG_NO_THROW void SLANG_MCALL copyTexture(                                                \
        ITextureResource* dst,                                                                          \
        ResourceState dstState,                                                                         \
        SubresourceRange dstSubresource,                                                                \
        ITextureResource::Offset3D dstOffset,                                                           \
        ITextureResource* src,                                                                          \
        ResourceState srcState,                                                                         \
        SubresourceRange srcSubresource,                                                                \
        ITextureResource::Offset3D srcOffset,                                                           \
        ITextureResource::Size extent) override                                                         \
    {                                                                                                   \
        ResourceCommandEncoderBase::copyTexture(                                                        \
            dst,                                                                                        \
            dstState,                                                                                   \
            dstSubresource,                                                                             \
            dstOffset,                                                                                  \
            src,                                                                                        \
            srcState,                                                                                   \
            srcSubresource,                                                                             \
            srcOffset,                                                                                  \
            extent);                                                                                    \
    }                                                                                                   \
    virtual SLANG_NO_THROW void SLANG_MCALL copyTextureToBuffer(                                        \
        IBufferResource* dst,                                                                           \
        size_t dstOffset,                                                                               \
        size_t dstSize,                                                                                 \
        size_t dstRowStride,                                                                            \
        ITextureResource* src,                                                                          \
        ResourceState srcState,                                                                         \
        SubresourceRange srcSubresource,                                                                \
        ITextureResource::Offset3D srcOffset,                                                           \
        ITextureResource::Size extent) override                                                         \
    {                                                                                                   \
        ResourceCommandEncoderBase::copyTextureToBuffer(                                                \
            dst, dstOffset, dstSize, dstRowStride, src, srcState, srcSubresource, srcOffset, extent);   \
    }                                                                                                   \
    virtual SLANG_NO_THROW void SLANG_MCALL uploadTextureData(                                          \
        ITextureResource* dst,                                                                          \
        SubresourceRange subResourceRange,                                                              \
        ITextureResource::Offset3D offset,                                                              \
        ITextureResource::Size extent,                                                                  \
        ITextureResource::SubresourceData* subResourceData,                                             \
        size_t subResourceDataCount) override                                                           \
    {                                                                                                   \
        ResourceCommandEncoderBase::uploadTextureData(                                                  \
            dst, subResourceRange, offset, extent, subResourceData, subResourceDataCount);              \
    }                                                                                                   \
    virtual SLANG_NO_THROW void SLANG_MCALL uploadBufferData(                                           \
        IBufferResource* dst, size_t offset, size_t size, void* data) override                          \
    {                                                                                                   \
        ResourceCommandEncoderBase::uploadBufferData(dst, offset, size, data);                          \
    }                                                                                                   \
    virtual SLANG_NO_THROW void SLANG_MCALL textureBarrier(                                             \
        size_t count, ITextureResource* const* textures, ResourceState src, ResourceState dst)          \
        override                                                                                        \
    {                                                                                                   \
        ResourceCommandEncoderBase::textureBarrier(count, textures, src, dst);                          \
    }                                                                                                   \
    virtual SLANG_NO_THROW void SLANG_MCALL textureSubresourceBarrier(                                  \
        ITextureResource* texture,                                                                      \
        SubresourceRange subresourceRange,                                                              \
        ResourceState src,                                                                              \
        ResourceState dst) override                                                                     \
    {                                                                                                   \
        ResourceCommandEncoderBase::textureSubresourceBarrier(                                          \
            texture, subresourceRange, src, dst);                                                       \
    }                                                                                                   \
    virtual SLANG_NO_THROW void SLANG_MCALL bufferBarrier(                                              \
        size_t count, IBufferResource* const* buffers, ResourceState src, ResourceState dst)            \
        override                                                                                        \
    {                                                                                                   \
        ResourceCommandEncoderBase::bufferBarrier(count, buffers, src, dst);                            \
    }                                                                                                   \
    virtual SLANG_NO_THROW void SLANG_MCALL clearResourceView(                                          \
        IResourceView* view, ClearValue* clearValue, ClearResourceViewFlags::Enum flags) override       \
    {                                                                                                   \
        ResourceCommandEncoderBase::clearResourceView(view, clearValue, flags);                         \
    }                                                                                                   \
    virtual SLANG_NO_THROW void SLANG_MCALL resolveResource(                                            \
        ITextureResource* source,                                                                       \
        ResourceState sourceState,                                                                      \
        SubresourceRange sourceRange,                                                                   \
        ITextureResource* dest,                                                                         \
        ResourceState destState,                                                                        \
        SubresourceRange destRange) override                                                            \
    {                                                                                                   \
        ResourceCommandEncoderBase::resolveResource(                                                    \
            source, sourceState, sourceRange, dest, destState, destRange);                              \
    }                                                                                                   \
    virtual SLANG_NO_THROW void SLANG_MCALL resolveQuery(                                               \
        IQueryPool* queryPool,                                                                          \
        uint32_t index,                                                                                 \
        uint32_t count,                                                                                 \
        IBufferResource* buffer,                                                                        \
        uint64_t offset) override                                                                       \
    {                                                                                                   \
        ResourceCommandEncoderBase::resolveQuery(queryPool, index, count, buffer, offset);              \
    }                                                                                                   \
    virtual SLANG_NO_THROW void SLANG_MCALL writeTimestamp(IQueryPool* pool, SlangInt index)            \
        override                                                                                        \
    {                                                                                                   \
        ResourceCommandEncoderBase::writeTimestamp(pool, index);                                        \
    }                                                                                                   \
    virtual SLANG_NO_THROW void SLANG_MCALL beginDebugEvent(const char* name, float rgbColor[3])        \
        override                                                                                        \
    {                                                                                                   \
        ResourceCommandEncoderBase::beginDebugEvent(name, rgbColor);                                    \
    }                                                                                                   \
    virtual SLANG_NO_THROW void SLANG_MCALL endDebugEvent() override                                    \
    {                                                                                                   \
        ResourceCommandEncoderBase::endDebugEvent();                                                    \
    }
