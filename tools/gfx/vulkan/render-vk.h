// render-vk.h
#pragma once

#include "../command-encoder-com-forward.h"
#include "../mutable-shader-object.h"
#include "../renderer-shared.h"
#include "../transient-resource-heap-base.h"
#include "core/slang-chunked-list.h"
#include "vk-api.h"
#include "vk-descriptor-allocator.h"
#include "vk-device-queue.h"

namespace gfx
{
namespace vk
{
using namespace Slang;

enum
{
    kMaxRenderTargets = 8,
    kMaxTargets = kMaxRenderTargets + 1,
    kMaxPushConstantSize = 256,
    kMaxDescriptorSets = 8,
};

class FramebufferImpl;
class TransientResourceHeapImpl;
class PipelineCommandEncoder;

class DeviceImpl : public RendererBase
{
public:
    // Renderer    implementation
    Result initVulkanInstanceAndDevice(const InteropHandle* handles, bool useValidationLayer);
    virtual SLANG_NO_THROW Result SLANG_MCALL initialize(const Desc& desc) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        getFormatSupportedResourceStates(Format format, ResourceStateSet* outStates) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createTransientResourceHeap(
        const ITransientResourceHeap::Desc& desc, ITransientResourceHeap** outHeap) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        createCommandQueue(const ICommandQueue::Desc& desc, ICommandQueue** outQueue) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createSwapchain(
        const ISwapchain::Desc& desc, WindowHandle window, ISwapchain** outSwapchain) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createFramebufferLayout(
        const IFramebufferLayout::Desc& desc, IFramebufferLayout** outLayout) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        createFramebuffer(const IFramebuffer::Desc& desc, IFramebuffer** outFramebuffer) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createRenderPassLayout(
        const IRenderPassLayout::Desc& desc, IRenderPassLayout** outRenderPassLayout) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createTextureResource(
        const ITextureResource::Desc& desc,
        const ITextureResource::SubresourceData* initData,
        ITextureResource** outResource) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createBufferResource(
        const IBufferResource::Desc& desc,
        const void* initData,
        IBufferResource** outResource) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createBufferFromNativeHandle(
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

    virtual SLANG_NO_THROW Result SLANG_MCALL
        createInputLayout(IInputLayout::Desc const& desc, IInputLayout** outLayout) override;

    virtual Result createShaderObjectLayout(
        slang::TypeLayoutReflection* typeLayout, ShaderObjectLayoutBase** outLayout) override;
    virtual Result createShaderObject(
        ShaderObjectLayoutBase* layout, IShaderObject** outObject) override;
    virtual Result createMutableShaderObject(
        ShaderObjectLayoutBase* layout, IShaderObject** outObject) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        createMutableRootShaderObject(IShaderProgram* program, IShaderObject** outObject) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL
        createShaderTable(const IShaderTable::Desc& desc, IShaderTable** outShaderTable) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createProgram(
        const IShaderProgram::Desc& desc,
        IShaderProgram** outProgram,
        ISlangBlob** outDiagnosticBlob) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createGraphicsPipelineState(
        const GraphicsPipelineStateDesc& desc, IPipelineState** outState) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createComputePipelineState(
        const ComputePipelineStateDesc& desc, IPipelineState** outState) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createRayTracingPipelineState(
        const RayTracingPipelineStateDesc& desc, IPipelineState** outState) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        createQueryPool(const IQueryPool::Desc& desc, IQueryPool** outPool) override;

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL readTextureResource(
        ITextureResource* texture,
        ResourceState state,
        ISlangBlob** outBlob,
        Size* outRowPitch,
        Size* outPixelSize) override;

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL readBufferResource(
        IBufferResource* buffer, Offset offset, Size size, ISlangBlob** outBlob) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL getAccelerationStructurePrebuildInfo(
        const IAccelerationStructure::BuildInputs& buildInputs,
        IAccelerationStructure::PrebuildInfo* outPrebuildInfo) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL createAccelerationStructure(
        const IAccelerationStructure::CreateDesc& desc, IAccelerationStructure** outView) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL getTextureAllocationInfo(
        const ITextureResource::Desc& desc, Size* outSize, Size* outAlignment) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL getTextureRowAlignment(Size* outAlignment) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL
        createFence(const IFence::Desc& desc, IFence** outFence) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL waitForFences(
        GfxCount fenceCount,
        IFence** fences,
        uint64_t* fenceValues,
        bool waitForAll,
        uint64_t timeout) override;

    void waitForGpu();

    virtual SLANG_NO_THROW const DeviceInfo& SLANG_MCALL getDeviceInfo() const override;

    virtual SLANG_NO_THROW Result SLANG_MCALL
        getNativeDeviceHandles(InteropHandles* outHandles) override;

    ~DeviceImpl();

public:
    VkBool32 handleDebugMessage(
        VkDebugReportFlagsEXT flags,
        VkDebugReportObjectTypeEXT objType,
        uint64_t srcObject,
        Size location, // TODO: Is "location" still needed for this function?
        int32_t msgCode,
        const char* pLayerPrefix,
        const char* pMsg);

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugMessageCallback(
        VkDebugReportFlagsEXT flags,
        VkDebugReportObjectTypeEXT objType,
        uint64_t srcObject,
        Size location, // TODO: Is "location" still needed? Calls handleDebugMessage() which doesn't use it
        int32_t msgCode,
        const char* pLayerPrefix,
        const char* pMsg,
        void* pUserData);

    void _transitionImageLayout(
        VkImage image,
        VkFormat format,
        const TextureResource::Desc& desc,
        VkImageLayout oldLayout,
        VkImageLayout newLayout);
    void _transitionImageLayout(
        VkCommandBuffer commandBuffer,
        VkImage image,
        VkFormat format,
        const TextureResource::Desc& desc,
        VkImageLayout oldLayout,
        VkImageLayout newLayout);

    uint32_t getQueueFamilyIndex(ICommandQueue::QueueType queueType);

public:
    // DeviceImpl members.

    DeviceInfo m_info;
    String m_adapterName;

    VkDebugReportCallbackEXT m_debugReportCallback = VK_NULL_HANDLE;

    VkDevice m_device = VK_NULL_HANDLE;

    VulkanModule m_module;
    VulkanApi m_api;

    VulkanDeviceQueue m_deviceQueue;
    uint32_t m_queueFamilyIndex;

    Desc m_desc;

    DescriptorSetAllocator descriptorSetAllocator;

    uint32_t m_queueAllocCount;

    // A list to hold objects that may have a strong back reference to the device
    // instance. Because of the pipeline cache in `RendererBase`, there could be a reference
    // cycle among `DeviceImpl`->`PipelineStateImpl`->`ShaderProgramImpl`->`DeviceImpl`.
    // Depending on whether a `PipelineState` objects gets stored in pipeline cache, there
    // may or may not be such a reference cycle.
    // We need to hold strong references to any objects that may become part of the reference
    // cycle here, so that when objects like `ShaderProgramImpl` lost all public refernces, we
    // can always safely break the strong reference in `ShaderProgramImpl::m_device` without
    // worrying the `ShaderProgramImpl` object getting destroyed after the completion of
    // `DeviceImpl::~DeviceImpl()'.
    ChunkedList<RefPtr<RefObject>, 1024> m_deviceObjectsWithPotentialBackReferences;

    VkSampler m_defaultSampler;

    RefPtr<FramebufferImpl> m_emptyFramebuffer;
};

class VKBufferHandleRAII
{
public:
    /// Initialize a buffer with specified size, and memory props
    Result init(
        const VulkanApi& api,
        Size bufferSize,
        VkBufferUsageFlags usage,
        VkMemoryPropertyFlags reqMemoryProperties,
        bool isShared = false,
        VkExternalMemoryHandleTypeFlagsKHR extMemHandleType = 0);

    /// Returns true if has been initialized
    bool isInitialized() const { return m_api != nullptr; }

    VKBufferHandleRAII()
        : m_api(nullptr)
    {}

    ~VKBufferHandleRAII()
    {
        if (m_api)
        {
            m_api->vkDestroyBuffer(m_api->m_device, m_buffer, nullptr);
            m_api->vkFreeMemory(m_api->m_device, m_memory, nullptr);
        }
    }

    VkBuffer m_buffer;
    VkDeviceMemory m_memory;
    const VulkanApi* m_api;
};

class InputLayoutImpl : public InputLayoutBase
{
public:
    List<VkVertexInputAttributeDescription> m_attributeDescs;
    List<VkVertexInputBindingDescription> m_streamDescs;
};

class BufferResourceImpl : public BufferResource
{
public:
    typedef BufferResource Parent;

    BufferResourceImpl(const IBufferResource::Desc& desc, DeviceImpl* renderer);

    ~BufferResourceImpl();

    RefPtr<DeviceImpl> m_renderer;
    VKBufferHandleRAII m_buffer;
    VKBufferHandleRAII m_uploadBuffer;

    virtual SLANG_NO_THROW DeviceAddress SLANG_MCALL getDeviceAddress() override;

    virtual SLANG_NO_THROW Result SLANG_MCALL
        getNativeResourceHandle(InteropHandle* outHandle) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL getSharedHandle(InteropHandle* outHandle) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL
        map(MemoryRange* rangeToRead, void** outPointer) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL unmap(MemoryRange* writtenRange) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL setDebugName(const char* name) override;
};

class FenceImpl : public FenceBase
{
public:
    VkSemaphore m_semaphore = VK_NULL_HANDLE;
    RefPtr<DeviceImpl> m_device;

    FenceImpl(DeviceImpl* device);

    ~FenceImpl();

    Result init(const IFence::Desc& desc);

    virtual SLANG_NO_THROW Result SLANG_MCALL getCurrentValue(uint64_t* outValue) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL setCurrentValue(uint64_t value) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL getSharedHandle(InteropHandle* outHandle) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL
        getNativeHandle(InteropHandle* outNativeHandle) override;
};

class TextureResourceImpl : public TextureResource
{
public:
    typedef TextureResource Parent;
    TextureResourceImpl(const Desc& desc, DeviceImpl* device);
    ~TextureResourceImpl();

    VkImage m_image = VK_NULL_HANDLE;
    VkFormat m_vkformat = VK_FORMAT_R8G8B8A8_UNORM;
    VkDeviceMemory m_imageMemory = VK_NULL_HANDLE;
    bool m_isWeakImageReference = false;
    RefPtr<DeviceImpl> m_device;

    virtual SLANG_NO_THROW Result SLANG_MCALL
        getNativeResourceHandle(InteropHandle* outHandle) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL getSharedHandle(InteropHandle* outHandle) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL setDebugName(const char* name) override;
};

class SamplerStateImpl : public SamplerStateBase
{
public:
    VkSampler m_sampler;
    RefPtr<DeviceImpl> m_device;
    SamplerStateImpl(DeviceImpl* device);
    ~SamplerStateImpl();
    virtual SLANG_NO_THROW Result SLANG_MCALL getNativeHandle(InteropHandle* outHandle) override;
};

class ResourceViewImpl : public ResourceViewBase
{
public:
    enum class ViewType
    {
        Texture,
        TexelBuffer,
        PlainBuffer,
    };

public:
    ResourceViewImpl(ViewType viewType, DeviceImpl* device)
        : m_type(viewType)
        , m_device(device)
    {}
    ViewType m_type;
    RefPtr<DeviceImpl> m_device;
};

class TextureResourceViewImpl : public ResourceViewImpl
{
public:
    TextureResourceViewImpl(DeviceImpl* device)
        : ResourceViewImpl(ViewType::Texture, device)
    {}
    ~TextureResourceViewImpl();
    RefPtr<TextureResourceImpl> m_texture;
    VkImageView m_view;
    VkImageLayout m_layout;

    virtual SLANG_NO_THROW Result SLANG_MCALL getNativeHandle(InteropHandle* outHandle) override;
};

class TexelBufferResourceViewImpl : public ResourceViewImpl
{
public:
    TexelBufferResourceViewImpl(DeviceImpl* device);
    ~TexelBufferResourceViewImpl();
    RefPtr<BufferResourceImpl> m_buffer;
    VkBufferView m_view;
    virtual SLANG_NO_THROW Result SLANG_MCALL getNativeHandle(InteropHandle* outHandle) override;
};

class PlainBufferResourceViewImpl : public ResourceViewImpl
{
public:
    PlainBufferResourceViewImpl(DeviceImpl* device);
    RefPtr<BufferResourceImpl> m_buffer;
    VkDeviceSize offset;
    VkDeviceSize size;

    virtual SLANG_NO_THROW Result SLANG_MCALL getNativeHandle(InteropHandle* outHandle) override;
};

class AccelerationStructureImpl : public AccelerationStructureBase
{
public:
    VkAccelerationStructureKHR m_vkHandle = VK_NULL_HANDLE;
    RefPtr<BufferResourceImpl> m_buffer;
    VkDeviceSize m_offset;
    VkDeviceSize m_size;
    RefPtr<DeviceImpl> m_device;

public:
    virtual SLANG_NO_THROW DeviceAddress SLANG_MCALL getDeviceAddress() override;
    virtual SLANG_NO_THROW Result SLANG_MCALL getNativeHandle(InteropHandle* outHandle) override;
    ~AccelerationStructureImpl();
};

class FramebufferLayoutImpl : public FramebufferLayoutBase
{
public:
    VkRenderPass m_renderPass;
    DeviceImpl* m_renderer;
    Array<VkAttachmentDescription, kMaxTargets> m_targetDescs;
    Array<VkAttachmentReference, kMaxRenderTargets> m_colorReferences;
    VkAttachmentReference m_depthReference;
    bool m_hasDepthStencilTarget;
    uint32_t m_renderTargetCount;
    VkSampleCountFlagBits m_sampleCount = VK_SAMPLE_COUNT_1_BIT;

public:
    ~FramebufferLayoutImpl();
    Result init(DeviceImpl* renderer, const IFramebufferLayout::Desc& desc);
};

class RenderPassLayoutImpl
    : public IRenderPassLayout
    , public ComObject
{
public:
    SLANG_COM_OBJECT_IUNKNOWN_ALL
    IRenderPassLayout* getInterface(const Guid& guid);

public:
    VkRenderPass m_renderPass;
    RefPtr<DeviceImpl> m_renderer;
    ~RenderPassLayoutImpl();

    Result init(DeviceImpl* renderer, const IRenderPassLayout::Desc& desc);
};

class FramebufferImpl : public FramebufferBase
{
public:
    VkFramebuffer m_handle;
    ShortList<ComPtr<IResourceView>> renderTargetViews;
    ComPtr<IResourceView> depthStencilView;
    uint32_t m_width;
    uint32_t m_height;
    BreakableReference<DeviceImpl> m_renderer;
    VkClearValue m_clearValues[kMaxTargets];
    RefPtr<FramebufferLayoutImpl> m_layout;

public:
    ~FramebufferImpl();

    Result init(DeviceImpl* renderer, const IFramebuffer::Desc& desc);
};

struct BoundVertexBuffer
{
    RefPtr<BufferResourceImpl> m_buffer;
    int m_offset;
};

class PipelineStateImpl : public PipelineStateBase
{
public:
    PipelineStateImpl(DeviceImpl* device);
    ~PipelineStateImpl();

    // Turns `m_device` into a strong reference.
    // This method should be called before returning the pipeline state object to
    // external users (i.e. via an `IPipelineState` pointer).
    void establishStrongDeviceReference();

    virtual void comFree() override;

    void init(const GraphicsPipelineStateDesc& inDesc);
    void init(const ComputePipelineStateDesc& inDesc);
    void init(const RayTracingPipelineStateDesc& inDesc);

    Result createVKGraphicsPipelineState();

    Result createVKComputePipelineState();

    virtual Result ensureAPIPipelineStateCreated() override;

    virtual SLANG_NO_THROW Result SLANG_MCALL getNativeHandle(InteropHandle* outHandle) override;

    BreakableReference<DeviceImpl> m_device;

    VkPipeline m_pipeline = VK_NULL_HANDLE;
};

class RayTracingPipelineStateImpl : public PipelineStateImpl
{
public:
    Dictionary<String, Index> shaderGroupNameToIndex;
    Int shaderGroupCount;

    RayTracingPipelineStateImpl(DeviceImpl* device);

    uint32_t findEntryPointIndexByName(
        const Dictionary<String, Index>& entryPointNameToIndex, const char* name);

    Result createVKRayTracingPipelineState();

    virtual Result ensureAPIPipelineStateCreated() override;

    virtual SLANG_NO_THROW Result SLANG_MCALL getNativeHandle(InteropHandle* outHandle) override;
};

// In order to bind shader parameters to the correct locations, we need to
// be able to describe those locations. Most shader parameters in Vulkan
// simply consume a single `binding`, but we also need to deal with
// parameters that represent push-constant ranges.
//
// In more complex cases we might be binding an entire "sub-object" like
// a parameter block, an entry point, etc. For the general case, we need
// to be able to represent a composite offset that includes offsets for
// each of the cases that Vulkan supports.

/// A "simple" binding offset that records `binding`, `set`, etc. offsets
struct SimpleBindingOffset
{
    /// An offset in GLSL/SPIR-V `binding`s
    uint32_t binding = 0;

    /// The descriptor `set` that the `binding` field should be understood as an index into
    uint32_t bindingSet = 0;

    /// The offset in push-constant ranges (not bytes)
    uint32_t pushConstantRange = 0;

    /// Create a default (zero) offset
    SimpleBindingOffset() {}

    /// Create an offset based on offset information in the given Slang `varLayout`
    SimpleBindingOffset(slang::VariableLayoutReflection* varLayout)
    {
        if (varLayout)
        {
            bindingSet = (uint32_t)varLayout->getBindingSpace(
                SLANG_PARAMETER_CATEGORY_DESCRIPTOR_TABLE_SLOT);
            binding =
                (uint32_t)varLayout->getOffset(SLANG_PARAMETER_CATEGORY_DESCRIPTOR_TABLE_SLOT);
            pushConstantRange =
                (uint32_t)varLayout->getOffset(SLANG_PARAMETER_CATEGORY_PUSH_CONSTANT_BUFFER);
        }
    }

    /// Add any values in the given `offset`
    void operator+=(SimpleBindingOffset const& offset)
    {
        binding += offset.binding;
        bindingSet += offset.bindingSet;
        pushConstantRange += offset.pushConstantRange;
    }
};

// While a "simple" binding offset representation will work in many cases,
// once we need to deal with layout for programs with interface-type parameters
// that have been statically specialized, we also need to track the offset
// for where to bind any "pending" data that arises from the process of static
// specialization.
//
// In order to conveniently track both the "primary" and "pending" offset information,
// we will define a more complete `BindingOffset` type that combines simple
// binding offsets for the primary and pending parts.

/// A representation of the offset at which to bind a shader parameter or sub-object
struct BindingOffset : SimpleBindingOffset
{
    // Offsets for "primary" data are stored directly in the `BindingOffset`
    // via the inheritance from `SimpleBindingOffset`.

    /// Offset for any "pending" data
    SimpleBindingOffset pending;

    /// Create a default (zero) offset
    BindingOffset() {}

    /// Create an offset from a simple offset
    explicit BindingOffset(SimpleBindingOffset const& offset)
        : SimpleBindingOffset(offset)
    {}

    /// Create an offset based on offset information in the given Slang `varLayout`
    BindingOffset(slang::VariableLayoutReflection* varLayout)
        : SimpleBindingOffset(varLayout)
        , pending(varLayout->getPendingDataLayout())
    {}

    /// Add any values in the given `offset`
    void operator+=(SimpleBindingOffset const& offset) { SimpleBindingOffset::operator+=(offset); }

    /// Add any values in the given `offset`
    void operator+=(BindingOffset const& offset)
    {
        SimpleBindingOffset::operator+=(offset);
        pending += offset.pending;
    }
};

class ShaderObjectLayoutImpl : public ShaderObjectLayoutBase
{
public:
    // A shader object comprises three main kinds of state:
    //
    // * Zero or more bytes of ordinary ("uniform") data
    // * Zero or more *bindings* for textures, buffers, and samplers
    // * Zero or more *sub-objects* representing nested parameter blocks, etc.
    //
    // A shader object *layout* stores information that can be used to
    // organize these different kinds of state and optimize access to them.
    //
    // For example, both texture/buffer/sampler bindings and sub-objects
    // are organized into logical *binding ranges* by the Slang reflection
    // API, and a shader object layout will store information about those
    // ranges in a form that is usable for the Vulkan API:

    struct BindingRangeInfo
    {
        slang::BindingType bindingType;
        Index count;
        Index baseIndex;

        /// An index into the sub-object array if this binding range is treated
        /// as a sub-object.
        Index subObjectIndex;

        /// The `binding` offset to apply for this range
        uint32_t bindingOffset;

        /// The `set` offset to apply for this range
        uint32_t setOffset;

        // Note: The 99% case is that `setOffset` will be zero. For any shader object
        // that was allocated from an ordinary Slang type (anything other than a root
        // shader object in fact), all of the bindings will have been allocated into
        // a single logical descriptor set.
        //
        // TODO: Ideally we could refactor so that only the root shader object layout
        // stores a set offset for its binding ranges, and all other objects skip
        // storing a field that never actually matters.
    };

    // Sometimes we just want to iterate over the ranges that represnet
    // sub-objects while skipping over the others, because sub-object
    // ranges often require extra handling or more state.
    //
    // For that reason we also store pre-computed information about each
    // sub-object range.

    /// Offset information for a sub-object range
    struct SubObjectRangeOffset : BindingOffset
    {
        SubObjectRangeOffset() {}

        SubObjectRangeOffset(slang::VariableLayoutReflection* varLayout)
            : BindingOffset(varLayout)
        {
            if (auto pendingLayout = varLayout->getPendingDataLayout())
            {
                pendingOrdinaryData =
                    (uint32_t)pendingLayout->getOffset(SLANG_PARAMETER_CATEGORY_UNIFORM);
            }
        }

        /// The offset for "pending" ordinary data related to this range
        uint32_t pendingOrdinaryData = 0;
    };

    /// Stride information for a sub-object range
    struct SubObjectRangeStride : BindingOffset
    {
        SubObjectRangeStride() {}

        SubObjectRangeStride(slang::TypeLayoutReflection* typeLayout)
        {
            if (auto pendingLayout = typeLayout->getPendingDataTypeLayout())
            {
                pendingOrdinaryData = (uint32_t)pendingLayout->getStride();
            }
        }

        /// The strid for "pending" ordinary data related to this range
        uint32_t pendingOrdinaryData = 0;
    };

    /// Information about a logical binding range as reported by Slang reflection
    struct SubObjectRangeInfo
    {
        /// The index of the binding range that corresponds to this sub-object range
        Index bindingRangeIndex;

        /// The layout expected for objects bound to this range (if known)
        RefPtr<ShaderObjectLayoutImpl> layout;

        /// The offset to use when binding the first object in this range
        SubObjectRangeOffset offset;

        /// Stride between consecutive objects in this range
        SubObjectRangeStride stride;
    };

    struct DescriptorSetInfo
    {
        List<VkDescriptorSetLayoutBinding> vkBindings;
        Slang::Int space = -1;
        VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    };

    struct Builder
    {
    public:
        Builder(DeviceImpl* renderer)
            : m_renderer(renderer)
        {}

        DeviceImpl* m_renderer;
        slang::TypeLayoutReflection* m_elementTypeLayout;

        /// The container type of this shader object. When `m_containerType` is
        /// `StructuredBuffer` or `UnsizedArray`, this shader object represents a collection
        /// instead of a single object.
        ShaderObjectContainerType m_containerType = ShaderObjectContainerType::None;

        List<BindingRangeInfo> m_bindingRanges;
        List<SubObjectRangeInfo> m_subObjectRanges;

        Index m_resourceViewCount = 0;
        Index m_samplerCount = 0;
        Index m_combinedTextureSamplerCount = 0;
        Index m_subObjectCount = 0;
        Index m_varyingInputCount = 0;
        Index m_varyingOutputCount = 0;
        List<DescriptorSetInfo> m_descriptorSetBuildInfos;
        Dictionary<Index, Index> m_mapSpaceToDescriptorSetIndex;

        /// The number of descriptor sets allocated by child/descendent objects
        uint32_t m_childDescriptorSetCount = 0;

        /// The total number of `binding`s consumed by this object and its children/descendents
        uint32_t m_totalBindingCount = 0;

        /// The push-constant ranges that belong to this object itself (if any)
        List<VkPushConstantRange> m_ownPushConstantRanges;

        /// The number of push-constant ranges owned by child/descendent objects
        uint32_t m_childPushConstantRangeCount = 0;

        uint32_t m_totalOrdinaryDataSize = 0;

        Index findOrAddDescriptorSet(Index space);

        static VkDescriptorType _mapDescriptorType(slang::BindingType slangBindingType);

        /// Add any descriptor ranges implied by this object containing a leaf
        /// sub-object described by `typeLayout`, at the given `offset`.
        void _addDescriptorRangesAsValue(
            slang::TypeLayoutReflection* typeLayout, BindingOffset const& offset);

        /// Add the descriptor ranges implied by a `ConstantBuffer<X>` where `X` is
        /// described by `elementTypeLayout`.
        ///
        /// The `containerOffset` and `elementOffset` are the binding offsets that
        /// should apply to the buffer itself and the contents of the buffer, respectively.
        ///
        void _addDescriptorRangesAsConstantBuffer(
            slang::TypeLayoutReflection* elementTypeLayout,
            BindingOffset const& containerOffset,
            BindingOffset const& elementOffset);

        /// Add the descriptor ranges implied by a `PushConstantBuffer<X>` where `X` is
        /// described by `elementTypeLayout`.
        ///
        /// The `containerOffset` and `elementOffset` are the binding offsets that
        /// should apply to the buffer itself and the contents of the buffer, respectively.
        ///
        void _addDescriptorRangesAsPushConstantBuffer(
            slang::TypeLayoutReflection* elementTypeLayout,
            BindingOffset const& containerOffset,
            BindingOffset const& elementOffset);

        /// Add binding ranges to this shader object layout, as implied by the given
        /// `typeLayout`
        void addBindingRanges(slang::TypeLayoutReflection* typeLayout);

        Result setElementTypeLayout(slang::TypeLayoutReflection* typeLayout);

        SlangResult build(ShaderObjectLayoutImpl** outLayout);
    };

    static Result createForElementType(
        DeviceImpl* renderer,
        slang::TypeLayoutReflection* elementType,
        ShaderObjectLayoutImpl** outLayout);

    ~ShaderObjectLayoutImpl();

    /// Get the number of descriptor sets that are allocated for this object itself
    /// (if it needed to be bound as a parameter block).
    ///
    uint32_t getOwnDescriptorSetCount() { return uint32_t(m_descriptorSetInfos.getCount()); }

    /// Get information about the descriptor sets that would be allocated to
    /// represent this object itself as a parameter block.
    ///
    List<DescriptorSetInfo> const& getOwnDescriptorSets() { return m_descriptorSetInfos; }

    /// Get the number of descriptor sets that would need to be allocated and bound
    /// to represent the children of this object if it were bound as a parameter
    /// block.
    ///
    /// To a first approximation, this is the number of (transitive) children
    /// that are declared as `ParameterBlock<X>`.
    ///
    uint32_t getChildDescriptorSetCount() { return m_childDescriptorSetCount; }

    /// Get the total number of descriptor sets that would need to be allocated and bound
    /// to represent this object and its children (transitively) as a parameter block.
    ///
    uint32_t getTotalDescriptorSetCount()
    {
        return getOwnDescriptorSetCount() + getChildDescriptorSetCount();
    }

    /// Get the total number of `binding`s required to represent this type and its
    /// (transitive) children.
    ///
    /// Note that this count does *not* include bindings that would be part of child
    /// parameter blocks, nor does it include the binding for an ordinary data buffer,
    /// if one is needed.
    ///
    uint32_t getTotalBindingCount() { return m_totalBindingCount; }

    /// Get the list of push constant ranges required to bind the state of this object itself.
    List<VkPushConstantRange> const& getOwnPushConstantRanges() const
    {
        return m_ownPushConstantRanges;
    }

    /// Get the number of push constant ranges required to bind the state of this object itself.
    uint32_t getOwnPushConstantRangeCount() { return (uint32_t)m_ownPushConstantRanges.getCount(); }

    /// Get the number of push constant ranges required to bind the state of the (transitive)
    /// children of this object.
    uint32_t getChildPushConstantRangeCount() { return m_childPushConstantRangeCount; }

    /// Get the total number of push constant ranges required to bind the state of this object
    /// and its (transitive) children.
    uint32_t getTotalPushConstantRangeCount()
    {
        return getOwnPushConstantRangeCount() + getChildPushConstantRangeCount();
    }

    uint32_t getTotalOrdinaryDataSize() const { return m_totalOrdinaryDataSize; }

    List<BindingRangeInfo> const& getBindingRanges() { return m_bindingRanges; }

    Index getBindingRangeCount() { return m_bindingRanges.getCount(); }

    BindingRangeInfo const& getBindingRange(Index index) { return m_bindingRanges[index]; }

    Index getResourceViewCount() { return m_resourceViewCount; }
    Index getSamplerCount() { return m_samplerCount; }
    Index getCombinedTextureSamplerCount() { return m_combinedTextureSamplerCount; }
    Index getSubObjectCount() { return m_subObjectCount; }

    SubObjectRangeInfo const& getSubObjectRange(Index index) { return m_subObjectRanges[index]; }
    List<SubObjectRangeInfo> const& getSubObjectRanges() { return m_subObjectRanges; }

    DeviceImpl* getDevice() { return static_cast<DeviceImpl*>(m_renderer); }

    slang::TypeReflection* getType() { return m_elementTypeLayout->getType(); }

protected:
    Result _init(Builder const* builder);

    List<DescriptorSetInfo> m_descriptorSetInfos;
    List<BindingRangeInfo> m_bindingRanges;
    Index m_resourceViewCount = 0;
    Index m_samplerCount = 0;
    Index m_combinedTextureSamplerCount = 0;
    Index m_subObjectCount = 0;
    List<VkPushConstantRange> m_ownPushConstantRanges;
    uint32_t m_childPushConstantRangeCount = 0;

    uint32_t m_childDescriptorSetCount = 0;
    uint32_t m_totalBindingCount = 0;
    uint32_t m_totalOrdinaryDataSize = 0;

    List<SubObjectRangeInfo> m_subObjectRanges;
};

class EntryPointLayout : public ShaderObjectLayoutImpl
{
    typedef ShaderObjectLayoutImpl Super;

public:
    struct Builder : Super::Builder
    {
        Builder(DeviceImpl* device)
            : Super::Builder(device)
        {}

        Result build(EntryPointLayout** outLayout);

        void addEntryPointParams(slang::EntryPointLayout* entryPointLayout);

        slang::EntryPointLayout* m_slangEntryPointLayout = nullptr;

        VkShaderStageFlags m_shaderStageFlag;
    };

    Result _init(Builder const* builder);

    VkShaderStageFlags getShaderStageFlag() const { return m_shaderStageFlag; }

    slang::EntryPointLayout* getSlangLayout() const { return m_slangEntryPointLayout; };

    slang::EntryPointLayout* m_slangEntryPointLayout;
    VkShaderStageFlags m_shaderStageFlag;
};

class RootShaderObjectLayout : public ShaderObjectLayoutImpl
{
    typedef ShaderObjectLayoutImpl Super;

public:
    ~RootShaderObjectLayout();

    /// Information stored for each entry point of the program
    struct EntryPointInfo
    {
        /// Layout of the entry point
        RefPtr<EntryPointLayout> layout;

        /// Offset for binding the entry point, relative to the start of the program
        BindingOffset offset;
    };

    struct Builder : Super::Builder
    {
        Builder(
            DeviceImpl* renderer,
            slang::IComponentType* program,
            slang::ProgramLayout* programLayout)
            : Super::Builder(renderer)
            , m_program(program)
            , m_programLayout(programLayout)
        {}

        Result build(RootShaderObjectLayout** outLayout);

        void addGlobalParams(slang::VariableLayoutReflection* globalsLayout);

        void addEntryPoint(EntryPointLayout* entryPointLayout);

        slang::IComponentType* m_program;
        slang::ProgramLayout* m_programLayout;
        List<EntryPointInfo> m_entryPoints;

        /// Offset to apply to "pending" data from this object, sub-objects, and entry points
        SimpleBindingOffset m_pendingDataOffset;
    };

    Index findEntryPointIndex(VkShaderStageFlags stage);

    EntryPointInfo const& getEntryPoint(Index index) { return m_entryPoints[index]; }

    List<EntryPointInfo> const& getEntryPoints() const { return m_entryPoints; }

    static Result create(
        DeviceImpl* renderer,
        slang::IComponentType* program,
        slang::ProgramLayout* programLayout,
        RootShaderObjectLayout** outLayout);

    SimpleBindingOffset const& getPendingDataOffset() const { return m_pendingDataOffset; }

    slang::IComponentType* getSlangProgram() const { return m_program; }
    slang::ProgramLayout* getSlangProgramLayout() const { return m_programLayout; }

    /// Get all of the push constant ranges that will be bound for this object and all
    /// (transitive) sub-objects
    List<VkPushConstantRange> const& getAllPushConstantRanges() { return m_allPushConstantRanges; }

protected:
    Result _init(Builder const* builder);

    /// Add all the descriptor sets implied by this root object and sub-objects
    Result addAllDescriptorSets();

    /// Recurisvely add descriptor sets defined by `layout` and sub-objects
    Result addAllDescriptorSetsRec(ShaderObjectLayoutImpl* layout);

    /// Recurisvely add descriptor sets defined by sub-objects of `layout`
    Result addChildDescriptorSetsRec(ShaderObjectLayoutImpl* layout);

    /// Add all the push-constant ranges implied by this root object and sub-objects
    Result addAllPushConstantRanges();

    /// Recurisvely add push-constant ranges defined by `layout` and sub-objects
    Result addAllPushConstantRangesRec(ShaderObjectLayoutImpl* layout);

    /// Recurisvely add push-constant ranges defined by sub-objects of `layout`
    Result addChildPushConstantRangesRec(ShaderObjectLayoutImpl* layout);

public:
    ComPtr<slang::IComponentType> m_program;
    slang::ProgramLayout* m_programLayout = nullptr;
    List<EntryPointInfo> m_entryPoints;
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    Array<VkDescriptorSetLayout, kMaxDescriptorSets> m_vkDescriptorSetLayouts;
    List<VkPushConstantRange> m_allPushConstantRanges;
    uint32_t m_totalPushConstantSize = 0;

    SimpleBindingOffset m_pendingDataOffset;
    DeviceImpl* m_renderer = nullptr;
};

class ShaderProgramImpl : public ShaderProgramBase
{
public:
    ShaderProgramImpl(DeviceImpl* device);

    ~ShaderProgramImpl();

    virtual void comFree() override;

    BreakableReference<DeviceImpl> m_device;

    Array<VkPipelineShaderStageCreateInfo, 8> m_stageCreateInfos;
    Array<String, 8> m_entryPointNames;
    Array<ComPtr<ISlangBlob>, 8> m_codeBlobs; //< To keep storage of code in scope
    Array<VkShaderModule, 8> m_modules;
    RefPtr<RootShaderObjectLayout> m_rootObjectLayout;

    VkPipelineShaderStageCreateInfo compileEntryPoint(
        const char* entryPointName,
        ISlangBlob* code,
        VkShaderStageFlagBits stage,
        VkShaderModule& outShaderModule);

    virtual Result createShaderModule(
        slang::EntryPointReflection* entryPointInfo, ComPtr<ISlangBlob> kernelCode) override;
};

class CommandBufferImpl;

class PipelineCommandEncoder : public RefObject
{
public:
    CommandBufferImpl* m_commandBuffer;
    VkCommandBuffer m_vkCommandBuffer;
    VkCommandBuffer m_vkPreCommandBuffer = VK_NULL_HANDLE;
    VkPipeline m_boundPipelines[3] = {};
    DeviceImpl* m_device = nullptr;
    RefPtr<PipelineStateImpl> m_currentPipeline;

    VulkanApi* m_api;

    static int getBindPointIndex(VkPipelineBindPoint bindPoint);

    void init(CommandBufferImpl* commandBuffer);

    void endEncodingImpl();

    static void _uploadBufferData(
        VkCommandBuffer commandBuffer,
        TransientResourceHeapImpl* transientHeap,
        BufferResourceImpl* buffer,
        Offset offset,
        Size size,
        void* data);

    void uploadBufferDataImpl(IBufferResource* buffer, Offset offset, Size size, void* data);

    Result bindRootShaderObjectImpl(VkPipelineBindPoint bindPoint);

    Result setPipelineStateImpl(IPipelineState* state, IShaderObject** outRootObject);

    Result setPipelineStateWithRootObjectImpl(IPipelineState* state, IShaderObject* inObject);

    void bindRenderState(VkPipelineBindPoint pipelineBindPoint);
};

/// Context information required when binding shader objects to the pipeline
struct RootBindingContext
{
    /// The pipeline layout being used for binding
    VkPipelineLayout pipelineLayout;

    /// An allocator to use for descriptor sets during binding
    DescriptorSetAllocator* descriptorSetAllocator;

    /// The dvice being used
    DeviceImpl* device;

    /// The descriptor sets that are being allocated and bound
    VkDescriptorSet* descriptorSets;

    /// Information about all the push-constant ranges that should be bound
    ConstArrayView<VkPushConstantRange> pushConstantRanges;

    uint32_t descriptorSetCounter = 0;
};

struct CombinedTextureSamplerSlot
{
    RefPtr<TextureResourceViewImpl> textureView;
    RefPtr<SamplerStateImpl> sampler;
    operator bool() { return textureView && sampler; }
};

class ShaderObjectImpl
    : public ShaderObjectBaseImpl<ShaderObjectImpl, ShaderObjectLayoutImpl, SimpleShaderObjectData>
{
public:
    static Result create(
        IDevice* device, ShaderObjectLayoutImpl* layout, ShaderObjectImpl** outShaderObject);

    RendererBase* getDevice();

    virtual SLANG_NO_THROW GfxCount SLANG_MCALL getEntryPointCount() override;

    virtual SLANG_NO_THROW Result SLANG_MCALL
        getEntryPoint(GfxIndex index, IShaderObject** outEntryPoint) override;

    virtual SLANG_NO_THROW const void* SLANG_MCALL getRawData() override;

    virtual SLANG_NO_THROW Size SLANG_MCALL getSize() override;

    // TODO: Changed size_t to Size? inSize assigned to an Index variable inside implementation
    virtual SLANG_NO_THROW Result SLANG_MCALL
        setData(ShaderOffset const& inOffset, void const* data, size_t inSize) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL
        setResource(ShaderOffset const& offset, IResourceView* resourceView) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL
        setSampler(ShaderOffset const& offset, ISamplerState* sampler) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL setCombinedTextureSampler(
        ShaderOffset const& offset, IResourceView* textureView, ISamplerState* sampler) override;

protected:
    friend class RootShaderObjectLayout;

    Result init(IDevice* device, ShaderObjectLayoutImpl* layout);

    /// Write the uniform/ordinary data of this object into the given `dest` buffer at the given
    /// `offset`
    Result _writeOrdinaryData(
        PipelineCommandEncoder* encoder,
        IBufferResource* buffer,
        Offset offset,
        Size destSize,
        ShaderObjectLayoutImpl* specializedLayout);

public:
    /// Write a single desriptor using the Vulkan API
    static void writeDescriptor(RootBindingContext& context, VkWriteDescriptorSet const& write);

    static void writeBufferDescriptor(
        RootBindingContext& context,
        BindingOffset const& offset,
        VkDescriptorType descriptorType,
        BufferResourceImpl* buffer,
        Offset bufferOffset,
        Size bufferSize);

    static void writeBufferDescriptor(
        RootBindingContext& context,
        BindingOffset const& offset,
        VkDescriptorType descriptorType,
        BufferResourceImpl* buffer);

    static void writePlainBufferDescriptor(
        RootBindingContext& context,
        BindingOffset const& offset,
        VkDescriptorType descriptorType,
        ArrayView<RefPtr<ResourceViewInternalBase>> resourceViews);

    static void writeTexelBufferDescriptor(
        RootBindingContext& context,
        BindingOffset const& offset,
        VkDescriptorType descriptorType,
        ArrayView<RefPtr<ResourceViewInternalBase>> resourceViews);

    static void writeTextureSamplerDescriptor(
        RootBindingContext& context,
        BindingOffset const& offset,
        VkDescriptorType descriptorType,
        ArrayView<CombinedTextureSamplerSlot> slots);

    static void writeAccelerationStructureDescriptor(
        RootBindingContext& context,
        BindingOffset const& offset,
        VkDescriptorType descriptorType,
        ArrayView<RefPtr<ResourceViewInternalBase>> resourceViews);

    static void writeTextureDescriptor(
        RootBindingContext& context,
        BindingOffset const& offset,
        VkDescriptorType descriptorType,
        ArrayView<RefPtr<ResourceViewInternalBase>> resourceViews);

    static void writeSamplerDescriptor(
        RootBindingContext& context,
        BindingOffset const& offset,
        VkDescriptorType descriptorType,
        ArrayView<RefPtr<SamplerStateImpl>> samplers);

    bool shouldAllocateConstantBuffer(TransientResourceHeapImpl* transientHeap);

    /// Ensure that the `m_ordinaryDataBuffer` has been created, if it is needed
    Result _ensureOrdinaryDataBufferCreatedIfNeeded(
        PipelineCommandEncoder* encoder, ShaderObjectLayoutImpl* specializedLayout);

public:
    /// Bind this shader object as a "value"
    ///
    /// This is the mode used for binding sub-objects for existential-type
    /// fields, and is also used as part of the implementation of the
    /// parameter-block and constant-buffer cases.
    ///
    Result bindAsValue(
        PipelineCommandEncoder* encoder,
        RootBindingContext& context,
        BindingOffset const& offset,
        ShaderObjectLayoutImpl* specializedLayout);

    /// Allocate the descriptor sets needed for binding this object (but not nested parameter
    /// blocks)
    Result allocateDescriptorSets(
        PipelineCommandEncoder* encoder,
        RootBindingContext& context,
        BindingOffset const& offset,
        ShaderObjectLayoutImpl* specializedLayout);

    /// Bind this object as a `ParameterBlock<X>`.
    Result bindAsParameterBlock(
        PipelineCommandEncoder* encoder,
        RootBindingContext& context,
        BindingOffset const& inOffset,
        ShaderObjectLayoutImpl* specializedLayout);

    /// Bind the ordinary data buffer if needed.
    Result bindOrdinaryDataBufferIfNeeded(
        PipelineCommandEncoder* encoder,
        RootBindingContext& context,
        BindingOffset& ioOffset,
        ShaderObjectLayoutImpl* specializedLayout);

    /// Bind this object as a `ConstantBuffer<X>`.
    Result bindAsConstantBuffer(
        PipelineCommandEncoder* encoder,
        RootBindingContext& context,
        BindingOffset const& inOffset,
        ShaderObjectLayoutImpl* specializedLayout);

    List<RefPtr<ResourceViewInternalBase>> m_resourceViews;

    List<RefPtr<SamplerStateImpl>> m_samplers;

    List<CombinedTextureSamplerSlot> m_combinedTextureSamplers;

    // The transient constant buffer that holds the GPU copy of the constant data,
    // weak referenced.
    IBufferResource* m_constantBuffer = nullptr;
    // The offset into the transient constant buffer where the constant data starts.
    Offset m_constantBufferOffset = 0;
    Size m_constantBufferSize = 0;

    /// Dirty bit tracking whether the constant buffer needs to be updated.
    bool m_isConstantBufferDirty = true;
    /// The transient heap from which the constant buffer is allocated.
    TransientResourceHeapImpl* m_constantBufferTransientHeap;
    /// The version of the transient heap when the constant buffer is allocated.
    uint64_t m_constantBufferTransientHeapVersion;

    /// Get the layout of this shader object with specialization arguments considered
    ///
    /// This operation should only be called after the shader object has been
    /// fully filled in and finalized.
    ///
    Result _getSpecializedLayout(ShaderObjectLayoutImpl** outLayout);

    /// Create the layout for this shader object with specialization arguments considered
    ///
    /// This operation is virtual so that it can be customized by `ProgramVars`.
    ///
    virtual Result _createSpecializedLayout(ShaderObjectLayoutImpl** outLayout);

    RefPtr<ShaderObjectLayoutImpl> m_specializedLayout;
};

class MutableShaderObjectImpl
    : public MutableShaderObject<MutableShaderObjectImpl, ShaderObjectLayoutImpl>
{};

class EntryPointShaderObject : public ShaderObjectImpl
{
    typedef ShaderObjectImpl Super;

public:
    static Result create(
        IDevice* device, EntryPointLayout* layout, EntryPointShaderObject** outShaderObject);

    EntryPointLayout* getLayout();

    /// Bind this shader object as an entry point
    Result bindAsEntryPoint(
        PipelineCommandEncoder* encoder,
        RootBindingContext& context,
        BindingOffset const& inOffset,
        EntryPointLayout* layout);

protected:
    Result init(IDevice* device, EntryPointLayout* layout);
};

class RootShaderObjectImpl : public ShaderObjectImpl
{
    using Super = ShaderObjectImpl;

public:
    // Override default reference counting behavior to disable lifetime management.
    // Root objects are managed by command buffer and does not need to be freed by the user.
    virtual SLANG_NO_THROW uint32_t SLANG_MCALL addRef() override { return 1; }
    virtual SLANG_NO_THROW uint32_t SLANG_MCALL release() override { return 1; }

public:
    RootShaderObjectLayout* getLayout();

    RootShaderObjectLayout* getSpecializedLayout();

    List<RefPtr<EntryPointShaderObject>> const& getEntryPoints() const;

    virtual GfxCount SLANG_MCALL getEntryPointCount() override;
    virtual Result SLANG_MCALL getEntryPoint(GfxIndex index, IShaderObject** outEntryPoint) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL
        copyFrom(IShaderObject* object, ITransientResourceHeap* transientHeap) override;

    /// Bind this object as a root shader object
    Result bindAsRoot(
        PipelineCommandEncoder* encoder,
        RootBindingContext& context,
        RootShaderObjectLayout* layout);

    virtual Result collectSpecializationArgs(ExtendedShaderObjectTypeList& args) override;

public:
    Result init(IDevice* device, RootShaderObjectLayout* layout);

protected:
    virtual Result _createSpecializedLayout(ShaderObjectLayoutImpl** outLayout) override;

    List<RefPtr<EntryPointShaderObject>> m_entryPoints;
};

class ShaderTableImpl : public ShaderTableBase
{
public:
    uint32_t m_raygenTableSize;
    uint32_t m_missTableSize;
    uint32_t m_hitTableSize;
    uint32_t m_callableTableSize;

    DeviceImpl* m_device;

    virtual RefPtr<BufferResource> createDeviceBuffer(
        PipelineStateBase* pipeline,
        TransientResourceHeapBase* transientHeap,
        IResourceCommandEncoder* encoder) override;
};

class ResourceCommandEncoder
    : public IResourceCommandEncoder
    , public PipelineCommandEncoder
{
public:
    virtual SLANG_NO_THROW void SLANG_MCALL copyBuffer(
        IBufferResource* dst,
        Offset dstOffset,
        IBufferResource* src,
        Offset srcOffset,
        Size size) override;
    virtual SLANG_NO_THROW void SLANG_MCALL
        uploadBufferData(IBufferResource* buffer, Offset offset, Size size, void* data) override;
    virtual SLANG_NO_THROW void SLANG_MCALL textureBarrier(
        GfxCount count,
        ITextureResource* const* textures,
        ResourceState src,
        ResourceState dst) override;
    virtual SLANG_NO_THROW void SLANG_MCALL bufferBarrier(
        GfxCount count,
        IBufferResource* const* buffers,
        ResourceState src,
        ResourceState dst) override;
    virtual SLANG_NO_THROW void SLANG_MCALL endEncoding() override;

    virtual SLANG_NO_THROW void SLANG_MCALL
        writeTimestamp(IQueryPool* queryPool, GfxIndex index) override;

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
        ITextureResource::Extents extend,
        ITextureResource::SubresourceData* subResourceData,
        GfxCount subResourceDataCount) override;

    void _clearColorImage(TextureResourceViewImpl* viewImpl, ClearValue* clearValue);

    void _clearDepthImage(
        TextureResourceViewImpl* viewImpl,
        ClearValue* clearValue,
        ClearResourceViewFlags::Enum flags);

    void _clearBuffer(
        VkBuffer buffer, uint64_t bufferSize, const IResourceView::Desc& desc, uint32_t clearValue);

    virtual SLANG_NO_THROW void SLANG_MCALL clearResourceView(
        IResourceView* view, ClearValue* clearValue, ClearResourceViewFlags::Enum flags) override;

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
    virtual SLANG_NO_THROW void SLANG_MCALL endDebugEvent() override;
};

class RenderCommandEncoder
    : public IRenderCommandEncoder
    , public ResourceCommandEncoder
{
public:
    SLANG_GFX_FORWARD_RESOURCE_COMMAND_ENCODER_IMPL(ResourceCommandEncoder)
public:
    List<VkViewport> m_viewports;
    List<VkRect2D> m_scissorRects;

public:
    void beginPass(IRenderPassLayout* renderPass, IFramebuffer* framebuffer);

    virtual SLANG_NO_THROW void SLANG_MCALL endEncoding() override;

    virtual SLANG_NO_THROW Result SLANG_MCALL
        bindPipeline(IPipelineState* pipelineState, IShaderObject** outRootObject) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL bindPipelineWithRootObject(
        IPipelineState* pipelineState, IShaderObject* rootObject) override;

    virtual SLANG_NO_THROW void SLANG_MCALL
        setViewports(GfxCount count, const Viewport* viewports) override;

    virtual SLANG_NO_THROW void SLANG_MCALL
        setScissorRects(GfxCount count, const ScissorRect* rects) override;

    virtual SLANG_NO_THROW void SLANG_MCALL
        setPrimitiveTopology(PrimitiveTopology topology) override;

    virtual SLANG_NO_THROW void SLANG_MCALL setVertexBuffers(
        GfxIndex startSlot,
        GfxCount slotCount,
        IBufferResource* const* buffers,
        const Offset* offsets) override;

    virtual SLANG_NO_THROW void SLANG_MCALL
        setIndexBuffer(IBufferResource* buffer, Format indexFormat, Offset offset = 0) override;

    void prepareDraw();

    virtual SLANG_NO_THROW void SLANG_MCALL
        draw(GfxCount vertexCount, GfxIndex startVertex = 0) override;
    virtual SLANG_NO_THROW void SLANG_MCALL
        drawIndexed(GfxCount indexCount, GfxIndex startIndex = 0, GfxIndex baseVertex = 0) override;

    virtual SLANG_NO_THROW void SLANG_MCALL setStencilReference(uint32_t referenceValue) override;

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
};

class ComputeCommandEncoder
    : public IComputeCommandEncoder
    , public ResourceCommandEncoder
{
public:
    SLANG_GFX_FORWARD_RESOURCE_COMMAND_ENCODER_IMPL(ResourceCommandEncoder)
public:
    virtual SLANG_NO_THROW void SLANG_MCALL endEncoding() override;

    virtual SLANG_NO_THROW Result SLANG_MCALL
        bindPipeline(IPipelineState* pipelineState, IShaderObject** outRootObject) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL bindPipelineWithRootObject(
        IPipelineState* pipelineState, IShaderObject* rootObject) override;

    virtual SLANG_NO_THROW void SLANG_MCALL dispatchCompute(int x, int y, int z) override;

    virtual SLANG_NO_THROW void SLANG_MCALL
        dispatchComputeIndirect(IBufferResource* argBuffer, Offset offset) override;
};

class RayTracingCommandEncoder
    : public IRayTracingCommandEncoder
    , public ResourceCommandEncoder
{
public:
    SLANG_GFX_FORWARD_RESOURCE_COMMAND_ENCODER_IMPL(ResourceCommandEncoder)
public:
    void _memoryBarrier(
        int count,
        IAccelerationStructure* const* structures,
        AccessFlag srcAccess,
        AccessFlag destAccess);

    void _queryAccelerationStructureProperties(
        GfxCount accelerationStructureCount,
        IAccelerationStructure* const* accelerationStructures,
        GfxCount queryCount,
        AccelerationStructureQueryDesc* queryDescs);

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
        IAccelerationStructure* dest, DeviceAddress source) override;

    virtual SLANG_NO_THROW void SLANG_MCALL
        bindPipeline(IPipelineState* pipeline, IShaderObject** outRootObject) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL bindPipelineWithRootObject(
        IPipelineState* pipelineState, IShaderObject* rootObject) override;

    virtual SLANG_NO_THROW void SLANG_MCALL dispatchRays(
        GfxIndex raygenShaderIndex,
        IShaderTable* shaderTable,
        GfxCount width,
        GfxCount height,
        GfxCount depth) override;

    virtual SLANG_NO_THROW void SLANG_MCALL endEncoding() override;
};

class CommandBufferImpl
    : public ICommandBuffer
    , public ComObject
{
public:
    // There are a pair of cyclic references between a `TransientResourceHeap` and
    // a `CommandBuffer` created from the heap. We need to break the cycle when
    // the public reference count of a command buffer drops to 0.
    SLANG_COM_OBJECT_IUNKNOWN_ALL
    ICommandBuffer* getInterface(const Guid& guid);
    virtual void comFree() override;

public:
    VkCommandBuffer m_commandBuffer;
    VkCommandBuffer m_preCommandBuffer = VK_NULL_HANDLE;
    VkCommandPool m_pool;
    DeviceImpl* m_renderer;
    BreakableReference<TransientResourceHeapImpl> m_transientHeap;
    bool m_isPreCommandBufferEmpty = true;
    RootShaderObjectImpl m_rootObject;

    RefPtr<ResourceCommandEncoder> m_resourceCommandEncoder;
    RefPtr<ComputeCommandEncoder> m_computeCommandEncoder;
    RefPtr<RenderCommandEncoder> m_renderCommandEncoder;
    RefPtr<RayTracingCommandEncoder> m_rayTracingCommandEncoder;

    // Command buffers are deallocated by its command pool,
    // so no need to free individually.
    ~CommandBufferImpl() = default;

    Result init(DeviceImpl* renderer, VkCommandPool pool, TransientResourceHeapImpl* transientHeap);

    void beginCommandBuffer();

    Result createPreCommandBuffer();

    VkCommandBuffer getPreCommandBuffer();

public:
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
};

class CommandQueueImpl
    : public ICommandQueue
    , public ComObject
{
public:
    SLANG_COM_OBJECT_IUNKNOWN_ALL
    ICommandQueue* getInterface(const Guid& guid);

public:
    Desc m_desc;
    RefPtr<DeviceImpl> m_renderer;
    VkQueue m_queue;
    uint32_t m_queueFamilyIndex;
    struct FenceWaitInfo
    {
        RefPtr<FenceImpl> fence;
        uint64_t waitValue;
    };
    List<FenceWaitInfo> m_pendingWaitFences;
    VkSemaphore m_pendingWaitSemaphores[2] = {VK_NULL_HANDLE, VK_NULL_HANDLE};
    List<VkCommandBuffer> m_submitCommandBuffers;
    VkSemaphore m_semaphore;
    ~CommandQueueImpl();

    void init(DeviceImpl* renderer, VkQueue queue, uint32_t queueFamilyIndex);

    virtual SLANG_NO_THROW void SLANG_MCALL waitOnHost() override;

    virtual SLANG_NO_THROW Result SLANG_MCALL getNativeHandle(InteropHandle* outHandle) override;

    virtual SLANG_NO_THROW const Desc& SLANG_MCALL getDesc() override;

    virtual SLANG_NO_THROW Result SLANG_MCALL waitForFenceValuesOnDevice(
        GfxCount fenceCount, IFence** fences, uint64_t* waitValues) override;

    void queueSubmitImpl(
        uint32_t count,
        ICommandBuffer* const* commandBuffers,
        IFence* fence,
        uint64_t valueToSignal);

    virtual SLANG_NO_THROW void SLANG_MCALL executeCommandBuffers(
        GfxCount count,
        ICommandBuffer* const* commandBuffers,
        IFence* fence,
        uint64_t valueToSignal) override;
};

class TransientResourceHeapImpl
    : public TransientResourceHeapBaseImpl<DeviceImpl, BufferResourceImpl>
{
private:
    typedef TransientResourceHeapBaseImpl<DeviceImpl, BufferResourceImpl> Super;

public:
    VkCommandPool m_commandPool;
    DescriptorSetAllocator m_descSetAllocator;
    List<VkFence> m_fences;
    Index m_fenceIndex = -1;
    List<RefPtr<CommandBufferImpl>> m_commandBufferPool;
    uint32_t m_commandBufferAllocId = 0;
    VkFence getCurrentFence() { return m_fences[m_fenceIndex]; }
    void advanceFence();

    Result init(const ITransientResourceHeap::Desc& desc, DeviceImpl* device);
    ~TransientResourceHeapImpl();

public:
    virtual SLANG_NO_THROW Result SLANG_MCALL
        createCommandBuffer(ICommandBuffer** outCommandBuffer) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL synchronizeAndReset() override;
};

class QueryPoolImpl : public QueryPoolBase
{
public:
    Result init(const IQueryPool::Desc& desc, DeviceImpl* device);
    ~QueryPoolImpl();

public:
    virtual SLANG_NO_THROW Result SLANG_MCALL
        getResult(GfxIndex index, GfxCount count, uint64_t* data) override;

public:
    VkQueryPool m_pool;
    RefPtr<DeviceImpl> m_device;
};

class SwapchainImpl
    : public ISwapchain
    , public ComObject
{
public:
    SLANG_COM_OBJECT_IUNKNOWN_ALL
    ISwapchain* getInterface(const Guid& guid);

public:
    struct PlatformDesc
    {};

#if SLANG_WINDOWS_FAMILY
    struct WinPlatformDesc : public PlatformDesc
    {
        HINSTANCE m_hinstance;
        HWND m_hwnd;
    };
#else
    struct XPlatformDesc : public PlatformDesc
    {
        Display* m_display;
        Window m_window;
    };
#endif
public:
    VkSwapchainKHR m_swapChain;
    VkSurfaceKHR m_surface;
    VkSemaphore m_nextImageSemaphore; // Semaphore to signal after `acquireNextImage`.
    ISwapchain::Desc m_desc;
    VkFormat m_vkformat;
    RefPtr<CommandQueueImpl> m_queue;
    ShortList<RefPtr<TextureResourceImpl>> m_images;
    RefPtr<DeviceImpl> m_renderer;
    VulkanApi* m_api;
    uint32_t m_currentImageIndex = 0;
    WindowHandle m_windowHandle;
    void destroySwapchainAndImages();

    void getWindowSize(int* widthOut, int* heightOut) const;

    Result createSwapchainAndImages();

public:
    ~SwapchainImpl();

    static Index _indexOfFormat(List<VkSurfaceFormatKHR>& formatsIn, VkFormat format);

    Result init(DeviceImpl* renderer, const ISwapchain::Desc& desc, WindowHandle window);

    virtual SLANG_NO_THROW const Desc& SLANG_MCALL getDesc() override { return m_desc; }
    virtual SLANG_NO_THROW Result SLANG_MCALL
        getImage(GfxIndex index, ITextureResource** outResource) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL resize(GfxCount width, GfxCount height) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL present() override;
    virtual SLANG_NO_THROW int SLANG_MCALL acquireNextImage() override;
    virtual SLANG_NO_THROW bool SLANG_MCALL isOccluded() override { return false; }
    virtual SLANG_NO_THROW Result SLANG_MCALL setFullScreenMode(bool mode) override;
};

} // namespace vk
} // namespace gfx
