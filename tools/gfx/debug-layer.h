#pragma once

#include "slang-gfx.h"
#include "slang-com-ptr.h"
#include "core/slang-com-object.h"
#include "command-encoder-com-forward.h"

namespace gfx
{

class DebugObjectBase : public Slang::ComObject
{
public:
    uint64_t uid;
    DebugObjectBase();
};

template<typename TInterface>
class DebugObject
    : public TInterface
    , public DebugObjectBase
{
public:
    Slang::ComPtr<TInterface> baseObject;
};

template <typename TInterface>
class UnownedDebugObject
    : public TInterface
    , public DebugObjectBase
{
public:
    TInterface* baseObject = nullptr;
};

class DebugDevice : public DebugObject<IDevice>
{
public:
    SLANG_COM_OBJECT_IUNKNOWN_ALL;

public:
    DebugDevice();
    IDevice* getInterface(const Slang::Guid& guid);
    virtual SLANG_NO_THROW Result SLANG_MCALL getNativeDeviceHandles(InteropHandles* outHandles) override;
    virtual SLANG_NO_THROW bool SLANG_MCALL hasFeature(const char* feature) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        getFeatures(const char** outFeatures, Size bufferSize, GfxCount* outFeatureCount) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        getFormatSupportedResourceStates(Format format, ResourceStateSet* outStates) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL
        getSlangSession(slang::ISession** outSlangSession) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createTransientResourceHeap(
        const ITransientResourceHeap::Desc& desc,
        ITransientResourceHeap** outHeap) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createTextureResource(
        const ITextureResource::Desc& desc,
        const ITextureResource::SubresourceData* initData,
        ITextureResource** outResource) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createTextureFromNativeHandle(
        InteropHandle handle,
        const ITextureResource::Desc& srcDesc,
        ITextureResource** outResource) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createTextureFromSharedHandle(
        InteropHandle handle,
        const ITextureResource::Desc& srcDesc,
        const Size size,
        ITextureResource** outResource) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createBufferResource(
        const IBufferResource::Desc& desc,
        const void* initData,
        IBufferResource** outResource) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createBufferFromNativeHandle(
        InteropHandle handle,
        const IBufferResource::Desc& srcDesc,
        IBufferResource** outResource) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createBufferFromSharedHandle(
        InteropHandle handle,
        const IBufferResource::Desc& srcDesc,
        IBufferResource** outResource) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        createSamplerState(ISamplerState::Desc const& desc, ISamplerState** outSampler) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createTextureView(
        ITextureResource* texture,
        IResourceView::Desc const& desc,
        IResourceView** outView) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createBufferView(
        IBufferResource* buffer,
        IBufferResource* counterBuffer,
        IResourceView::Desc const& desc,
        IResourceView** outView) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL getAccelerationStructurePrebuildInfo(
        const IAccelerationStructure::BuildInputs& buildInputs,
        IAccelerationStructure::PrebuildInfo* outPrebuildInfo) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createAccelerationStructure(
        const IAccelerationStructure::CreateDesc& desc,
        IAccelerationStructure** outView) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createFramebufferLayout(
        IFramebufferLayout::Desc const& desc,
        IFramebufferLayout** outFrameBuffer) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        createFramebuffer(IFramebuffer::Desc const& desc, IFramebuffer** outFrameBuffer) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createRenderPassLayout(
        const IRenderPassLayout::Desc& desc,
        IRenderPassLayout** outRenderPassLayout) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createSwapchain(
        ISwapchain::Desc const& desc,
        WindowHandle window,
        ISwapchain** outSwapchain) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createInputLayout(
        IInputLayout::Desc const& desc,
        IInputLayout** outLayout) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        createCommandQueue(const ICommandQueue::Desc& desc, ICommandQueue** outQueue) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createShaderObject(
        slang::TypeReflection* type,
        ShaderObjectContainerType container,
        IShaderObject** outObject) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createMutableShaderObject(
        slang::TypeReflection* type,
        ShaderObjectContainerType container,
        IShaderObject** outObject) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createShaderObjectFromTypeLayout(
        slang::TypeLayoutReflection* typeLayout, IShaderObject** outObject) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createMutableShaderObjectFromTypeLayout(
        slang::TypeLayoutReflection* typeLayout, IShaderObject** outObject) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        createMutableRootShaderObject(IShaderProgram* program, IShaderObject** outObject) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        createProgram(const IShaderProgram::Desc& desc, IShaderProgram** outProgram, ISlangBlob** outDiagnostics) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createGraphicsPipelineState(
        const GraphicsPipelineStateDesc& desc,
        IPipelineState** outState) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createComputePipelineState(
        const ComputePipelineStateDesc& desc,
        IPipelineState** outState) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createRayTracingPipelineState(
        const RayTracingPipelineStateDesc& desc,
        IPipelineState** outState) override;
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL readTextureResource(
        ITextureResource* resource,
        ResourceState state,
        ISlangBlob** outBlob,
        Size* outRowPitch,
        Size* outPixelSize) override;
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL readBufferResource(
        IBufferResource* buffer,
        Offset offset,
        Size size,
        ISlangBlob** outBlob) override;
    virtual SLANG_NO_THROW const DeviceInfo& SLANG_MCALL getDeviceInfo() const override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createQueryPool(
        const IQueryPool::Desc& desc,
        IQueryPool** outPool) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        createFence(const IFence::Desc& desc, IFence** outFence) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL waitForFences(
        GfxCount fenceCount,
        IFence** fences,
        uint64_t* values,
        bool waitForAll,
        uint64_t timeout) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL getTextureAllocationInfo(
        const ITextureResource::Desc& desc, size_t* outSize, size_t* outAlignment) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL getTextureRowAlignment(size_t* outAlignment) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        createShaderTable(const IShaderTable::Desc& desc, IShaderTable** outTable) override;
};

class DebugShaderTable : public DebugObject<IShaderTable>
{
public:
    SLANG_COM_OBJECT_IUNKNOWN_ALL;
    IShaderTable* getInterface(const Slang::Guid& guid);
};

class DebugQueryPool : public DebugObject<IQueryPool>
{
public:
    SLANG_COM_OBJECT_IUNKNOWN_ALL;

    IQueryPool::Desc desc;
public:
    IQueryPool* getInterface(const Slang::Guid& guid);
    virtual SLANG_NO_THROW Result SLANG_MCALL
        getResult(GfxIndex index, GfxCount count, uint64_t* data) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL reset() override;
};

class DebugBufferResource : public DebugObject<IBufferResource>
{
public:
    SLANG_COM_OBJECT_IUNKNOWN_ALL;

public:
    IBufferResource* getInterface(const Slang::Guid& guid);
    virtual SLANG_NO_THROW Type SLANG_MCALL getType() override;
    virtual SLANG_NO_THROW Desc* SLANG_MCALL getDesc() override;
    virtual SLANG_NO_THROW DeviceAddress SLANG_MCALL getDeviceAddress() override;
    virtual SLANG_NO_THROW Result SLANG_MCALL getNativeResourceHandle(InteropHandle* outHandle) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL getSharedHandle(InteropHandle* outHandle) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL setDebugName(const char* name) override;
    virtual SLANG_NO_THROW const char* SLANG_MCALL getDebugName() override;

    virtual SLANG_NO_THROW Result SLANG_MCALL
        map(MemoryRange* rangeToRead, void** outPointer) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL unmap(MemoryRange* writtenRange) override;
};

class DebugTextureResource : public DebugObject<ITextureResource>
{
public:
    SLANG_COM_OBJECT_IUNKNOWN_ALL;

public:
    ITextureResource* getInterface(const Slang::Guid& guid);
    virtual SLANG_NO_THROW Type SLANG_MCALL getType() override;
    virtual SLANG_NO_THROW Desc* SLANG_MCALL getDesc() override;
    virtual SLANG_NO_THROW Result SLANG_MCALL getNativeResourceHandle(InteropHandle* outHandle) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL getSharedHandle(InteropHandle* outHandle) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL setDebugName(const char* name) override;
    virtual SLANG_NO_THROW const char* SLANG_MCALL getDebugName() override;
};

class DebugResourceView : public DebugObject<IResourceView>
{
public:
    SLANG_COM_OBJECT_IUNKNOWN_ALL;

public:
    IResourceView* getInterface(const Slang::Guid& guid);
    virtual SLANG_NO_THROW Desc* SLANG_MCALL getViewDesc() override;
    virtual SLANG_NO_THROW Result SLANG_MCALL getNativeHandle(InteropHandle* outNativeHandle) override;
};

class DebugAccelerationStructure : public DebugObject<IAccelerationStructure>
{
public:
    SLANG_COM_OBJECT_IUNKNOWN_ALL;

public:
    IAccelerationStructure* getInterface(const Slang::Guid& guid);
    virtual SLANG_NO_THROW DeviceAddress SLANG_MCALL getDeviceAddress() override;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        getNativeHandle(InteropHandle* outNativeHandle) override;
    virtual SLANG_NO_THROW Desc* SLANG_MCALL getViewDesc() override;
};

class DebugSamplerState : public DebugObject<ISamplerState>
{
public:
    SLANG_COM_OBJECT_IUNKNOWN_ALL;

public:
    ISamplerState* getInterface(const Slang::Guid& guid);
    virtual SLANG_NO_THROW Result SLANG_MCALL
        getNativeHandle(InteropHandle* outNativeHandle) override;};

struct ShaderOffsetKey
{
    ShaderOffset offset;
    bool operator==(ShaderOffsetKey other)
    {
        return offset.bindingArrayIndex == other.offset.bindingArrayIndex &&
            offset.bindingRangeIndex == other.offset.bindingRangeIndex &&
            offset.uniformOffset == other.offset.uniformOffset;
    }
    Slang::HashCode getHashCode()
    {
        return Slang::combineHash(
            (Slang::HashCode)offset.uniformOffset,
            Slang::combineHash(
                (Slang::HashCode)offset.bindingArrayIndex,
                (Slang::HashCode)offset.bindingRangeIndex));
    }
};

class DebugShaderObject : public DebugObject<IShaderObject>
{
public:
    SLANG_COM_OBJECT_IUNKNOWN_ALL;

public:
    IShaderObject* getInterface(const Slang::Guid& guid);
    virtual SLANG_NO_THROW slang::TypeLayoutReflection* SLANG_MCALL getElementTypeLayout() override;
    virtual SLANG_NO_THROW ShaderObjectContainerType SLANG_MCALL getContainerType() override;
    virtual SLANG_NO_THROW GfxCount SLANG_MCALL getEntryPointCount() override;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        getEntryPoint(GfxIndex index, IShaderObject** entryPoint) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        setData(ShaderOffset const& offset, void const* data, size_t size) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        getObject(ShaderOffset const& offset, IShaderObject** object) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        setObject(ShaderOffset const& offset, IShaderObject* object) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        setResource(ShaderOffset const& offset, IResourceView* resourceView) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        setSampler(ShaderOffset const& offset, ISamplerState* sampler) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL setCombinedTextureSampler(
        ShaderOffset const& offset,
        IResourceView* textureView,
        ISamplerState* sampler) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL setSpecializationArgs(
        ShaderOffset const& offset,
        const slang::SpecializationArg* args,
        GfxCount count) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL getCurrentVersion(
        ITransientResourceHeap* transientHeap, IShaderObject** outObject) override;
    virtual SLANG_NO_THROW const void* SLANG_MCALL getRawData() override;
    virtual SLANG_NO_THROW size_t SLANG_MCALL getSize() override;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        setConstantBufferOverride(IBufferResource* constantBuffer) override;

public:
    // Type name of an ordinary shader object.
    Slang::String m_typeName;

    // The slang Type of an ordinary shader object. This is null for root objects.
    slang::TypeReflection* m_slangType = nullptr;

    // The slang program from which a root shader object is created, this is null for ordinary
    // objects.
    Slang::ComPtr<slang::IComponentType> m_rootComponentType;

    DebugDevice* m_device;

    Slang::List<Slang::RefPtr<DebugShaderObject>> m_entryPoints;
    Slang::Dictionary<ShaderOffsetKey, Slang::RefPtr<DebugShaderObject>> m_objects;
    Slang::Dictionary<ShaderOffsetKey, Slang::RefPtr<DebugResourceView>> m_resources;
    Slang::Dictionary<ShaderOffsetKey, Slang::RefPtr<DebugSamplerState>> m_samplers;
};

class DebugRootShaderObject : public DebugShaderObject
{
public:
    virtual SLANG_NO_THROW uint32_t SLANG_MCALL addRef() override { return 1; }
    virtual SLANG_NO_THROW uint32_t SLANG_MCALL release() override { return 1; }
    virtual SLANG_NO_THROW Result SLANG_MCALL setSpecializationArgs(
        ShaderOffset const& offset,
        const slang::SpecializationArg* args,
        GfxCount count) override;
    void reset();
};

class DebugCommandBuffer;

class DebugResourceCommandEncoderImpl
{
public:
    virtual DebugCommandBuffer* getCommandBuffer() = 0;
    virtual bool getIsOpen() = 0;
    virtual IResourceCommandEncoder* getBaseResourceEncoder() = 0;

public:
    virtual SLANG_NO_THROW void SLANG_MCALL copyBuffer(
        IBufferResource* dst,
        Offset dstOffset,
        IBufferResource* src,
        Offset srcOffset,
        Size size);
    virtual SLANG_NO_THROW void SLANG_MCALL
        uploadBufferData(IBufferResource* dst, Offset offset, Size size, void* data);
    virtual SLANG_NO_THROW void SLANG_MCALL
        writeTimestamp(IQueryPool* pool, GfxIndex index);
    virtual SLANG_NO_THROW void SLANG_MCALL textureBarrier(
        GfxCount count,
        ITextureResource* const* textures,
        ResourceState src,
        ResourceState dst);
    virtual SLANG_NO_THROW void SLANG_MCALL bufferBarrier(
        GfxCount count,
        IBufferResource* const* buffers,
        ResourceState src,
        ResourceState dst);
    virtual SLANG_NO_THROW void SLANG_MCALL copyTexture(
        ITextureResource* dst,
        ResourceState dstState,
        SubresourceRange dstSubresource,
        ITextureResource::Offset3D dstOffset,
        ITextureResource* src,
        ResourceState srcState,
        SubresourceRange srcSubresource,
        ITextureResource::Offset3D srcOffset,
        ITextureResource::Extents extent);
    virtual SLANG_NO_THROW void SLANG_MCALL uploadTextureData(
        ITextureResource* dst,
        SubresourceRange subResourceRange,
        ITextureResource::Offset3D offset,
        ITextureResource::Extents extent,
        ITextureResource::SubresourceData* subResourceData,
        GfxCount subResourceDataCount);
    virtual SLANG_NO_THROW void SLANG_MCALL clearResourceView(
        IResourceView* view, ClearValue* clearValue, ClearResourceViewFlags::Enum flags);
    virtual SLANG_NO_THROW void SLANG_MCALL resolveResource(
        ITextureResource* source,
        ResourceState sourceState,
        SubresourceRange sourceRange,
        ITextureResource* dest,
        ResourceState destState,
        SubresourceRange destRange);
    virtual SLANG_NO_THROW void SLANG_MCALL copyTextureToBuffer(
        IBufferResource* dst,
        Offset dstOffset,
        Size dstSize,
        Size dstRowStride,
        ITextureResource* src,
        ResourceState srcState,
        SubresourceRange srcSubresource,
        ITextureResource::Offset3D srcOffset,
        ITextureResource::Extents extent);
    virtual SLANG_NO_THROW void SLANG_MCALL textureSubresourceBarrier(
        ITextureResource* texture,
        SubresourceRange subresourceRange,
        ResourceState src,
        ResourceState dst);
    virtual SLANG_NO_THROW void SLANG_MCALL resolveQuery(
        IQueryPool* queryPool,
        GfxIndex index,
        GfxCount count,
        IBufferResource* buffer,
        Offset offset);
    virtual SLANG_NO_THROW void SLANG_MCALL beginDebugEvent(const char* name, float rgbColor[3]);
    virtual SLANG_NO_THROW void SLANG_MCALL endDebugEvent();
};

class DebugComputeCommandEncoder
    : public UnownedDebugObject<IComputeCommandEncoder>
    , public DebugResourceCommandEncoderImpl
{
public:
    SLANG_GFX_FORWARD_RESOURCE_COMMAND_ENCODER_IMPL(DebugResourceCommandEncoderImpl)
    virtual DebugCommandBuffer* getCommandBuffer() override { return commandBuffer; }
    virtual bool getIsOpen() override { return isOpen; }
    virtual IResourceCommandEncoder* getBaseResourceEncoder() override { return baseObject; }

public:
    virtual SLANG_NO_THROW void SLANG_MCALL endEncoding() override;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        bindPipeline(IPipelineState* state, IShaderObject** outRootShaderObject) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        bindPipelineWithRootObject(IPipelineState* state, IShaderObject* rootObject) override;
    virtual SLANG_NO_THROW void SLANG_MCALL dispatchCompute(int x, int y, int z) override;
    virtual SLANG_NO_THROW void SLANG_MCALL
        dispatchComputeIndirect(IBufferResource* cmdBuffer, Offset offset) override;

public:
    DebugCommandBuffer* commandBuffer;
    bool isOpen = false;
};

class DebugResourceCommandEncoder
    : public UnownedDebugObject<IResourceCommandEncoder>
    , public DebugResourceCommandEncoderImpl
{
public:
    SLANG_GFX_FORWARD_RESOURCE_COMMAND_ENCODER_IMPL(DebugResourceCommandEncoderImpl)
    virtual DebugCommandBuffer* getCommandBuffer() override { return commandBuffer; }
    virtual bool getIsOpen() override { return isOpen; }
    virtual IResourceCommandEncoder* getBaseResourceEncoder() override { return baseObject; }


public:
    virtual SLANG_NO_THROW void SLANG_MCALL endEncoding() override;

public:
    DebugCommandBuffer* commandBuffer;
    bool isOpen = false;
};

class DebugRenderCommandEncoder
    : public UnownedDebugObject<IRenderCommandEncoder>
    , public DebugResourceCommandEncoderImpl
{
public:
    SLANG_GFX_FORWARD_RESOURCE_COMMAND_ENCODER_IMPL(DebugResourceCommandEncoderImpl)
    virtual DebugCommandBuffer* getCommandBuffer() override
    {
        return commandBuffer;
    }
    virtual bool getIsOpen() override { return isOpen; }
    virtual IResourceCommandEncoder* getBaseResourceEncoder() override { return baseObject; }

public:
    virtual SLANG_NO_THROW void SLANG_MCALL endEncoding() override;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        bindPipeline(IPipelineState* state, IShaderObject** outRootShaderObject) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        bindPipelineWithRootObject(IPipelineState* state, IShaderObject* rootObject) override;
    virtual SLANG_NO_THROW void SLANG_MCALL
        setViewports(GfxCount count, const Viewport* viewports) override;
    virtual SLANG_NO_THROW void SLANG_MCALL
        setScissorRects(GfxCount count, const ScissorRect* scissors) override;
    virtual SLANG_NO_THROW void SLANG_MCALL
        setPrimitiveTopology(PrimitiveTopology topology) override;
    virtual SLANG_NO_THROW void SLANG_MCALL setVertexBuffers(
        GfxIndex startSlot,
        GfxCount slotCount,
        IBufferResource* const* buffers,
        const Offset* offsets) override;
    virtual SLANG_NO_THROW void SLANG_MCALL
        setIndexBuffer(IBufferResource* buffer, Format indexFormat, Offset offset = 0) override;
    virtual SLANG_NO_THROW void SLANG_MCALL
        draw(GfxCount vertexCount, GfxIndex startVertex = 0) override;
    virtual SLANG_NO_THROW void SLANG_MCALL
        drawIndexed(GfxCount indexCount, GfxIndex startIndex = 0, GfxIndex baseVertex = 0) override;
    virtual SLANG_NO_THROW void SLANG_MCALL drawIndirect(
        GfxCount maxDrawCount,
        IBufferResource* argBuffer,
        Offset argOffset,
        IBufferResource* countBuffer,
        Offset countOffset) override;
    virtual SLANG_NO_THROW void SLANG_MCALL drawIndexedIndirect(
        GfxCount maxDrawCount,
        IBufferResource* argBuffer,
        Offset argOffset,
        IBufferResource* countBuffer,
        Offset countOffset) override;
    virtual SLANG_NO_THROW void SLANG_MCALL setStencilReference(uint32_t referenceValue) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL setSamplePositions(
        GfxCount samplesPerPixel,
        GfxCount pixelCount,
        const SamplePosition* samplePositions) override;
    virtual SLANG_NO_THROW void SLANG_MCALL drawInstanced(
        GfxCount vertexCount,
        GfxCount instanceCount,
        GfxIndex startVertex,
        GfxIndex startInstanceLocation) override;

    virtual SLANG_NO_THROW void SLANG_MCALL drawIndexedInstanced(
        GfxCount indexCount,
        GfxCount instanceCount,
        GfxIndex startIndexLocation,
        GfxIndex baseVertexLocation,
        GfxIndex startInstanceLocation) override;

public:
    DebugCommandBuffer* commandBuffer;
    bool isOpen = false;
};

class DebugRayTracingCommandEncoder
    : public UnownedDebugObject<IRayTracingCommandEncoder>
    , public DebugResourceCommandEncoderImpl
{
public:
    SLANG_GFX_FORWARD_RESOURCE_COMMAND_ENCODER_IMPL(DebugResourceCommandEncoderImpl)
    virtual DebugCommandBuffer* getCommandBuffer() override { return commandBuffer; }
    virtual bool getIsOpen() override { return isOpen; }
    virtual IResourceCommandEncoder* getBaseResourceEncoder() override { return baseObject; }

public:
    virtual SLANG_NO_THROW void SLANG_MCALL endEncoding() override;
    virtual SLANG_NO_THROW void SLANG_MCALL buildAccelerationStructure(
        const IAccelerationStructure::BuildDesc& desc,
        GfxCount propertyQueryCount,
        AccelerationStructureQueryDesc* queryDescs) override;
    virtual SLANG_NO_THROW void SLANG_MCALL copyAccelerationStructure(
        IAccelerationStructure* dest,
        IAccelerationStructure* src,
        AccelerationStructureCopyMode mode) override;
    virtual SLANG_NO_THROW void SLANG_MCALL queryAccelerationStructureProperties(
        GfxCount accelerationStructureCount,
        IAccelerationStructure* const* accelerationStructures,
        GfxCount queryCount,
        AccelerationStructureQueryDesc* queryDescs) override;
    virtual SLANG_NO_THROW void SLANG_MCALL
        serializeAccelerationStructure(DeviceAddress dest, IAccelerationStructure* source) override;
    virtual SLANG_NO_THROW void SLANG_MCALL deserializeAccelerationStructure(
        IAccelerationStructure* dest,
        DeviceAddress source) override;
    virtual SLANG_NO_THROW void SLANG_MCALL
        bindPipeline(IPipelineState* state, IShaderObject** outRootObject) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        bindPipelineWithRootObject(IPipelineState* state, IShaderObject* rootObject) override;
    virtual SLANG_NO_THROW void SLANG_MCALL dispatchRays(
        GfxIndex rayGenShaderIndex,
        IShaderTable* shaderTable,
        GfxCount width,
        GfxCount height,
        GfxCount depth) override;

public:
    DebugCommandBuffer* commandBuffer;
    bool isOpen = false;
};

class DebugFence : public DebugObject<IFence>
{
public:
    SLANG_COM_OBJECT_IUNKNOWN_ALL;
    IFence* getInterface(const Slang::Guid& guid);
    virtual SLANG_NO_THROW Result SLANG_MCALL getCurrentValue(uint64_t* outValue) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL setCurrentValue(uint64_t value) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL getSharedHandle(InteropHandle* outHandle) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL getNativeHandle(InteropHandle* outNativeHandle) override;
};

class DebugTransientResourceHeap;

class DebugCommandBuffer : public DebugObject<ICommandBuffer>
{
public:
    SLANG_COM_OBJECT_IUNKNOWN_ALL;

public:
    DebugTransientResourceHeap* m_transientHeap;

private:
    DebugRenderCommandEncoder m_renderCommandEncoder;
    DebugComputeCommandEncoder m_computeCommandEncoder;
    DebugResourceCommandEncoder m_resourceCommandEncoder;
    DebugRayTracingCommandEncoder m_rayTracingCommandEncoder;

public:
    DebugCommandBuffer();
    ICommandBuffer* getInterface(const Slang::Guid& guid);
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

private:
    void checkEncodersClosedBeforeNewEncoder();
    void checkCommandBufferOpenWhenCreatingEncoder();
public:
    DebugRootShaderObject rootObject;
    bool isOpen = true;
};

class DebugCommandQueue : public DebugObject<ICommandQueue>
{
public:
    SLANG_COM_OBJECT_IUNKNOWN_ALL;

public:
    ICommandQueue* getInterface(const Slang::Guid& guid);
    virtual SLANG_NO_THROW const Desc& SLANG_MCALL getDesc() override;
    virtual SLANG_NO_THROW void SLANG_MCALL
        executeCommandBuffers(GfxCount count, ICommandBuffer* const* commandBuffers, IFence* fence, uint64_t valueToSignal) override;
    virtual SLANG_NO_THROW void SLANG_MCALL waitOnHost() override;
    virtual SLANG_NO_THROW Result SLANG_MCALL waitForFenceValuesOnDevice(
        GfxCount fenceCount, IFence** fences, uint64_t* waitValues) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL getNativeHandle(InteropHandle* outHandle) override;
};

class DebugFramebuffer
    : public DebugObject<IFramebuffer>
{
public:
    SLANG_COM_OBJECT_IUNKNOWN_ALL;

public:
    IFramebuffer* getInterface(const Slang::Guid& guid);
};

class DebugFramebufferLayout : public DebugObject<IFramebufferLayout>
{
public:
    SLANG_COM_OBJECT_IUNKNOWN_ALL;

public:
    IFramebufferLayout* getInterface(const Slang::Guid& guid);
};

class DebugInputLayout : public DebugObject<IInputLayout>
{
public:
    SLANG_COM_OBJECT_IUNKNOWN_ALL;

public:
    IInputLayout* getInterface(const Slang::Guid& guid);
};

class DebugPipelineState : public DebugObject<IPipelineState>
{
public:
    SLANG_COM_OBJECT_IUNKNOWN_ALL;

public:
    IPipelineState* getInterface(const Slang::Guid& guid);
    virtual SLANG_NO_THROW Result SLANG_MCALL getNativeHandle(InteropHandle* outHandle) override;
};

class DebugRenderPassLayout : public DebugObject<IRenderPassLayout>
{
public:
    SLANG_COM_OBJECT_IUNKNOWN_ALL;

public:
    IRenderPassLayout* getInterface(const Slang::Guid& guid);
};

class DebugShaderProgram : public DebugObject<IShaderProgram>
{
public:
    SLANG_COM_OBJECT_IUNKNOWN_ALL;

public:
    IShaderProgram* getInterface(const Slang::Guid& guid);

    DebugShaderProgram(const IShaderProgram::Desc& desc);

public:
    Slang::ComPtr<slang::IComponentType> m_slangProgram;
};

class DebugTransientResourceHeap : public DebugObject<ITransientResourceHeap>
{
public:
    SLANG_COM_OBJECT_IUNKNOWN_ALL;

public:
    ITransientResourceHeap* getInterface(const Slang::Guid& guid);
    virtual SLANG_NO_THROW Result SLANG_MCALL synchronizeAndReset() override;
    virtual SLANG_NO_THROW Result SLANG_MCALL finish() override;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        createCommandBuffer(ICommandBuffer** outCommandBuffer) override;
};

class DebugSwapchain : public DebugObject<ISwapchain>
{
public:
    SLANG_COM_OBJECT_IUNKNOWN_ALL;

public:
    ISwapchain* getInterface(const Slang::Guid& guid);
    virtual SLANG_NO_THROW const Desc& SLANG_MCALL getDesc() override;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        getImage(GfxIndex index, ITextureResource** outResource) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL present() override;
    virtual SLANG_NO_THROW int SLANG_MCALL acquireNextImage() override;
    virtual SLANG_NO_THROW Result SLANG_MCALL resize(GfxCount width, GfxCount height) override;
    virtual SLANG_NO_THROW bool SLANG_MCALL isOccluded() override;
    virtual SLANG_NO_THROW Result SLANG_MCALL setFullScreenMode(bool mode) override;

public:
    Slang::RefPtr<DebugCommandQueue> queue;
    Desc desc;

private:
    Slang::List<Slang::RefPtr<DebugTextureResource>> m_images;
    void maybeRebuildImageList();
};

}
