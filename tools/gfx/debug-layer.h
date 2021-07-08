#pragma once

#include "slang-gfx.h"
#include "slang-com-ptr.h"
#include "core/slang-com-object.h"

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
    virtual SLANG_NO_THROW bool SLANG_MCALL hasFeature(const char* feature) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        getFeatures(const char** outFeatures, UInt bufferSize, UInt* outFeatureCount) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        getSlangSession(slang::ISession** outSlangSession) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createTransientResourceHeap(
        const ITransientResourceHeap::Desc& desc,
        ITransientResourceHeap** outHeap) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createTextureResource(
        const ITextureResource::Desc& desc,
        const ITextureResource::SubresourceData* initData,
        ITextureResource** outResource) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createBufferResource(
        const IBufferResource::Desc& desc,
        const void* initData,
        IBufferResource** outResource) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        createSamplerState(ISamplerState::Desc const& desc, ISamplerState** outSampler) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createTextureView(
        ITextureResource* texture,
        IResourceView::Desc const& desc,
        IResourceView** outView) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createBufferView(
        IBufferResource* buffer,
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
        const InputElementDesc* inputElements,
        UInt inputElementCount,
        IInputLayout** outLayout) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        createCommandQueue(const ICommandQueue::Desc& desc, ICommandQueue** outQueue) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createShaderObject(
        slang::TypeReflection* type,
        ShaderObjectContainerType container,
        IShaderObject** outObject) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        createProgram(const IShaderProgram::Desc& desc, IShaderProgram** outProgram) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createGraphicsPipelineState(
        const GraphicsPipelineStateDesc& desc,
        IPipelineState** outState) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createComputePipelineState(
        const ComputePipelineStateDesc& desc,
        IPipelineState** outState) override;
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL readTextureResource(
        ITextureResource* resource,
        ResourceState state,
        ISlangBlob** outBlob,
        size_t* outRowPitch,
        size_t* outPixelSize) override;
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL readBufferResource(
        IBufferResource* buffer,
        size_t offset,
        size_t size,
        ISlangBlob** outBlob) override;
    virtual SLANG_NO_THROW const DeviceInfo& SLANG_MCALL getDeviceInfo() const override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createQueryPool(
        const IQueryPool::Desc& desc,
        IQueryPool** outPool) override;
};

class DebugQueryPool : public DebugObject<IQueryPool>
{
public:
    SLANG_COM_OBJECT_IUNKNOWN_ALL;

    IQueryPool::Desc desc;
public:
    IQueryPool* getInterface(const Slang::Guid& guid);
    virtual SLANG_NO_THROW Result SLANG_MCALL getResult(SlangInt index, SlangInt count, uint64_t* data) override;
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
};

class DebugTextureResource : public DebugObject<ITextureResource>
{
public:
    SLANG_COM_OBJECT_IUNKNOWN_ALL;

public:
    ITextureResource* getInterface(const Slang::Guid& guid);
    virtual SLANG_NO_THROW Type SLANG_MCALL getType() override;
    virtual SLANG_NO_THROW Desc* SLANG_MCALL getDesc() override;
};

class DebugResourceView : public DebugObject<IResourceView>
{
public:
    SLANG_COM_OBJECT_IUNKNOWN_ALL;

public:
    IResourceView* getInterface(const Slang::Guid& guid);
    virtual SLANG_NO_THROW Desc* SLANG_MCALL getViewDesc() override;
};

class DebugAccelerationStructure : public DebugObject<IAccelerationStructure>
{
public:
    SLANG_COM_OBJECT_IUNKNOWN_ALL;

public:
    IAccelerationStructure* getInterface(const Slang::Guid& guid);
    virtual SLANG_NO_THROW DeviceAddress SLANG_MCALL getDeviceAddress() override;
    virtual SLANG_NO_THROW Desc* SLANG_MCALL getViewDesc() override;
};

class DebugSamplerState : public DebugObject<ISamplerState>
{
public:
    SLANG_COM_OBJECT_IUNKNOWN_ALL;

public:
    ISamplerState* getInterface(const Slang::Guid& guid);
};

class DebugShaderObject : public DebugObject<IShaderObject>
{
public:
    SLANG_COM_OBJECT_IUNKNOWN_ALL;

public:
    IShaderObject* getInterface(const Slang::Guid& guid);
    virtual SLANG_NO_THROW slang::TypeLayoutReflection* SLANG_MCALL getElementTypeLayout() override;
    virtual SLANG_NO_THROW ShaderObjectContainerType SLANG_MCALL getContainerType() override;
    virtual SLANG_NO_THROW UInt SLANG_MCALL getEntryPointCount() override;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        getEntryPoint(UInt index, IShaderObject** entryPoint) override;
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
        uint32_t count) override;

public:
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
    Slang::String m_typeName;
    slang::TypeReflection* m_slangType = nullptr;
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
        uint32_t count) override;
};

class DebugCommandBuffer;

class DebugComputeCommandEncoder : public UnownedDebugObject<IComputeCommandEncoder>
{
public:
    virtual SLANG_NO_THROW void SLANG_MCALL endEncoding() override;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        bindPipeline(IPipelineState* state, IShaderObject** outRootShaderObject) override;
    virtual SLANG_NO_THROW void SLANG_MCALL dispatchCompute(int x, int y, int z) override;
    virtual SLANG_NO_THROW void SLANG_MCALL writeTimestamp(IQueryPool* pool, SlangInt index) override;

public:
    DebugCommandBuffer* commandBuffer;
    bool isOpen = false;
};

class DebugRenderCommandEncoder : public UnownedDebugObject<IRenderCommandEncoder>
{
public:
    virtual SLANG_NO_THROW void SLANG_MCALL endEncoding() override;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        bindPipeline(IPipelineState* state, IShaderObject** outRootShaderObject) override;
    virtual SLANG_NO_THROW void SLANG_MCALL
        setViewports(uint32_t count, const Viewport* viewports) override;
    virtual SLANG_NO_THROW void SLANG_MCALL
        setScissorRects(uint32_t count, const ScissorRect* scissors) override;
    virtual SLANG_NO_THROW void SLANG_MCALL
        setPrimitiveTopology(PrimitiveTopology topology) override;
    virtual SLANG_NO_THROW void SLANG_MCALL setVertexBuffers(
        UInt startSlot,
        UInt slotCount,
        IBufferResource* const* buffers,
        const UInt* strides,
        const UInt* offsets) override;
    virtual SLANG_NO_THROW void SLANG_MCALL
        setIndexBuffer(IBufferResource* buffer, Format indexFormat, UInt offset = 0) override;
    virtual SLANG_NO_THROW void SLANG_MCALL draw(UInt vertexCount, UInt startVertex = 0) override;
    virtual SLANG_NO_THROW void SLANG_MCALL
        drawIndexed(UInt indexCount, UInt startIndex = 0, UInt baseVertex = 0) override;
    virtual SLANG_NO_THROW void SLANG_MCALL setStencilReference(uint32_t referenceValue) override;
    virtual SLANG_NO_THROW void SLANG_MCALL writeTimestamp(IQueryPool* pool, SlangInt index) override;

public:
    DebugCommandBuffer* commandBuffer;
    bool isOpen = false;
};

class DebugResourceCommandEncoder : public UnownedDebugObject<IResourceCommandEncoder>
{
public:
    virtual SLANG_NO_THROW void SLANG_MCALL endEncoding() override;
    virtual SLANG_NO_THROW void SLANG_MCALL copyBuffer(
        IBufferResource* dst,
        size_t dstOffset,
        IBufferResource* src,
        size_t srcOffset,
        size_t size) override;
    virtual SLANG_NO_THROW void SLANG_MCALL
        uploadBufferData(IBufferResource* dst, size_t offset, size_t size, void* data) override;
    virtual SLANG_NO_THROW void SLANG_MCALL writeTimestamp(IQueryPool* pool, SlangInt index) override;

public:
    DebugCommandBuffer* commandBuffer;
    bool isOpen = false;
};

class DebugRayTracingCommandEncoder : public UnownedDebugObject<IRayTracingCommandEncoder>
{
public:
    virtual SLANG_NO_THROW void SLANG_MCALL endEncoding() override;
    virtual SLANG_NO_THROW void SLANG_MCALL
        writeTimestamp(IQueryPool* pool, SlangInt index) override;
    virtual SLANG_NO_THROW void SLANG_MCALL buildAccelerationStructure(
        const IAccelerationStructure::BuildDesc& desc,
        int propertyQueryCount,
        AccelerationStructureQueryDesc* queryDescs) override;
    virtual SLANG_NO_THROW void SLANG_MCALL copyAccelerationStructure(
        IAccelerationStructure* dest,
        IAccelerationStructure* src,
        AccelerationStructureCopyMode mode) override;
    virtual SLANG_NO_THROW void SLANG_MCALL queryAccelerationStructureProperties(
        int accelerationStructureCount,
        IAccelerationStructure* const* accelerationStructures,
        int queryCount,
        AccelerationStructureQueryDesc* queryDescs) override;
    virtual SLANG_NO_THROW void SLANG_MCALL
        serializeAccelerationStructure(DeviceAddress dest, IAccelerationStructure* source) override;
    virtual SLANG_NO_THROW void SLANG_MCALL deserializeAccelerationStructure(
        IAccelerationStructure* dest,
        DeviceAddress source) override;
    virtual SLANG_NO_THROW void SLANG_MCALL memoryBarrier(
        int count,
        IAccelerationStructure* const* structures,
        AccessFlag::Enum sourceAccess,
        AccessFlag::Enum destAccess) override;

public:
    DebugCommandBuffer* commandBuffer;
    bool isOpen = false;
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
        executeCommandBuffers(uint32_t count, ICommandBuffer* const* commandBuffers) override;
    virtual SLANG_NO_THROW void SLANG_MCALL wait() override;
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

};

class DebugTransientResourceHeap : public DebugObject<ITransientResourceHeap>
{
public:
    SLANG_COM_OBJECT_IUNKNOWN_ALL;

public:
    ITransientResourceHeap* getInterface(const Slang::Guid& guid);
    virtual SLANG_NO_THROW Result SLANG_MCALL synchronizeAndReset() override;
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
        getImage(uint32_t index, ITextureResource** outResource) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL present() override;
    virtual SLANG_NO_THROW int SLANG_MCALL acquireNextImage() override;
    virtual SLANG_NO_THROW Result SLANG_MCALL resize(uint32_t width, uint32_t height) override;

public:
    Slang::RefPtr<DebugCommandQueue> queue;
    Desc desc;

private:
    Slang::List<Slang::RefPtr<DebugTextureResource>> m_images;
    void maybeRebuildImageList();
};

}
