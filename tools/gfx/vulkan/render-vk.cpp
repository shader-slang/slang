// render-vk.cpp
#include "render-vk.h"

//WORKING:#include "options.h"
#include "../render.h"

#include "../../source/core/slang-smart-pointer.h"

#include "vk-api.h"
#include "vk-util.h"
#include "vk-device-queue.h"
#include "vk-swap-chain.h"

#include "../surface.h"

// Vulkan has a different coordinate system to ogl
// http://anki3d.org/vulkan-coordinate-system/

#define ENABLE_VALIDATION_LAYER 1

#ifdef _MSC_VER
#   include <stddef.h>
#   pragma warning(disable: 4996)
#   if (_MSC_VER < 1900)
#       define snprintf sprintf_s
#   endif
#endif

namespace gfx {
using namespace Slang;

class VKRenderer : public Renderer
{
public:
    enum
    {
        kMaxRenderTargets = 8,
        kMaxAttachments = kMaxRenderTargets + 1,

        kMaxDescriptorSets = 4,
    };

    // Renderer    implementation
    virtual SlangResult initialize(const Desc& desc, void* inWindowHandle) override;
    virtual const List<String>& getFeatures() override { return m_features; }
    virtual void setClearColor(const float color[4]) override;
    virtual void clearFrame() override;
    virtual void presentFrame() override;
    TextureResource::Desc getSwapChainTextureDesc() override;

    Result createTextureResource(Resource::Usage initialUsage, const TextureResource::Desc& desc, const TextureResource::Data* initData, TextureResource** outResource) override;
    Result createBufferResource(Resource::Usage initialUsage, const BufferResource::Desc& desc, const void* initData, BufferResource** outResource) override;
    Result createSamplerState(SamplerState::Desc const& desc, SamplerState** outSampler) override;

    Result createTextureView(TextureResource* texture, ResourceView::Desc const& desc, ResourceView** outView) override;
    Result createBufferView(BufferResource* buffer, ResourceView::Desc const& desc, ResourceView** outView) override;

    Result createInputLayout(const InputElementDesc* inputElements, UInt inputElementCount, InputLayout** outLayout) override;

    Result createDescriptorSetLayout(const DescriptorSetLayout::Desc& desc, DescriptorSetLayout** outLayout) override;
    Result createPipelineLayout(const PipelineLayout::Desc& desc, PipelineLayout** outLayout) override;
    Result createDescriptorSet(DescriptorSetLayout* layout, DescriptorSet** outDescriptorSet) override;

    Result createProgram(const ShaderProgram::Desc& desc, ShaderProgram** outProgram) override;
    Result createGraphicsPipelineState(const GraphicsPipelineStateDesc& desc, PipelineState** outState) override;
    Result createComputePipelineState(const ComputePipelineStateDesc& desc, PipelineState** outState) override;

    virtual SlangResult captureScreenSurface(Surface& surface) override;

    virtual void* map(BufferResource* buffer, MapFlavor flavor) override;
    virtual void unmap(BufferResource* buffer) override;
    virtual void setPrimitiveTopology(PrimitiveTopology topology) override;

    virtual void setDescriptorSet(PipelineType pipelineType, PipelineLayout* layout, UInt index, DescriptorSet* descriptorSet) override;

    virtual void setVertexBuffers(UInt startSlot, UInt slotCount, BufferResource*const* buffers, const UInt* strides, const UInt* offsets) override;
    virtual void setIndexBuffer(BufferResource* buffer, Format indexFormat, UInt offset) override;
    virtual void setDepthStencilTarget(ResourceView* depthStencilView) override;
    void setViewports(UInt count, Viewport const* viewports) override;
    void setScissorRects(UInt count, ScissorRect const* rects) override;
    virtual void setPipelineState(PipelineType pipelineType, PipelineState* state) override;
    virtual void draw(UInt vertexCount, UInt startVertex) override;
    virtual void drawIndexed(UInt indexCount, UInt startIndex, UInt baseVertex) override;
    virtual void dispatchCompute(int x, int y, int z) override;
    virtual void submitGpuWork() override;
    virtual void waitForGpu() override;
    virtual RendererType getRendererType() const override { return RendererType::Vulkan; }

        /// Dtor
    ~VKRenderer();

    protected:

    class Buffer
    {
        public:
            /// Initialize a buffer with specified size, and memory props
        Result init(const VulkanApi& api, size_t bufferSize, VkBufferUsageFlags usage, VkMemoryPropertyFlags reqMemoryProperties);

            /// Returns true if has been initialized
        bool isInitialized() const { return m_api != nullptr; }

            // Default Ctor
        Buffer():
            m_api(nullptr)
        {}

            /// Dtor
        ~Buffer()
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

    class InputLayoutImpl : public InputLayout
    {
    public:
        List<VkVertexInputAttributeDescription> m_vertexDescs;
        int m_vertexSize;
    };

    class BufferResourceImpl: public BufferResource
    {
		public:
        typedef BufferResource Parent;

        BufferResourceImpl(Resource::Usage initialUsage, const BufferResource::Desc& desc, VKRenderer* renderer):
            Parent(desc),
			m_renderer(renderer),
            m_initialUsage(initialUsage)
		{
			assert(renderer);
		}

        Resource::Usage m_initialUsage;
		VKRenderer* m_renderer;
        Buffer m_buffer;
        Buffer m_uploadBuffer;
        List<uint8_t> m_readBuffer;                         ///< Stores the contents when a map read is performed

        MapFlavor m_mapFlavor = MapFlavor::Unknown;         ///< If resource is mapped, records what kind of mapping else Unknown (if not mapped)
    };

    class TextureResourceImpl : public TextureResource
    {
    public:
        typedef TextureResource Parent;

        TextureResourceImpl(const Desc& desc, Usage initialUsage, const VulkanApi* api) :
            Parent(desc),
            m_initialUsage(initialUsage),
            m_api(api)
        {
        }
        ~TextureResourceImpl()
        {
            if (m_api)
            {
                if (m_imageMemory != VK_NULL_HANDLE)
                {
                    m_api->vkFreeMemory(m_api->m_device, m_imageMemory, nullptr);
                }
                if (m_image != VK_NULL_HANDLE)
                {
                    m_api->vkDestroyImage(m_api->m_device, m_image, nullptr);
                }
            }
        }

        Usage m_initialUsage;

        VkImage m_image = VK_NULL_HANDLE;
        VkDeviceMemory m_imageMemory = VK_NULL_HANDLE;

        const VulkanApi* m_api;
    };

    class SamplerStateImpl : public SamplerState
    {
    public:
        VkSampler m_sampler;
    };

    class ResourceViewImpl : public ResourceView
    {
    public:
        enum class ViewType
        {
            Texture,
            TexelBuffer,
            PlainBuffer,
        };
        ViewType            m_type;
    };

    class TextureResourceViewImpl : public ResourceViewImpl
    {
    public:
        TextureResourceViewImpl()
        {
            m_type = ViewType::Texture;
        }

        RefPtr<TextureResourceImpl> m_texture;
        VkImageView                 m_view;
        VkImageLayout               m_layout;
    };

    class TexelBufferResourceViewImpl : public ResourceViewImpl
    {
    public:
        TexelBufferResourceViewImpl()
        {
            m_type = ViewType::TexelBuffer;
        }

        RefPtr<BufferResourceImpl>  m_buffer;
        VkBufferView m_view;
    };

    class PlainBufferResourceViewImpl : public ResourceViewImpl
    {
    public:
        PlainBufferResourceViewImpl()
        {
            m_type = ViewType::PlainBuffer;
        }

        RefPtr<BufferResourceImpl>  m_buffer;
        VkDeviceSize                offset;
        VkDeviceSize                size;
    };

    class ShaderProgramImpl: public ShaderProgram
    {
		public:

        ShaderProgramImpl(PipelineType pipelineType):
            m_pipelineType(pipelineType)
        {}

        PipelineType m_pipelineType;

        VkPipelineShaderStageCreateInfo m_compute;
        VkPipelineShaderStageCreateInfo m_vertex;
        VkPipelineShaderStageCreateInfo m_fragment;

		List<char> m_buffers[2];								//< To keep storage of code in scope
    };

    class DescriptorSetLayoutImpl : public DescriptorSetLayout
    {
    public:
        DescriptorSetLayoutImpl(const VulkanApi& api)
            : m_api(&api)
        {
        }

        ~DescriptorSetLayoutImpl()
        {
            if(m_descriptorSetLayout != VK_NULL_HANDLE)
            {
                m_api->vkDestroyDescriptorSetLayout(m_api->m_device, m_descriptorSetLayout, nullptr);
            }
            if (m_descriptorPool != VK_NULL_HANDLE)
            {
                m_api->vkDestroyDescriptorPool(m_api->m_device, m_descriptorPool, nullptr);
            }
        }

        VulkanApi const* m_api;
        VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
        VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;

        struct RangeInfo
        {
            VkDescriptorType descriptorType;
        };
        List<RangeInfo> m_ranges;
    };

    class PipelineLayoutImpl : public PipelineLayout
    {
    public:
        PipelineLayoutImpl(const VulkanApi& api)
            : m_api(&api)
        {
        }

        ~PipelineLayoutImpl()
        {
            if (m_pipelineLayout != VK_NULL_HANDLE)
            {
                m_api->vkDestroyPipelineLayout(m_api->m_device, m_pipelineLayout, nullptr);
            }
        }

        VulkanApi const*    m_api;
        VkPipelineLayout    m_pipelineLayout        = VK_NULL_HANDLE;
        UInt                m_descriptorSetCount    = 0;
    };

    class DescriptorSetImpl : public DescriptorSet
    {
    public:
        // Record the view binding
        struct Binding
        {
            enum class Type : uint8_t
            {
                Unknown,
                ResourceView,
                SamplerState,
                BufferResource,
                CountOf,
            };
            Type type;
            uint32_t range;
            uint32_t index;
            RefPtr<RefObject> obj;
        };

        DescriptorSetImpl(VKRenderer* renderer)
            : m_renderer(renderer)
        {
        }

        ~DescriptorSetImpl()
        {
        }

        virtual void setConstantBuffer(UInt range, UInt index, BufferResource* buffer) override;
        virtual void setResource(UInt range, UInt index, ResourceView* view) override;
        virtual void setSampler(UInt range, UInt index, SamplerState* sampler) override;
        virtual void setCombinedTextureSampler(
            UInt range,
            UInt index,
            ResourceView*   textureView,
            SamplerState*   sampler) override;

        static Binding::Type _getBindingType(RefObject* ptr);
        void _setBinding(Binding::Type type, UInt range, UInt index, RefObject* ptr);

        VKRenderer*                         m_renderer = nullptr;   ///< Weak pointer, can't be strong, because if set will become circular reference
        RefPtr<DescriptorSetLayoutImpl>     m_layout;
        VkDescriptorSet                     m_descriptorSet = VK_NULL_HANDLE;

        List<Binding>                       m_bindings;             ///< Records entities are bound to this descriptor set, and keeps the associated resources/views/state in scope
    };

    struct BoundVertexBuffer
    {
        RefPtr<BufferResourceImpl> m_buffer;
        int m_stride;
        int m_offset;
    };

    class PipelineStateImpl : public PipelineState
    {
    public:
        PipelineStateImpl(const VulkanApi& api):
            m_api(&api)
        {
        }
        ~PipelineStateImpl()
        {
            if (m_pipeline != VK_NULL_HANDLE)
            {
                m_api->vkDestroyPipeline(m_api->m_device, m_pipeline, nullptr);
            }
        }

        const VulkanApi* m_api;

        RefPtr<PipelineLayoutImpl>  m_pipelineLayout;

        RefPtr<ShaderProgramImpl> m_shaderProgram;

        VkPipeline m_pipeline = VK_NULL_HANDLE;
    };

    VkBool32 handleDebugMessage(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t srcObject,
        size_t location, int32_t msgCode, const char* pLayerPrefix, const char* pMsg);

    VkPipelineShaderStageCreateInfo compileEntryPoint(
        ShaderProgram::KernelDesc const&    kernelDesc,
        VkShaderStageFlagBits stage,
        List<char>& bufferOut);

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugMessageCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t srcObject,
        size_t location, int32_t msgCode, const char* pLayerPrefix, const char* pMsg, void* pUserData);

        /// Returns true if m_currentPipeline matches the current configuration
//    Pipeline* _getPipeline();
//    bool _isEqual(const Pipeline& pipeline) const;
//    Slang::Result _createPipeline(RefPtr<Pipeline>& pipelineOut);
    void _beginRender();
    void _endRender();

    Slang::Result _beginPass();
    void _endPass();
    void _transitionImageLayout(VkImage image, VkFormat format, const TextureResource::Desc& desc, VkImageLayout oldLayout, VkImageLayout newLayout);

    VkDebugReportCallbackEXT m_debugReportCallback;

//    RefPtr<InputLayoutImpl> m_currentInputLayout;

//    RefPtr<BindingStateImpl> m_currentBindingState;
    RefPtr<PipelineLayoutImpl>  m_currentPipelineLayout;

    RefPtr<DescriptorSetImpl>   m_currentDescriptorSetImpls [kMaxDescriptorSets];
    VkDescriptorSet             m_currentDescriptorSets     [kMaxDescriptorSets];

//    RefPtr<ShaderProgramImpl> m_currentProgram;

//    List<RefPtr<Pipeline> > m_pipelineCache;
    RefPtr<PipelineStateImpl>   m_currentPipeline;

    List<BoundVertexBuffer> m_boundVertexBuffers;

    VkPrimitiveTopology m_primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkDevice m_device = VK_NULL_HANDLE;

    VulkanModule m_module;
    VulkanApi m_api;

    VulkanDeviceQueue m_deviceQueue;
    VulkanSwapChain m_swapChain;

    VkRenderPass m_renderPass = VK_NULL_HANDLE;

    int m_swapChainImageIndex = -1;

    float m_clearColor[4] = { 0, 0, 0, 0 };

    Desc m_desc;
    List<String> m_features;
};

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! VkRenderer::Buffer !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

Result VKRenderer::Buffer::init(const VulkanApi& api, size_t bufferSize, VkBufferUsageFlags usage, VkMemoryPropertyFlags reqMemoryProperties)
{
    assert(!isInitialized());

    m_api = &api;
    m_memory = VK_NULL_HANDLE;
    m_buffer = VK_NULL_HANDLE;

    VkBufferCreateInfo bufferCreateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bufferCreateInfo.size = bufferSize;
    bufferCreateInfo.usage = usage;

    SLANG_VK_CHECK(api.vkCreateBuffer(api.m_device, &bufferCreateInfo, nullptr, &m_buffer));

    VkMemoryRequirements memoryReqs = {};
    api.vkGetBufferMemoryRequirements(api.m_device, m_buffer, &memoryReqs);

    int memoryTypeIndex = api.findMemoryTypeIndex(memoryReqs.memoryTypeBits, reqMemoryProperties);
    assert(memoryTypeIndex >= 0);

    VkMemoryPropertyFlags actualMemoryProperites = api.m_deviceMemoryProperties.memoryTypes[memoryTypeIndex].propertyFlags;

    VkMemoryAllocateInfo allocateInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
    allocateInfo.allocationSize = memoryReqs.size;
    allocateInfo.memoryTypeIndex = memoryTypeIndex;

    SLANG_VK_CHECK(api.vkAllocateMemory(api.m_device, &allocateInfo, nullptr, &m_memory));
    SLANG_VK_CHECK(api.vkBindBufferMemory(api.m_device, m_buffer, m_memory, 0));

    return SLANG_OK;
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! VkRenderer !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

#if 0
bool VKRenderer::_isEqual(const Pipeline& pipeline) const
{
    return
        pipeline.m_pipelineLayout == m_currentPipelineLayout &&
        pipeline.m_primitiveTopology == m_primitiveTopology &&
        pipeline.m_inputLayout == m_currentInputLayout &&
        pipeline.m_shaderProgram == m_currentProgram;
}

VKRenderer::Pipeline* VKRenderer::_getPipeline()
{
    if (m_currentPipeline && _isEqual(*m_currentPipeline))
    {
        return m_currentPipeline;
    }

    // Look for a match in the cache
    for (int i = 0; i < int(m_pipelineCache.Count()); ++i)
    {
        Pipeline* pipeline = m_pipelineCache[i];
        if (_isEqual(*pipeline))
        {
            m_currentPipeline = pipeline;
            return pipeline;
        }
    }

    RefPtr<Pipeline> pipeline;
    SLANG_RETURN_NULL_ON_FAIL(_createPipeline(pipeline));
    m_pipelineCache.Add(pipeline);
    m_currentPipeline = pipeline;
    return pipeline;
}

Slang::Result VKRenderer::_createPipeline(RefPtr<Pipeline>& pipelineOut)
{
    RefPtr<Pipeline> pipeline(new Pipeline(m_api));

    // Initialize the state
    pipeline->m_primitiveTopology = m_primitiveTopology;
    pipeline->m_pipelineLayout = m_currentPipelineLayout;
    pipeline->m_shaderProgram = m_currentProgram;
    pipeline->m_inputLayout = m_currentInputLayout;

    // Must be equal at this point if all the items are correctly set in pipeline
    assert(_isEqual(*pipeline));

    VkPipelineCache pipelineCache = VK_NULL_HANDLE;

    if (m_currentProgram->m_pipelineType == PipelineType::Compute)
    {
        // Then create a pipeline to use that layout

        VkComputePipelineCreateInfo computePipelineInfo = { VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO };
        computePipelineInfo.stage = m_currentProgram->m_compute;
        computePipelineInfo.layout = pipeline->m_pipelineLayout->m_pipelineLayout;

        SLANG_VK_CHECK(m_api.vkCreateComputePipelines(m_device, pipelineCache, 1, &computePipelineInfo, nullptr, &pipeline->m_pipeline));
    }
    else if (m_currentProgram->m_pipelineType == PipelineType::Graphics)
    {
        // Create the graphics pipeline

        const int width = m_swapChain.getWidth();
        const int height = m_swapChain.getHeight();

        VkPipelineShaderStageCreateInfo shaderStages[] = {  m_currentProgram->m_vertex, m_currentProgram->m_fragment };

        // VertexBuffer/s
        // Currently only handles one

        VkPipelineVertexInputStateCreateInfo vertexInputInfo = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 0;
        vertexInputInfo.vertexAttributeDescriptionCount = 0;

        VkVertexInputBindingDescription vertexInputBindingDescription;

        if (m_currentInputLayout)
        {
            vertexInputBindingDescription.binding = 0;
            vertexInputBindingDescription.stride = m_currentInputLayout->m_vertexSize;
            vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            const auto& srcAttributeDescs = m_currentInputLayout->m_vertexDescs;

            vertexInputInfo.vertexBindingDescriptionCount = 1;
            vertexInputInfo.pVertexBindingDescriptions = &vertexInputBindingDescription;

            vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(srcAttributeDescs.Count());
            vertexInputInfo.pVertexAttributeDescriptions = srcAttributeDescs.getBuffer();
        }

        //

        VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkViewport viewport = {};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)width;
        viewport.height = (float)height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor = {};
        scissor.offset = { 0, 0 };
        scissor.extent = { uint32_t(width), uint32_t(height) };

        VkPipelineViewportStateCreateInfo viewportState = {};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;

        VkPipelineRasterizationStateCreateInfo rasterizer = {};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_NONE;
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;

        VkPipelineMultisampleStateCreateInfo multisampling = {};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo colorBlending = {};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

        VkGraphicsPipelineCreateInfo pipelineInfo = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };

        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.layout = pipeline->m_pipelineLayout->m_pipelineLayout;
        pipelineInfo.renderPass = m_renderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        SLANG_VK_CHECK(m_api.vkCreateGraphicsPipelines(m_device, pipelineCache, 1, &pipelineInfo, nullptr, &pipeline->m_pipeline));
    }
    else
    {
        assert(!"Unhandled program type");
        return SLANG_FAIL;
    }

    pipelineOut = pipeline;
    return SLANG_OK;
}
#endif

Result VKRenderer::_beginPass()
{
    if (m_swapChainImageIndex < 0)
    {
        return SLANG_FAIL;
    }

    const int numRenderTargets = 1;

    const VulkanSwapChain::Image& image = m_swapChain.getImages()[m_swapChainImageIndex];

    int numAttachments = 0;

    // Start render pass
    VkClearValue clearValues[kMaxAttachments];
    clearValues[numAttachments++] = VkClearValue{ m_clearColor[0], m_clearColor[1], m_clearColor[2], m_clearColor[3] };

    bool hasDepthBuffer = false;
    if (hasDepthBuffer)
    {
        VkClearValue& clearValue = clearValues[numAttachments++];

        clearValue.depthStencil.depth = 1.0f;
        clearValue.depthStencil.stencil = 0;
    }

    const int width = m_swapChain.getWidth();
    const int height = m_swapChain.getHeight();

    VkCommandBuffer cmdBuffer = m_deviceQueue.getCommandBuffer();

    VkRenderPassBeginInfo renderPassBegin = {};
    renderPassBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBegin.renderPass = m_renderPass;
    renderPassBegin.framebuffer = image.m_frameBuffer;
    renderPassBegin.renderArea.offset.x = 0;
    renderPassBegin.renderArea.offset.y = 0;
    renderPassBegin.renderArea.extent.width = width;
    renderPassBegin.renderArea.extent.height = height;
    renderPassBegin.clearValueCount = numAttachments;
    renderPassBegin.pClearValues = clearValues;

    m_api.vkCmdBeginRenderPass(cmdBuffer, &renderPassBegin, VK_SUBPASS_CONTENTS_INLINE);

    // Set up scissor and viewport
    {
        VkRect2D rects[kMaxRenderTargets] = {};
        VkViewport viewports[kMaxRenderTargets] = {};
        for (int i = 0; i < numRenderTargets; ++i)
        {
            rects[i] = VkRect2D{ 0, 0, uint32_t(width), uint32_t(height) };

            VkViewport& dstViewport = viewports[i];

            dstViewport.x = 0.0f;
            dstViewport.y = 0.0f;
            dstViewport.width = float(width);
            dstViewport.height = float(height);
            dstViewport.minDepth = 0.0f;
            dstViewport.maxDepth = 1.0f;
        }

        m_api.vkCmdSetScissor(cmdBuffer, 0, numRenderTargets, rects);
        m_api.vkCmdSetViewport(cmdBuffer, 0, numRenderTargets, viewports);
    }

    return SLANG_OK;
}

void VKRenderer::_endPass()
{
    VkCommandBuffer cmdBuffer = m_deviceQueue.getCommandBuffer();
    m_api.vkCmdEndRenderPass(cmdBuffer);
}

void VKRenderer::_beginRender()
{
    m_swapChainImageIndex = m_swapChain.nextFrontImageIndex();

    if (m_swapChainImageIndex < 0)
    {
        return;
    }
}

void VKRenderer::_endRender()
{
    m_deviceQueue.flush();
}

Renderer* createVKRenderer()
{
    return new VKRenderer;
}

VKRenderer::~VKRenderer()
{
    // Check the device queue is valid else, we can't wait on it..
    if (m_deviceQueue.isValid())
    {
        waitForGpu();
    }

    m_currentPipeline.setNull();

    // Same as clear but, also dtors all elements, which clear does not
    m_boundVertexBuffers = List<BoundVertexBuffer>();

    m_currentPipelineLayout.setNull();
    for (auto& impl : m_currentDescriptorSetImpls)
    {
        impl.setNull();
    }

    if (m_renderPass != VK_NULL_HANDLE)
    {
        m_api.vkDestroyRenderPass(m_device, m_renderPass, nullptr);
        m_renderPass = VK_NULL_HANDLE;
    }

    m_swapChain.destroy();

    m_deviceQueue.destroy();

    if (m_device != VK_NULL_HANDLE)
    {
        m_api.vkDestroyDevice(m_device, nullptr);
        m_device = VK_NULL_HANDLE;
    }
}


VkBool32 VKRenderer::handleDebugMessage(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t srcObject,
    size_t location, int32_t msgCode, const char* pLayerPrefix, const char* pMsg)
{
    char const* severity = "message";
    if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
        severity = "warning";
    if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
        severity = "error";

    // pMsg can be really big (it can be assembler dump for example)
    // Use a dynamic buffer to store
    size_t bufferSize = strlen(pMsg) + 1 + 1024;
    List<char> bufferArray;
    bufferArray.setCount(bufferSize);
    char* buffer = bufferArray.getBuffer();

    sprintf_s(buffer,
        bufferSize,
        "%s: %s %d: %s\n",
        pLayerPrefix,
        severity,
        msgCode,
        pMsg);

    fprintf(stderr, "%s", buffer);
    fflush(stderr);

    OutputDebugStringA(buffer);

    return VK_FALSE;
}

/* static */VKAPI_ATTR VkBool32 VKAPI_CALL VKRenderer::debugMessageCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t srcObject,
    size_t location, int32_t msgCode, const char* pLayerPrefix, const char* pMsg, void* pUserData)
{
    return ((VKRenderer*)pUserData)->handleDebugMessage(flags, objType, srcObject, location, msgCode, pLayerPrefix, pMsg);
}

VkPipelineShaderStageCreateInfo VKRenderer::compileEntryPoint(
    ShaderProgram::KernelDesc const&    kernelDesc,
    VkShaderStageFlagBits stage,
    List<char>& bufferOut)
{
    char const* dataBegin = (char const*) kernelDesc.codeBegin;
    char const* dataEnd = (char const*) kernelDesc.codeEnd;

    // We need to make a copy of the code, since the Slang compiler
    // will free the memory after a compile request is closed.
    size_t codeSize = dataEnd - dataBegin;

	bufferOut.insertRange(0, dataBegin, codeSize);

    char* codeBegin = bufferOut.getBuffer();

    VkShaderModuleCreateInfo moduleCreateInfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
    moduleCreateInfo.pCode = (uint32_t*)codeBegin;
    moduleCreateInfo.codeSize = codeSize;

    VkShaderModule module;
    SLANG_VK_CHECK(m_api.vkCreateShaderModule(m_device, &moduleCreateInfo, nullptr, &module));

    VkPipelineShaderStageCreateInfo shaderStageCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
    shaderStageCreateInfo.stage = stage;

    shaderStageCreateInfo.module = module;
    shaderStageCreateInfo.pName = "main";

    return shaderStageCreateInfo;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!! Renderer interface !!!!!!!!!!!!!!!!!!!!!!!!!!

SlangResult VKRenderer::initialize(const Desc& desc, void* inWindowHandle)
{
    SLANG_RETURN_ON_FAIL(m_module.init());
    SLANG_RETURN_ON_FAIL(m_api.initGlobalProcs(m_module));

    m_desc = desc;

    VkApplicationInfo applicationInfo = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
    applicationInfo.pApplicationName = "slang-render-test";
    applicationInfo.pEngineName = "slang-render-test";
    applicationInfo.apiVersion = VK_API_VERSION_1_0;

    char const* instanceExtensions[] =
    {
        VK_KHR_SURFACE_EXTENSION_NAME,

        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,

#if SLANG_WINDOWS_FAMILY
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#else
        VK_KHR_XLIB_SURFACE_EXTENSION_NAME
#endif

#if ENABLE_VALIDATION_LAYER
        VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
#endif
    };

    VkInstance instance = VK_NULL_HANDLE;

    VkInstanceCreateInfo instanceCreateInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
    instanceCreateInfo.pApplicationInfo = &applicationInfo;

    instanceCreateInfo.enabledExtensionCount = SLANG_COUNT_OF(instanceExtensions);
    instanceCreateInfo.ppEnabledExtensionNames = &instanceExtensions[0];

#if ENABLE_VALIDATION_LAYER
    const char* layerNames[] = { "VK_LAYER_LUNARG_standard_validation" };
    instanceCreateInfo.enabledLayerCount = SLANG_COUNT_OF(layerNames);
    instanceCreateInfo.ppEnabledLayerNames = layerNames;
#endif

    SLANG_VK_RETURN_ON_FAIL(m_api.vkCreateInstance(&instanceCreateInfo, nullptr, &instance));
    SLANG_RETURN_ON_FAIL(m_api.initInstanceProcs(instance));

#if ENABLE_VALIDATION_LAYER
    VkDebugReportFlagsEXT debugFlags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;

    VkDebugReportCallbackCreateInfoEXT debugCreateInfo = { VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT };
    debugCreateInfo.pfnCallback = &debugMessageCallback;
    debugCreateInfo.pUserData = this;
    debugCreateInfo.flags = debugFlags;

    SLANG_VK_RETURN_ON_FAIL(m_api.vkCreateDebugReportCallbackEXT(instance, &debugCreateInfo, nullptr, &m_debugReportCallback));
#endif

    uint32_t numPhysicalDevices = 0;
    SLANG_VK_RETURN_ON_FAIL(m_api.vkEnumeratePhysicalDevices(instance, &numPhysicalDevices, nullptr));

    List<VkPhysicalDevice> physicalDevices;
    physicalDevices.setCount(numPhysicalDevices);
    SLANG_VK_RETURN_ON_FAIL(m_api.vkEnumeratePhysicalDevices(instance, &numPhysicalDevices, physicalDevices.getBuffer()));

    Index selectedDeviceIndex = 0;

    if (desc.adapter.getLength())
    {
        selectedDeviceIndex = -1;

        String lowerAdapter = desc.adapter.toLower();

        for (Index i = 0; i < physicalDevices.getCount(); ++i)
        {
            auto physicalDevice = physicalDevices[i];

            VkPhysicalDeviceProperties basicProps = {};
            m_api.vkGetPhysicalDeviceProperties(physicalDevice, &basicProps);

            String lowerName = String(basicProps.deviceName).toLower();

            if (lowerName.indexOf(lowerAdapter) != Index(-1))
            {
                selectedDeviceIndex = i;
                break;
            }
        }
        if (selectedDeviceIndex < 0)
        {
            // Device not found
            return SLANG_FAIL;
        }
    }

    SLANG_RETURN_ON_FAIL(m_api.initPhysicalDevice(physicalDevices[selectedDeviceIndex]));

    List<const char*> deviceExtensions;
    deviceExtensions.add(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    VkDeviceCreateInfo deviceCreateInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
    deviceCreateInfo.queueCreateInfoCount = 1;
    
    deviceCreateInfo.pEnabledFeatures = &m_api.m_deviceFeatures;

    // Get the device features (doesn't use, but useful when debugging)
    if (m_api.vkGetPhysicalDeviceFeatures2)
    {
        VkPhysicalDeviceFeatures2 deviceFeatures2 = {};
        deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        m_api.vkGetPhysicalDeviceFeatures2(m_api.m_physicalDevice, &deviceFeatures2);
    }

    VkPhysicalDeviceProperties basicProps = {};
    m_api.vkGetPhysicalDeviceProperties(m_api.m_physicalDevice, &basicProps);

    // Get the API version
    const uint32_t majorVersion = VK_VERSION_MAJOR(basicProps.apiVersion);
    const uint32_t minorVersion = VK_VERSION_MINOR(basicProps.apiVersion);

    // Float16 features
    // Need in this scope because it will be linked into the device creation (if it is available)
    VkPhysicalDeviceFloat16Int8FeaturesKHR float16Features = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FLOAT16_INT8_FEATURES_KHR };

    // API version check, can't use vkGetPhysicalDeviceProperties2 yet since this device might not support it
    if (VK_MAKE_VERSION(majorVersion, minorVersion, 0) >= VK_API_VERSION_1_1 &&
        m_api.vkGetPhysicalDeviceProperties2 &&
        m_api.vkGetPhysicalDeviceFeatures2)
    {
        VkPhysicalDeviceProperties2 physicalDeviceProps2;

        physicalDeviceProps2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        physicalDeviceProps2.pNext = nullptr;
        physicalDeviceProps2.properties = {};

        m_api.vkGetPhysicalDeviceProperties2(m_api.m_physicalDevice, &physicalDeviceProps2);

        // Get device features
        VkPhysicalDeviceFeatures2 deviceFeatures2 = {};
        deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;

        // Link together for lookup 
        float16Features.pNext = deviceFeatures2.pNext;
        deviceFeatures2.pNext = &float16Features;

        m_api.vkGetPhysicalDeviceFeatures2(m_api.m_physicalDevice, &deviceFeatures2);

        // If we have float16 features then enable
        if (float16Features.shaderFloat16)
        {
            // Link into the creation features
            float16Features.pNext = (void*)deviceCreateInfo.pNext;
            deviceCreateInfo.pNext = &float16Features;

            // Add the Float16 extension
            deviceExtensions.add(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME);

            // We have half support
            m_features.add("half");
        }   
    }

    int queueFamilyIndex = m_api.findQueue(VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT);
    assert(queueFamilyIndex >= 0);

    float queuePriority = 0.0f;
    VkDeviceQueueCreateInfo queueCreateInfo = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
    queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;

    deviceCreateInfo.enabledExtensionCount = uint32_t(deviceExtensions.getCount());
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.getBuffer();

    SLANG_VK_RETURN_ON_FAIL(m_api.vkCreateDevice(m_api.m_physicalDevice, &deviceCreateInfo, nullptr, &m_device));
    SLANG_RETURN_ON_FAIL(m_api.initDeviceProcs(m_device));

    {
        VkQueue queue;
        m_api.vkGetDeviceQueue(m_device, queueFamilyIndex, 0, &queue);
        SLANG_RETURN_ON_FAIL(m_deviceQueue.init(m_api, queue, queueFamilyIndex));
    }

    // set up swap chain

    {
        VulkanSwapChain::Desc desc;
        VulkanSwapChain::PlatformDesc* platformDesc = nullptr;

        desc.init();
        desc.m_format = Format::RGBA_Unorm_UInt8;

#if SLANG_WINDOWS_FAMILY
        VulkanSwapChain::WinPlatformDesc winPlatformDesc;
        winPlatformDesc.m_hinstance = ::GetModuleHandle(nullptr);
        winPlatformDesc.m_hwnd = (HWND)inWindowHandle;
        platformDesc = &winPlatformDesc;
#endif

        SLANG_RETURN_ON_FAIL(m_swapChain.init(&m_deviceQueue, desc, platformDesc));
    }

    // depth/stencil?

    // render pass?

    {
        const int numRenderTargets = 1;
        bool shouldClear = true;
        bool shouldClearDepth = false;
        bool shouldClearStencil = false;
        bool hasDepthBuffer = false;

        Format depthFormat = Format::Unknown;
        VkFormat colorFormat = m_swapChain.getVkFormat();

        int numAttachments = 0;
        // We need extra space if we have depth buffer
        VkAttachmentDescription attachmentDesc[kMaxRenderTargets + 1] = {};
        for (int i = 0; i < numRenderTargets; ++i)
        {
            VkAttachmentDescription& dst = attachmentDesc[numAttachments ++];

            dst.flags = 0;
            dst.format = colorFormat;
            dst.samples = VK_SAMPLE_COUNT_1_BIT;
            dst.loadOp = shouldClear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
            dst.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            dst.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            dst.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            dst.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            dst.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        }
        if (hasDepthBuffer)
        {
            VkAttachmentDescription& dst = attachmentDesc[numAttachments++];

            dst.flags = 0;
            dst.format = VulkanUtil::getVkFormat(depthFormat);
            dst.samples = VK_SAMPLE_COUNT_1_BIT;
            dst.loadOp = shouldClearDepth ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
            dst.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            dst.stencilLoadOp = shouldClearStencil ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
            dst.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
            dst.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            dst.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }

        VkAttachmentReference colorAttachments[kMaxRenderTargets] = {};
        for (int i = 0; i < numRenderTargets; ++i)
        {
            VkAttachmentReference& dst = colorAttachments[i];
            dst.attachment = i;
            dst.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        }

        VkAttachmentReference depthAttachment = {};
        depthAttachment.attachment = numRenderTargets;
        depthAttachment.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpassDesc = {};
        subpassDesc.flags = 0;
        subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDesc.inputAttachmentCount = 0u;
        subpassDesc.pInputAttachments = nullptr;
        subpassDesc.colorAttachmentCount = numRenderTargets;
        subpassDesc.pColorAttachments = colorAttachments;
        subpassDesc.pResolveAttachments = nullptr;
        subpassDesc.pDepthStencilAttachment = hasDepthBuffer ? &depthAttachment : nullptr;
        subpassDesc.preserveAttachmentCount = 0u;
        subpassDesc.pPreserveAttachments = nullptr;

        VkRenderPassCreateInfo renderPassCreateInfo = {};
        renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassCreateInfo.attachmentCount = numAttachments;
        renderPassCreateInfo.pAttachments = attachmentDesc;
        renderPassCreateInfo.subpassCount = 1;
        renderPassCreateInfo.pSubpasses = &subpassDesc;
        SLANG_VK_RETURN_ON_FAIL(m_api.vkCreateRenderPass(m_device, &renderPassCreateInfo, nullptr, &m_renderPass));
    }

    // frame buffer
    SLANG_RETURN_ON_FAIL(m_swapChain.createFrameBuffers(m_renderPass));

    _beginRender();

    return SLANG_OK;
}

void VKRenderer::submitGpuWork()
{
    m_deviceQueue.flush();
}

void VKRenderer::waitForGpu()
{
    m_deviceQueue.flushAndWait();
}

void VKRenderer::setClearColor(const float color[4])
{
    for (int ii = 0; ii < 4; ++ii)
        m_clearColor[ii] = color[ii];
}

void VKRenderer::clearFrame()
{
}

void VKRenderer::presentFrame()
{
    _endRender();

    const bool vsync = true;
    m_swapChain.present(vsync);

    _beginRender();
}

TextureResource::Desc VKRenderer::getSwapChainTextureDesc()
{
    TextureResource::Desc desc;
    desc.init2D(Resource::Type::Texture2D, Format::Unknown, m_desc.width, m_desc.height, 1);
    return desc;
}

SlangResult VKRenderer::captureScreenSurface(Surface& surfaceOut)
{
    return SLANG_FAIL;
}

static VkBufferUsageFlagBits _calcBufferUsageFlags(Resource::BindFlag::Enum bind)
{
    typedef Resource::BindFlag BindFlag;

    switch (bind)
    {
        case BindFlag::VertexBuffer:            return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        case BindFlag::IndexBuffer:             return VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        case BindFlag::ConstantBuffer:          return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        case BindFlag::StreamOutput:
        case BindFlag::RenderTarget:
        case BindFlag::DepthStencil:
        {
            assert(!"Not supported yet");
            return VkBufferUsageFlagBits(0);
        }
        case BindFlag::UnorderedAccess:         return VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
        case BindFlag::PixelShaderResource:     return VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        case BindFlag::NonPixelShaderResource:  return VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        default:                                return VkBufferUsageFlagBits(0);
    }
}

static VkBufferUsageFlagBits _calcBufferUsageFlags(int bindFlags)
{
    int dstFlags = 0;
    while (bindFlags)
    {
        int lsb = bindFlags & -bindFlags;
        dstFlags |= _calcBufferUsageFlags(Resource::BindFlag::Enum(lsb));
        bindFlags &= ~lsb;
    }
    return VkBufferUsageFlagBits(dstFlags);
}

static VkBufferUsageFlags _calcBufferUsageFlags(int bindFlags, int cpuAccessFlags, const void* initData)
{
    VkBufferUsageFlags usage = _calcBufferUsageFlags(bindFlags);

    if (cpuAccessFlags & Resource::AccessFlag::Read)
    {
        // If it can be read from, set this
        usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    }
    if ((cpuAccessFlags & Resource::AccessFlag::Write) || initData)
    {
        usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    }

    return usage;
}

static VkImageUsageFlagBits _calcImageUsageFlags(Resource::BindFlag::Enum bind)
{
    typedef Resource::BindFlag BindFlag;

    switch (bind)
    {
        case BindFlag::RenderTarget:            return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        case BindFlag::DepthStencil:            return VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        case BindFlag::NonPixelShaderResource:
        case BindFlag::PixelShaderResource:
        {
            // Ignore
            return VkImageUsageFlagBits(0);
        }
        default:
        {
            assert(!"Unsupported");
            return VkImageUsageFlagBits(0);
        }
    }
}

static VkImageUsageFlagBits _calcImageUsageFlags(int bindFlags)
{
    int dstFlags = 0;
    while (bindFlags)
    {
        int lsb = bindFlags & -bindFlags;
        dstFlags |= _calcImageUsageFlags(Resource::BindFlag::Enum(lsb));
        bindFlags &= ~lsb;
    }
    return VkImageUsageFlagBits(dstFlags);
}

static VkImageUsageFlags _calcImageUsageFlags(int bindFlags, int cpuAccessFlags, const void* initData)
{
    VkImageUsageFlags usage = _calcImageUsageFlags(bindFlags);

    usage |= VK_IMAGE_USAGE_SAMPLED_BIT;

    if (cpuAccessFlags & Resource::AccessFlag::Read)
    {
        // If it can be read from, set this
        usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }
    if ((cpuAccessFlags & Resource::AccessFlag::Write) || initData)
    {
        usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }

    return usage;
}

void VKRenderer::_transitionImageLayout(VkImage image, VkFormat format, const TextureResource::Desc& desc, VkImageLayout oldLayout, VkImageLayout newLayout)
{
    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = desc.numMipLevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else
    {
        assert(!"unsupported layout transition!");
        return;
    }

    VkCommandBuffer commandBuffer = m_deviceQueue.getCommandBuffer();

    m_api.vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

Result VKRenderer::createTextureResource(Resource::Usage initialUsage, const TextureResource::Desc& descIn, const TextureResource::Data* initData, TextureResource** outResource)
{
    TextureResource::Desc desc(descIn);
    desc.setDefaults(initialUsage);

    const VkFormat format = VulkanUtil::getVkFormat(desc.format);
    if (format == VK_FORMAT_UNDEFINED)
    {
        assert(!"Unhandled image format");
        return SLANG_FAIL;
    }

    const int arraySize = desc.calcEffectiveArraySize();

    RefPtr<TextureResourceImpl> texture(new TextureResourceImpl(desc, initialUsage, &m_api));

    // Create the image
    {
        VkImageCreateInfo imageInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};

        switch (desc.type)
        {
            case Resource::Type::Texture1D:
            {
                imageInfo.imageType = VK_IMAGE_TYPE_1D;
                imageInfo.extent = VkExtent3D{ uint32_t(descIn.size.width), 1, 1 };
                break;
            }
            case Resource::Type::Texture2D:
            {
                imageInfo.imageType = VK_IMAGE_TYPE_2D;
                imageInfo.extent = VkExtent3D{ uint32_t(descIn.size.width), uint32_t(descIn.size.height), 1 };
                break;
            }
            case Resource::Type::TextureCube:
            {
                imageInfo.imageType = VK_IMAGE_TYPE_2D;
                imageInfo.extent = VkExtent3D{ uint32_t(descIn.size.width), uint32_t(descIn.size.height), 1 };
                break;
            }
            case Resource::Type::Texture3D:
            {
                // Can't have an array and 3d texture
                assert(desc.arraySize <= 1);

                imageInfo.imageType = VK_IMAGE_TYPE_3D;
                imageInfo.extent = VkExtent3D{ uint32_t(descIn.size.width), uint32_t(descIn.size.height), uint32_t(descIn.size.depth) };
                break;
            }
            default:
            {
                assert(!"Unhandled type");
                return SLANG_FAIL;
            }
        }

        imageInfo.mipLevels = desc.numMipLevels;
        imageInfo.arrayLayers = arraySize;

        imageInfo.format = format;

        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.usage = _calcImageUsageFlags(desc.bindFlags, desc.cpuAccessFlags, initData);
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.flags = 0; // Optional

        SLANG_VK_RETURN_ON_FAIL(m_api.vkCreateImage(m_device, &imageInfo, nullptr, &texture->m_image));
    }

    VkMemoryRequirements memRequirements;
    m_api.vkGetImageMemoryRequirements(m_device, texture->m_image, &memRequirements);

    // Allocate the memory
    {
        VkMemoryPropertyFlags reqMemoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

        VkMemoryAllocateInfo allocInfo = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};

        int memoryTypeIndex = m_api.findMemoryTypeIndex(memRequirements.memoryTypeBits, reqMemoryProperties);
        assert(memoryTypeIndex >= 0);

        VkMemoryPropertyFlags actualMemoryProperites = m_api.m_deviceMemoryProperties.memoryTypes[memoryTypeIndex].propertyFlags;

        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = memoryTypeIndex;

        SLANG_VK_RETURN_ON_FAIL(m_api.vkAllocateMemory(m_device, &allocInfo, nullptr, &texture->m_imageMemory));
    }

    // Bind the memory to the image
    m_api.vkBindImageMemory(m_device, texture->m_image, texture->m_imageMemory, 0);

    if (initData)
    {
        List<TextureResource::Size> mipSizes;

        VkCommandBuffer commandBuffer = m_deviceQueue.getCommandBuffer();

        const int numMipMaps = desc.numMipLevels;
        assert(initData->numMips == numMipMaps);

        // Calculate how large the buffer has to be
        size_t bufferSize = 0;
        // Calculate how large an array entry is
        for (int j = 0; j < numMipMaps; ++j)
        {
            const TextureResource::Size mipSize = desc.size.calcMipSize(j);

            const int rowSizeInBytes = Surface::calcRowSize(desc.format, mipSize.width);
            const int numRows =  Surface::calcNumRows(desc.format, mipSize.height);

            mipSizes.add(mipSize);

            bufferSize += (rowSizeInBytes * numRows) * mipSize.depth;
        }


        // Calculate the total size taking into account the array
        bufferSize *= arraySize;

        Buffer uploadBuffer;
        SLANG_RETURN_ON_FAIL(uploadBuffer.init(m_api, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));

        assert(mipSizes.getCount() == numMipMaps);

        // Copy into upload buffer
        {
            int subResourceIndex = 0;

            uint8_t* dstData;
            m_api.vkMapMemory(m_device, uploadBuffer.m_memory, 0, bufferSize, 0, (void**)&dstData);

            for (int i = 0; i < arraySize; ++i)
            {
                for (Index j = 0; j < mipSizes.getCount(); ++j)
                {
                    const auto& mipSize = mipSizes[j];

                    const ptrdiff_t srcRowStride = initData->mipRowStrides[j];
                    const int dstRowSizeInBytes = Surface::calcRowSize(desc.format, mipSize.width);
                    const int numRows = Surface::calcNumRows(desc.format, mipSize.height);

                    for (int k = 0; k < mipSize.depth; k++)
                    {
                        const uint8_t* srcData = (const uint8_t*)(initData->subResources[subResourceIndex]);

                        for (int l = 0; l < numRows; l++)
                        {
                            ::memcpy(dstData, srcData, dstRowSizeInBytes);

                            dstData += dstRowSizeInBytes;
                            srcData += srcRowStride;
                        }

                        subResourceIndex++;
                    }
                }
            }

            m_api.vkUnmapMemory(m_device, uploadBuffer.m_memory);
        }

        _transitionImageLayout(texture->m_image, format, texture->getDesc(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        {
            size_t srcOffset = 0;
            for (int i = 0; i < arraySize; ++i)
            {
                for (Index j = 0; j < mipSizes.getCount(); ++j)
                {
                    const auto& mipSize = mipSizes[j];

                    const int rowSizeInBytes = Surface::calcRowSize(desc.format, mipSize.width);
                    const int numRows = Surface::calcNumRows(desc.format, mipSize.height);

                    // https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/VkBufferImageCopy.html
                    // bufferRowLength and bufferImageHeight specify the data in buffer memory as a subregion of a larger two- or three-dimensional image,
                    // and control the addressing calculations of data in buffer memory. If either of these values is zero, that aspect of the buffer memory
                    // is considered to be tightly packed according to the imageExtent.

                    VkBufferImageCopy region = {};

                    region.bufferOffset = srcOffset;
                    region.bufferRowLength = 0; //rowSizeInBytes;
                    region.bufferImageHeight = 0;

                    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    region.imageSubresource.mipLevel = uint32_t(j);
                    region.imageSubresource.baseArrayLayer = i;
                    region.imageSubresource.layerCount = 1;
                    region.imageOffset = { 0, 0, 0 };
                    region.imageExtent = { uint32_t(mipSize.width), uint32_t(mipSize.height), uint32_t(mipSize.depth) };

                    // Do the copy (do all depths in a single go)
                    m_api.vkCmdCopyBufferToImage(commandBuffer, uploadBuffer.m_buffer, texture->m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

                    // Next
                    srcOffset += rowSizeInBytes * numRows * mipSize.depth;
                }
            }
        }

        _transitionImageLayout(texture->m_image, format, texture->getDesc(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        m_deviceQueue.flushAndWait();
    }

    *outResource = texture.detach();
    return SLANG_OK;
}

Result VKRenderer::createBufferResource(Resource::Usage initialUsage, const BufferResource::Desc& descIn, const void* initData, BufferResource** outResource)
{
    BufferResource::Desc desc(descIn);
    desc.setDefaults(initialUsage);

    const size_t bufferSize = desc.sizeInBytes;

    VkMemoryPropertyFlags reqMemoryProperties = 0;

    VkBufferUsageFlags usage = _calcBufferUsageFlags(desc.bindFlags, desc.cpuAccessFlags, initData);

    switch (initialUsage)
    {
        case Resource::Usage::ConstantBuffer:
        {
            reqMemoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
            break;
        }
        default: break;
    }

    RefPtr<BufferResourceImpl> buffer(new BufferResourceImpl(initialUsage, desc, this));
    SLANG_RETURN_ON_FAIL(buffer->m_buffer.init(m_api, desc.sizeInBytes, usage, reqMemoryProperties));

    if ((desc.cpuAccessFlags & Resource::AccessFlag::Write) || initData)
    {
        SLANG_RETURN_ON_FAIL(buffer->m_uploadBuffer.init(m_api, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
    }

    if (initData)
    {
        // TODO: only create staging buffer if the memory type
        // used for the buffer doesn't let us fill things in
        // directly.
        // Copy into staging buffer
        void* mappedData = nullptr;
        SLANG_VK_CHECK(m_api.vkMapMemory(m_device, buffer->m_uploadBuffer.m_memory, 0, bufferSize, 0, &mappedData));
        ::memcpy(mappedData, initData, bufferSize);
        m_api.vkUnmapMemory(m_device, buffer->m_uploadBuffer.m_memory);

        // Copy from staging buffer to real buffer
        VkCommandBuffer commandBuffer = m_deviceQueue.getCommandBuffer();

        VkBufferCopy copyInfo = {};
        copyInfo.size = bufferSize;
        m_api.vkCmdCopyBuffer(commandBuffer, buffer->m_uploadBuffer.m_buffer, buffer->m_buffer.m_buffer, 1, &copyInfo);

        //flushCommandBuffer(commandBuffer);
    }

    *outResource = buffer.detach();
    return SLANG_OK;
}


VkFilter translateFilterMode(TextureFilteringMode mode)
{
    switch (mode)
    {
    default:
        return VkFilter(0);

#define CASE(SRC, DST) \
    case TextureFilteringMode::SRC: return VK_FILTER_##DST

        CASE(Point, NEAREST);
        CASE(Linear, LINEAR);

#undef CASE
    }
}

VkSamplerMipmapMode translateMipFilterMode(TextureFilteringMode mode)
{
    switch (mode)
    {
    default:
        return VkSamplerMipmapMode(0);

#define CASE(SRC, DST) \
    case TextureFilteringMode::SRC: return VK_SAMPLER_MIPMAP_MODE_##DST

        CASE(Point, NEAREST);
        CASE(Linear, LINEAR);

#undef CASE
    }
}

VkSamplerAddressMode translateAddressingMode(TextureAddressingMode mode)
{
    switch (mode)
    {
    default:
        return VkSamplerAddressMode(0);

#define CASE(SRC, DST) \
    case TextureAddressingMode::SRC: return VK_SAMPLER_ADDRESS_MODE_##DST

    CASE(Wrap,          REPEAT);
    CASE(ClampToEdge,   CLAMP_TO_EDGE);
    CASE(ClampToBorder, CLAMP_TO_BORDER);
    CASE(MirrorRepeat,  MIRRORED_REPEAT);
    CASE(MirrorOnce,    MIRROR_CLAMP_TO_EDGE);

#undef CASE
    }
}

static VkCompareOp translateComparisonFunc(ComparisonFunc func)
{
    switch (func)
    {
    default:
        // TODO: need to report failures
        return VK_COMPARE_OP_ALWAYS;

#define CASE(FROM, TO) \
    case ComparisonFunc::FROM: return VK_COMPARE_OP_##TO

        CASE(Never, NEVER);
        CASE(Less, LESS);
        CASE(Equal, EQUAL);
        CASE(LessEqual, LESS_OR_EQUAL);
        CASE(Greater, GREATER);
        CASE(NotEqual, NOT_EQUAL);
        CASE(GreaterEqual, GREATER_OR_EQUAL);
        CASE(Always, ALWAYS);
#undef CASE
    }
}

Result VKRenderer::createSamplerState(SamplerState::Desc const& desc, SamplerState** outSampler)
{
    VkSamplerCreateInfo samplerInfo = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };

    samplerInfo.magFilter = translateFilterMode(desc.minFilter);
    samplerInfo.minFilter = translateFilterMode(desc.magFilter);

    samplerInfo.addressModeU = translateAddressingMode(desc.addressU);
    samplerInfo.addressModeV = translateAddressingMode(desc.addressV);
    samplerInfo.addressModeW = translateAddressingMode(desc.addressW);

    samplerInfo.anisotropyEnable = desc.maxAnisotropy > 1;
    samplerInfo.maxAnisotropy = (float) desc.maxAnisotropy;

    // TODO: support translation of border color...
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = desc.reductionOp == TextureReductionOp::Comparison;
    samplerInfo.compareOp = translateComparisonFunc(desc.comparisonFunc);
    samplerInfo.mipmapMode = translateMipFilterMode(desc.mipFilter);

    VkSampler sampler;
    SLANG_VK_RETURN_ON_FAIL(m_api.vkCreateSampler(m_device, &samplerInfo, nullptr, &sampler));

    RefPtr<SamplerStateImpl> samplerImpl = new SamplerStateImpl();
    samplerImpl->m_sampler = sampler;
    *outSampler = samplerImpl.detach();
    return SLANG_OK;
}

Result VKRenderer::createTextureView(TextureResource* texture, ResourceView::Desc const& desc, ResourceView** outView)
{
    assert(!"unimplemented");
    return SLANG_FAIL;
}

Result VKRenderer::createBufferView(BufferResource* buffer, ResourceView::Desc const& desc, ResourceView** outView)
{
    auto resourceImpl = (BufferResourceImpl*) buffer;

    // TODO: These should come from the `ResourceView::Desc`
    VkDeviceSize offset = 0;
    VkDeviceSize size = resourceImpl->getDesc().sizeInBytes;

    // There are two different cases we need to think about for buffers.
    //
    // One is when we have a "uniform texel buffer" or "storage texel buffer,"
    // in which case we need to construct a `VkBufferView` to represent the
    // formatting that is applied to the buffer. This case would correspond
    // to a `textureBuffer` or `imageBuffer` in GLSL, and more or less to
    // `Buffer<..>` or `RWBuffer<...>` in HLSL.
    //
    // The other case is a `storage buffer` which is the catch-all for any
    // non-formatted R/W access to a buffer. In GLSL this is a `buffer { ... }`
    // declaration, while in HLSL it covers a bunch of different `RW*Buffer`
    // cases. In these cases we do *not* need a `VkBufferView`, but in
    // order to be compatible with other APIs that require views for any
    // potentially writable access, we will have to create one anyway.
    //
    // We will distinguish the two cases by looking at whether the view
    // is being requested with a format or not.
    //

    switch(desc.type)
    {
    default:
        assert(!"unhandled");
        return SLANG_FAIL;

    case ResourceView::Type::UnorderedAccess:
        // Is this a formatted view?
        //
        if(desc.format == Format::Unknown)
        {
            // Buffer usage that doesn't involve formatting doesn't
            // require a view in Vulkan.
            RefPtr<PlainBufferResourceViewImpl> viewImpl = new PlainBufferResourceViewImpl();
            viewImpl->m_buffer = resourceImpl;
            viewImpl->offset = 0;
            viewImpl->size = size;
            *outView = viewImpl.detach();
            return SLANG_OK;
        }
        //
        // If the view is formatted, then we need to handle
        // it just like we would for a "sampled" buffer:
        //
        // FALLTHROUGH
    case ResourceView::Type::ShaderResource:
        {
            VkBufferViewCreateInfo info = { VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO };

            info.format = VulkanUtil::getVkFormat(desc.format);
            info.buffer = resourceImpl->m_buffer.m_buffer;
            info.offset = offset;
            info.range = size;

            VkBufferView view;
            SLANG_VK_RETURN_ON_FAIL(m_api.vkCreateBufferView(m_device, &info, nullptr, &view));

            RefPtr<TexelBufferResourceViewImpl> viewImpl = new TexelBufferResourceViewImpl();
            viewImpl->m_buffer = resourceImpl;
            viewImpl->m_view = view;
            *outView = viewImpl.detach();
            return SLANG_OK;
        }
        break;
    }
}

Result VKRenderer::createInputLayout(const InputElementDesc* elements, UInt numElements, InputLayout** outLayout)
{
    RefPtr<InputLayoutImpl> layout(new InputLayoutImpl);

    List<VkVertexInputAttributeDescription>& dstVertexDescs = layout->m_vertexDescs;

    size_t vertexSize = 0;
    dstVertexDescs.setCount(numElements);

    for (UInt i = 0; i <  numElements; ++i)
    {
        const InputElementDesc& srcDesc = elements[i];
        VkVertexInputAttributeDescription& dstDesc = dstVertexDescs[i];

        dstDesc.location = uint32_t(i);
        dstDesc.binding = 0;
        dstDesc.format = VulkanUtil::getVkFormat(srcDesc.format);
        if (dstDesc.format == VK_FORMAT_UNDEFINED)
        {
            return SLANG_FAIL;
        }

        dstDesc.offset = uint32_t(srcDesc.offset);

        const size_t elementSize = RendererUtil::getFormatSize(srcDesc.format);
        assert(elementSize > 0);
        const size_t endElement = srcDesc.offset + elementSize;

        vertexSize = (vertexSize < endElement) ? endElement : vertexSize;
    }

    // Work out the overall size
    layout->m_vertexSize = int(vertexSize);
    *outLayout = layout.detach();
    return SLANG_OK;
}

void* VKRenderer::map(BufferResource* bufferIn, MapFlavor flavor)
{
    BufferResourceImpl* buffer = static_cast<BufferResourceImpl*>(bufferIn);
    assert(buffer->m_mapFlavor == MapFlavor::Unknown);

    // Make sure everything has completed before reading...
    m_deviceQueue.flushAndWait();

    const size_t bufferSize = buffer->getDesc().sizeInBytes;

    switch (flavor)
    {
        case MapFlavor::WriteDiscard:
        case MapFlavor::HostWrite:
        {
            if (!buffer->m_uploadBuffer.isInitialized())
            {
                return nullptr;
            }

            void* mappedData = nullptr;
            SLANG_VK_CHECK(m_api.vkMapMemory(m_device, buffer->m_uploadBuffer.m_memory, 0, bufferSize, 0, &mappedData));
            buffer->m_mapFlavor = flavor;
            return mappedData;
        }
        case MapFlavor::HostRead:
        {
            // Make sure there is space in the read buffer
            buffer->m_readBuffer.setCount(bufferSize);

            // create staging buffer
            Buffer staging;

            SLANG_RETURN_NULL_ON_FAIL(staging.init(m_api, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));

            // Copy from real buffer to staging buffer
            VkCommandBuffer commandBuffer = m_deviceQueue.getCommandBuffer();

            VkBufferCopy copyInfo = {};
            copyInfo.size = bufferSize;
            m_api.vkCmdCopyBuffer(commandBuffer, buffer->m_buffer.m_buffer, staging.m_buffer, 1, &copyInfo);

            m_deviceQueue.flushAndWait();

            // Write out the data from the buffer
            void* mappedData = nullptr;
            SLANG_VK_CHECK(m_api.vkMapMemory(m_device, staging.m_memory, 0, bufferSize, 0, &mappedData));

            ::memcpy(buffer->m_readBuffer.getBuffer(), mappedData, bufferSize);
            m_api.vkUnmapMemory(m_device, staging.m_memory);

            buffer->m_mapFlavor = flavor;

            return buffer->m_readBuffer.getBuffer();
        }
        default:
            return nullptr;
    }
}

void VKRenderer::unmap(BufferResource* bufferIn)
{
    BufferResourceImpl* buffer = static_cast<BufferResourceImpl*>(bufferIn);
    assert(buffer->m_mapFlavor != MapFlavor::Unknown);

    const size_t bufferSize = buffer->getDesc().sizeInBytes;

    switch (buffer->m_mapFlavor)
    {
        case MapFlavor::WriteDiscard:
        case MapFlavor::HostWrite:
        {
            m_api.vkUnmapMemory(m_device, buffer->m_uploadBuffer.m_memory);

            // Copy from staging buffer to real buffer
            VkCommandBuffer commandBuffer = m_deviceQueue.getCommandBuffer();

            VkBufferCopy copyInfo = {};
            copyInfo.size = bufferSize;
            m_api.vkCmdCopyBuffer(commandBuffer, buffer->m_uploadBuffer.m_buffer, buffer->m_buffer.m_buffer, 1, &copyInfo);

            // TODO: is this necessary?
            //m_deviceQueue.flushAndWait();
            break;
        }
        default: break;
    }

    // Mark as no longer mapped
    buffer->m_mapFlavor = MapFlavor::Unknown;
}

void VKRenderer::setPrimitiveTopology(PrimitiveTopology topology)
{
    m_primitiveTopology = VulkanUtil::getVkPrimitiveTopology(topology);
}

void VKRenderer::setVertexBuffers(UInt startSlot, UInt slotCount, BufferResource*const* buffers, const UInt* strides, const UInt* offsets)
{
    {
        const Index num = Index(startSlot + slotCount);
        if (num > m_boundVertexBuffers.getCount())
        {
            m_boundVertexBuffers.setCount(num);
        }
    }

    for (Index i = 0; i < Index(slotCount); i++)
    {
        BufferResourceImpl* buffer = static_cast<BufferResourceImpl*>(buffers[i]);
        if (buffer)
        {
            assert(buffer->m_initialUsage == Resource::Usage::VertexBuffer);
        }

        BoundVertexBuffer& boundBuffer = m_boundVertexBuffers[startSlot + i];
        boundBuffer.m_buffer = buffer;
        boundBuffer.m_stride = int(strides[i]);
        boundBuffer.m_offset = int(offsets[i]);
    }
}

void VKRenderer::setIndexBuffer(BufferResource* buffer, Format indexFormat, UInt offset)
{
}

void VKRenderer::setDepthStencilTarget(ResourceView* depthStencilView)
{
}

void VKRenderer::setViewports(UInt count, Viewport const* viewports)
{
    static const int kMaxViewports = 8; // TODO: base on device caps
    assert(count <= kMaxViewports);

    VkViewport vkViewports[kMaxViewports];
    for(UInt ii = 0; ii < count; ++ii)
    {
        auto& inViewport = viewports[ii];
        auto& vkViewport = vkViewports[ii];

        vkViewport.x        = inViewport.originX;
        vkViewport.y        = inViewport.originY;
        vkViewport.width    = inViewport.extentX;
        vkViewport.height   = inViewport.extentY;
        vkViewport.minDepth = inViewport.minZ;
        vkViewport.maxDepth = inViewport.maxZ;
    }

    VkCommandBuffer commandBuffer = m_deviceQueue.getCommandBuffer();
    m_api.vkCmdSetViewport(commandBuffer, 0, uint32_t(count), vkViewports);
}

void VKRenderer::setScissorRects(UInt count, ScissorRect const* rects)
{
    static const int kMaxScissorRects = 8; // TODO: base on device caps
    assert(count <= kMaxScissorRects);

    VkRect2D vkRects[kMaxScissorRects];
    for(UInt ii = 0; ii < count; ++ii)
    {
        auto& inRect = rects[ii];
        auto& vkRect = vkRects[ii];

        vkRect.offset.x         = int32_t(inRect.minX);
        vkRect.offset.y         = int32_t(inRect.minY);
        vkRect.extent.width     = uint32_t(inRect.maxX - inRect.minX);
        vkRect.extent.height    = uint32_t(inRect.maxY - inRect.minY);
    }

    VkCommandBuffer commandBuffer = m_deviceQueue.getCommandBuffer();
    m_api.vkCmdSetScissor(commandBuffer, 0, uint32_t(count), vkRects);
}

void VKRenderer::setPipelineState(PipelineType pipelineType, PipelineState* state)
{
    m_currentPipeline = (PipelineStateImpl*)state;
}

void VKRenderer::draw(UInt vertexCount, UInt startVertex = 0)
{
    auto pipeline = m_currentPipeline;
    if (!pipeline || pipeline->m_shaderProgram->m_pipelineType != PipelineType::Graphics)
    {
        assert(!"Invalid render pipeline");
        return;
    }

    SLANG_RETURN_VOID_ON_FAIL(_beginPass());

    // Also create descriptor sets based on the given pipeline layout
    VkCommandBuffer commandBuffer = m_deviceQueue.getCommandBuffer();

    m_api.vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->m_pipeline);

    auto pipelineLayoutImpl = pipeline->m_pipelineLayout.Ptr();
    m_api.vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayoutImpl->m_pipelineLayout,
        0, uint32_t(pipelineLayoutImpl->m_descriptorSetCount),
        &m_currentDescriptorSets[0],
        0, nullptr);

    // Bind the vertex buffer
    if (m_boundVertexBuffers.getCount() > 0 && m_boundVertexBuffers[0].m_buffer)
    {
        const BoundVertexBuffer& boundVertexBuffer = m_boundVertexBuffers[0];

        VkBuffer vertexBuffers[] = { boundVertexBuffer.m_buffer->m_buffer.m_buffer };
        VkDeviceSize offsets[] = { VkDeviceSize(boundVertexBuffer.m_offset) };

        m_api.vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    }

    m_api.vkCmdDraw(commandBuffer, static_cast<uint32_t>(vertexCount), 1, 0, 0);

    _endPass();
}

void VKRenderer::drawIndexed(UInt indexCount, UInt startIndex, UInt baseVertex)
{
}

void VKRenderer::dispatchCompute(int x, int y, int z)
{
    auto pipeline = m_currentPipeline;
    if (!pipeline || pipeline->m_shaderProgram->m_pipelineType != PipelineType::Compute)
    {
        assert(!"Invalid compute pipeline");
        return;
    }

    // Also create descriptor sets based on the given pipeline layout
    VkCommandBuffer commandBuffer = m_deviceQueue.getCommandBuffer();

    m_api.vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->m_pipeline);

    auto pipelineLayoutImpl = pipeline->m_pipelineLayout.Ptr();
    m_api.vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayoutImpl->m_pipelineLayout,
        0, uint32_t(pipelineLayoutImpl->m_descriptorSetCount),
        &m_currentDescriptorSets[0],
        0, nullptr);

    m_api.vkCmdDispatch(commandBuffer, x, y, z);
}

static VkImageViewType _calcImageViewType(TextureResource::Type type, const TextureResource::Desc& desc)
{
    switch (type)
    {
        case Resource::Type::Texture1D:        return desc.arraySize > 1 ? VK_IMAGE_VIEW_TYPE_1D_ARRAY : VK_IMAGE_VIEW_TYPE_1D;
        case Resource::Type::Texture2D:        return desc.arraySize > 1 ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
        case Resource::Type::TextureCube:      return desc.arraySize > 1 ? VK_IMAGE_VIEW_TYPE_CUBE_ARRAY : VK_IMAGE_VIEW_TYPE_CUBE;
        case Resource::Type::Texture3D:
        {
            // Can't have an array and 3d texture
            assert(desc.arraySize <= 1);
            if (desc.arraySize <= 1)
            {
                return VK_IMAGE_VIEW_TYPE_3D;
            }
            break;
        }
        default: break;
    }

    return VK_IMAGE_VIEW_TYPE_MAX_ENUM;
}

#if 0
BindingState* VKRenderer::createBindingState(const BindingState::Desc& bindingStateDesc)
{
    RefPtr<BindingStateImpl> bindingState(new BindingStateImpl(bindingStateDesc, &m_api));

    const auto& srcBindings = bindingStateDesc.m_bindings;
    const int numBindings = int(srcBindings.Count());

    auto& dstDetails = bindingState->m_bindingDetails;
    dstDetails.SetSize(numBindings);

    for (int i = 0; i < numBindings; ++i)
    {
        auto& dstDetail = dstDetails[i];
        const auto& srcBinding = srcBindings[i];

        switch (srcBinding.bindingType)
        {
            case BindingType::Buffer:
            {
                if (!srcBinding.resource || !srcBinding.resource->isBuffer())
                {
                    assert(!"Needs to have a buffer resource set");
                    return nullptr;
                }

                BufferResourceImpl* bufferResource = static_cast<BufferResourceImpl*>(srcBinding.resource.Ptr());
                const BufferResource::Desc& bufferResourceDesc = bufferResource->getDesc();

                if (bufferResourceDesc.bindFlags & Resource::BindFlag::UnorderedAccess)
                {
                    // VkBufferView uav

                    VkBufferViewCreateInfo info = { VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO };

                    info.format = VK_FORMAT_R32_SFLOAT;
                    // TODO:
                    // Not sure how to handle typeless?
                    if (bufferResourceDesc.elementSize == 0)
                    {
                        info.format = VK_FORMAT_R32_SFLOAT;  // DXGI_FORMAT_R32_TYPELESS ?
                    }

                    info.buffer = bufferResource->m_buffer.m_buffer;
                    info.offset = 0;
                    info.range = bufferResourceDesc.sizeInBytes;

                    SLANG_VK_RETURN_NULL_ON_FAIL(m_api.vkCreateBufferView(m_device, &info, nullptr, &dstDetail.m_uav));
                }

                // TODO: Setup views.
                // VkImageView srv


                break;
            }
            case BindingType::Sampler:
            {
                VkSamplerCreateInfo samplerInfo = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };

                samplerInfo.magFilter = VK_FILTER_LINEAR;
                samplerInfo.minFilter = VK_FILTER_LINEAR;

                samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
                samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
                samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

                samplerInfo.anisotropyEnable = VK_FALSE;
                samplerInfo.maxAnisotropy = 1;

                samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
                samplerInfo.unnormalizedCoordinates = VK_FALSE;
                samplerInfo.compareEnable = VK_FALSE;
                samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
                samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

                SLANG_VK_RETURN_NULL_ON_FAIL(m_api.vkCreateSampler(m_device, &samplerInfo, nullptr, &dstDetail.m_sampler));

                break;
            }
            case BindingType::Texture:
            {
                if (!srcBinding.resource || !srcBinding.resource->isTexture())
                {
                    assert(!"Needs to have a texture resource set");
                    return nullptr;
                }

                TextureResourceImpl* textureResource = static_cast<TextureResourceImpl*>(srcBinding.resource.Ptr());
                const TextureResource::Desc& texDesc = textureResource->getDesc();

                VkImageViewType imageViewType = _calcImageViewType(textureResource->getType(), texDesc);
                if (imageViewType == VK_IMAGE_VIEW_TYPE_MAX_ENUM)
                {
                    assert(!"Invalid view type");
                    return nullptr;
                }
                const VkFormat format = VulkanUtil::getVkFormat(texDesc.format);
                if (format == VK_FORMAT_UNDEFINED)
                {
                    assert(!"Unhandled image format");
                    return nullptr;
                }

                // Create the image view

                VkImageViewCreateInfo viewInfo = {};
                viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                viewInfo.image = textureResource->m_image;
                viewInfo.viewType = imageViewType;
                viewInfo.format = format;
                viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                viewInfo.subresourceRange.baseMipLevel = 0;
                viewInfo.subresourceRange.levelCount = 1;
                viewInfo.subresourceRange.baseArrayLayer = 0;
                viewInfo.subresourceRange.layerCount = 1;

                viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
                viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
                viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
                viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

                SLANG_VK_RETURN_NULL_ON_FAIL(m_api.vkCreateImageView(m_device, &viewInfo, nullptr, &dstDetail.m_srv));

                break;
            }
            case BindingType::CombinedTextureSampler:
            {
                assert(!"not implemented");
                return nullptr;
            }
        }
    }

    return bindingState.detach();;
}
#endif

static VkDescriptorType translateDescriptorType(DescriptorSlotType type)
{
    switch(type)
    {
    default:
        return VK_DESCRIPTOR_TYPE_MAX_ENUM;

#define CASE(SRC, DST) \
    case DescriptorSlotType::SRC: return VK_DESCRIPTOR_TYPE_##DST

        CASE(Sampler,               SAMPLER);
        CASE(CombinedImageSampler,  COMBINED_IMAGE_SAMPLER);
        CASE(SampledImage,          SAMPLED_IMAGE);
        CASE(StorageImage,          STORAGE_IMAGE);
        CASE(UniformTexelBuffer,    UNIFORM_TEXEL_BUFFER);
        CASE(StorageTexelBuffer,    STORAGE_TEXEL_BUFFER);
        CASE(UniformBuffer,         UNIFORM_BUFFER);
        CASE(StorageBuffer,         STORAGE_BUFFER);
        CASE(DynamicUniformBuffer,  UNIFORM_BUFFER_DYNAMIC);
        CASE(DynamicStorageBuffer,  STORAGE_BUFFER_DYNAMIC);
        CASE(InputAttachment,       INPUT_ATTACHMENT);

#undef CASE
    }
}

Result VKRenderer::createDescriptorSetLayout(const DescriptorSetLayout::Desc& desc, DescriptorSetLayout** outLayout)
{
    RefPtr<DescriptorSetLayoutImpl> descriptorSetLayoutImpl = new DescriptorSetLayoutImpl(m_api);

    Slang::List<VkDescriptorSetLayoutBinding> dstBindings;

    uint32_t descriptorCountForTypes[VK_DESCRIPTOR_TYPE_RANGE_SIZE] = { 0, };

    UInt rangeCount = desc.slotRangeCount;
    for(UInt rr = 0; rr < rangeCount; ++rr)
    {
        auto& srcRange = desc.slotRanges[rr];

        VkDescriptorType dstDescriptorType = translateDescriptorType(srcRange.type);

        VkDescriptorSetLayoutBinding dstBinding;
        dstBinding.binding = uint32_t(rr);
        dstBinding.descriptorType = dstDescriptorType;
        dstBinding.descriptorCount = uint32_t(srcRange.count);
        dstBinding.stageFlags = VK_SHADER_STAGE_ALL;
        dstBinding.pImmutableSamplers = nullptr;

        descriptorCountForTypes[dstDescriptorType] += uint32_t(srcRange.count);

        dstBindings.add(dstBinding);

        DescriptorSetLayoutImpl::RangeInfo rangeInfo;
        rangeInfo.descriptorType = dstDescriptorType;
        descriptorSetLayoutImpl->m_ranges.add(rangeInfo);
    }

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
    descriptorSetLayoutInfo.bindingCount = uint32_t(dstBindings.getCount());
    descriptorSetLayoutInfo.pBindings = dstBindings.getBuffer();

    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    SLANG_VK_CHECK(m_api.vkCreateDescriptorSetLayout(m_device, &descriptorSetLayoutInfo, nullptr, &descriptorSetLayout));

    // Create a pool while we are at it, to allocate descriptor sets of this type.

    VkDescriptorPoolSize poolSizes[VK_DESCRIPTOR_TYPE_RANGE_SIZE];
    uint32_t poolSizeCount = 0;
    for (int ii = 0; ii < SLANG_COUNT_OF(descriptorCountForTypes); ++ii)
    {
        auto descriptorCount = descriptorCountForTypes[ii];
        if (descriptorCount > 0)
        {
            poolSizes[poolSizeCount].type = VkDescriptorType(ii);
            poolSizes[poolSizeCount].descriptorCount = descriptorCount;
            poolSizeCount++;
        }
    }

    VkDescriptorPoolCreateInfo descriptorPoolInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
    descriptorPoolInfo.maxSets = 128; // TODO: actually pick a size.
    descriptorPoolInfo.poolSizeCount = poolSizeCount;
    descriptorPoolInfo.pPoolSizes = &poolSizes[0];

    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    SLANG_VK_CHECK(m_api.vkCreateDescriptorPool(m_device, &descriptorPoolInfo, nullptr, &descriptorPool));

    descriptorSetLayoutImpl->m_descriptorSetLayout = descriptorSetLayout;
    descriptorSetLayoutImpl->m_descriptorPool = descriptorPool;

    *outLayout = descriptorSetLayoutImpl.detach();
    return SLANG_OK;
}

Result VKRenderer::createPipelineLayout(const PipelineLayout::Desc& desc, PipelineLayout** outLayout)
{
    UInt descriptorSetCount = desc.descriptorSetCount;

    VkDescriptorSetLayout descriptorSetLayouts[kMaxDescriptorSets];
    for(UInt ii = 0; ii < descriptorSetCount; ++ii)
    {
        descriptorSetLayouts[ii] = ((DescriptorSetLayoutImpl*) desc.descriptorSets[ii].layout)->m_descriptorSetLayout;
    }

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
    pipelineLayoutInfo.setLayoutCount = uint32_t(desc.descriptorSetCount);
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayouts[0];

    VkPipelineLayout pipelineLayout;
    SLANG_VK_CHECK(m_api.vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, nullptr, &pipelineLayout));

    RefPtr<PipelineLayoutImpl> pipelineLayoutImpl = new PipelineLayoutImpl(m_api);
    pipelineLayoutImpl->m_pipelineLayout = pipelineLayout;
    pipelineLayoutImpl->m_descriptorSetCount = descriptorSetCount;

    *outLayout = pipelineLayoutImpl.detach();
    return SLANG_OK;
}

Result VKRenderer::createDescriptorSet(DescriptorSetLayout* layout, DescriptorSet** outDescriptorSet)
{
    auto layoutImpl = (DescriptorSetLayoutImpl*)layout;

    VkDescriptorSetAllocateInfo descriptorSetAllocInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
    descriptorSetAllocInfo.descriptorPool = layoutImpl->m_descriptorPool;
    descriptorSetAllocInfo.descriptorSetCount = 1;
    descriptorSetAllocInfo.pSetLayouts = &layoutImpl->m_descriptorSetLayout;

    VkDescriptorSet descriptorSet;
    SLANG_VK_CHECK(m_api.vkAllocateDescriptorSets(m_device, &descriptorSetAllocInfo, &descriptorSet));

    RefPtr<DescriptorSetImpl> descriptorSetImpl = new DescriptorSetImpl(this);
    descriptorSetImpl->m_layout = layoutImpl;
    descriptorSetImpl->m_descriptorSet = descriptorSet;
    *outDescriptorSet = descriptorSetImpl.detach();
    return SLANG_OK;
}

/* static */VKRenderer::DescriptorSetImpl::Binding::Type VKRenderer::DescriptorSetImpl::_getBindingType(RefObject* ptr)
{
    typedef Binding::Type Type;

    if (ptr)
    {
        if (dynamic_cast<ResourceView*>(ptr))
        {
            return Type::ResourceView;
        }
        else if (dynamic_cast<BufferResource*>(ptr))
        {
            return Type::BufferResource;
        }
        else if (dynamic_cast<SamplerState*>(ptr))
        {
            return Type::SamplerState;
        }
    }
    return Type::Unknown;
}

void VKRenderer::DescriptorSetImpl::_setBinding(Binding::Type type, UInt range, UInt index, RefObject* ptr)
{
    SLANG_ASSERT(ptr == nullptr || _getBindingType(ptr) == type);

    const Index numBindings = m_bindings.getCount();
    for (Index i = 0; i < numBindings; ++i)
    {
        Binding& binding = m_bindings[i];

        if (binding.type == type && binding.range == uint32_t(range) && binding.index == uint32_t(index))
        {
            if (ptr)
            {
                binding.obj = ptr;
            }
            else
            {
                m_bindings.removeAt(i);
            }

            return;
        }
    }

    // If an entry is not found, and we have a pointer, create an entry
    if (ptr)
    {
        Binding binding;
        binding.type = type;
        binding.range = uint32_t(range);
        binding.index = uint32_t(index);
        binding.obj = ptr;

        m_bindings.add(binding);
    }
}

void VKRenderer::DescriptorSetImpl::setConstantBuffer(UInt range, UInt index, BufferResource* buffer)
{
    auto bufferImpl = (BufferResourceImpl*)buffer;

    VkDescriptorBufferInfo bufferInfo = {};
    bufferInfo.buffer = bufferImpl->m_buffer.m_buffer;
    bufferInfo.offset = 0;
    bufferInfo.range = bufferImpl->getDesc().sizeInBytes;

    VkWriteDescriptorSet writeInfo = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
    writeInfo.dstSet = m_descriptorSet;
    writeInfo.dstBinding = uint32_t(range);
    writeInfo.dstArrayElement = uint32_t(index);
    writeInfo.descriptorCount = 1;
    writeInfo.descriptorType = m_layout->m_ranges[range].descriptorType;
    writeInfo.pBufferInfo = &bufferInfo;

    m_renderer->m_api.vkUpdateDescriptorSets(m_renderer->m_device, 1, &writeInfo, 0, nullptr);

    _setBinding(Binding::Type::BufferResource, range, index, buffer);
}

void VKRenderer::DescriptorSetImpl::setResource(UInt range, UInt index, ResourceView* view)
{
    auto viewImpl = (ResourceViewImpl*)view;
    switch (viewImpl->m_type)
    {
    case ResourceViewImpl::ViewType::Texture:
        {
            auto textureViewImpl = (TextureResourceViewImpl*)viewImpl;
            VkDescriptorImageInfo imageInfo = {};
            imageInfo.imageView = textureViewImpl->m_view;
            imageInfo.imageLayout = textureViewImpl->m_layout;
            //            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            VkWriteDescriptorSet writeInfo = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
            writeInfo.dstSet = m_descriptorSet;
            writeInfo.dstBinding = uint32_t(range);
            writeInfo.dstArrayElement = uint32_t(index);
            writeInfo.descriptorCount = 1;
            writeInfo.descriptorType = m_layout->m_ranges[range].descriptorType;
            writeInfo.pImageInfo = &imageInfo;

            m_renderer->m_api.vkUpdateDescriptorSets(m_renderer->m_device, 1, &writeInfo, 0, nullptr);
        }
        break;

    case ResourceViewImpl::ViewType::TexelBuffer:
        {
            auto bufferViewImpl = (TexelBufferResourceViewImpl*)viewImpl;

            VkWriteDescriptorSet writeInfo = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
            writeInfo.dstSet = m_descriptorSet;
            writeInfo.dstBinding = uint32_t(range);
            writeInfo.dstArrayElement = uint32_t(index);
            writeInfo.descriptorCount = 1;
            writeInfo.descriptorType = m_layout->m_ranges[range].descriptorType;
            writeInfo.pTexelBufferView = &bufferViewImpl->m_view;

            m_renderer->m_api.vkUpdateDescriptorSets(m_renderer->m_device, 1, &writeInfo, 0, nullptr);
        }
        break;

    case ResourceViewImpl::ViewType::PlainBuffer:
        {
            auto bufferViewImpl = (PlainBufferResourceViewImpl*) viewImpl;

            VkDescriptorBufferInfo bufferInfo = {};
            bufferInfo.buffer = bufferViewImpl->m_buffer->m_buffer.m_buffer;
            bufferInfo.offset = bufferViewImpl->offset;
            bufferInfo.range = bufferViewImpl->size;

            VkWriteDescriptorSet writeInfo = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
            writeInfo.dstSet = m_descriptorSet;
            writeInfo.dstBinding = uint32_t(range);
            writeInfo.dstArrayElement = uint32_t(index);
            writeInfo.descriptorCount = 1;
            writeInfo.descriptorType = m_layout->m_ranges[range].descriptorType;
            writeInfo.pBufferInfo = &bufferInfo;

            m_renderer->m_api.vkUpdateDescriptorSets(m_renderer->m_device, 1, &writeInfo, 0, nullptr);
        }
        break;

    }

    _setBinding(Binding::Type::ResourceView, range, index, view);
}

void VKRenderer::DescriptorSetImpl::setSampler(UInt range, UInt index, SamplerState* sampler)
{

    _setBinding(Binding::Type::SamplerState, range, index, sampler);
}

void VKRenderer::DescriptorSetImpl::setCombinedTextureSampler(
    UInt range,
    UInt index,
    ResourceView*   textureView,
    SamplerState*   sampler)
{

    _setBinding(Binding::Type::SamplerState, range, index, sampler);
    _setBinding(Binding::Type::ResourceView, range, index, textureView);
}

void VKRenderer::setDescriptorSet(PipelineType pipelineType, PipelineLayout* layout, UInt index, DescriptorSet* descriptorSet)
{
    // Ideally this should eventually be as simple as:
    //
    //      m_api.vkCmdBindDescriptorSets(
    //          commandBuffer,
    //          translatePipelineBindPoint(pipelineType),
    //          layout->m_pipelineLayout,
    //          index,
    //          1,
    //          ((DescriptorSetImpl*) descriptorSet)->m_descriptorSet,
    //          0,
    //          nullptr);
    //
    // For now we are lazily flushing state right before drawing, so
    // we will hang onto the parameters that were passed in and then
    // use them later.
    //

    auto descriptorSetImpl = (DescriptorSetImpl*)descriptorSet;
    m_currentDescriptorSetImpls[index] = descriptorSetImpl;
    m_currentDescriptorSets[index] = descriptorSetImpl->m_descriptorSet;
}

Result VKRenderer::createProgram(const ShaderProgram::Desc& desc, ShaderProgram** outProgram)
{
    RefPtr<ShaderProgramImpl> impl = new ShaderProgramImpl(desc.pipelineType);
    if( desc.pipelineType == PipelineType::Compute)
    {
        auto computeKernel = desc.findKernel(StageType::Compute);
        impl->m_compute = compileEntryPoint(*computeKernel, VK_SHADER_STAGE_COMPUTE_BIT, impl->m_buffers[0]);
    }
    else
    {
        auto vertexKernel = desc.findKernel(StageType::Vertex);
        auto fragmentKernel = desc.findKernel(StageType::Fragment);

        impl->m_vertex = compileEntryPoint(*vertexKernel, VK_SHADER_STAGE_VERTEX_BIT, impl->m_buffers[0]);
        impl->m_fragment = compileEntryPoint(*fragmentKernel, VK_SHADER_STAGE_FRAGMENT_BIT, impl->m_buffers[1]);
    }
    *outProgram = impl.detach();
    return SLANG_OK;
}

Result VKRenderer::createGraphicsPipelineState(const GraphicsPipelineStateDesc& desc, PipelineState** outState)
{
    VkPipelineCache pipelineCache = VK_NULL_HANDLE;

    auto programImpl = (ShaderProgramImpl*) desc.program;
    auto pipelineLayoutImpl = (PipelineLayoutImpl*) desc.pipelineLayout;
    auto inputLayoutImpl = (InputLayoutImpl*) desc.inputLayout;

    const int width = int(desc.framebufferWidth);
    const int height = int(desc.framebufferHeight);

    // Shader Stages
    //
    // Currently only handles vertex/fragment.

    static const uint32_t kMaxShaderStages = 2;
    VkPipelineShaderStageCreateInfo shaderStages[kMaxShaderStages];

    uint32_t shaderStageCount = 0;
    shaderStages[shaderStageCount++] = programImpl->m_vertex;
    shaderStages[shaderStageCount++] = programImpl->m_fragment;

    // VertexBuffer/s
    // Currently only handles one

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;

    VkVertexInputBindingDescription vertexInputBindingDescription;

    if (inputLayoutImpl)
    {
        vertexInputBindingDescription.binding = 0;
        vertexInputBindingDescription.stride = inputLayoutImpl->m_vertexSize;
        vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        const auto& srcAttributeDescs = inputLayoutImpl->m_vertexDescs;

        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.pVertexBindingDescriptions = &vertexInputBindingDescription;

        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(srcAttributeDescs.getCount());
        vertexInputInfo.pVertexAttributeDescriptions = srcAttributeDescs.getBuffer();
    }

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)width;
    viewport.height = (float)height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = { 0, 0 };
    scissor.extent = { uint32_t(width), uint32_t(height) };

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_NONE;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    VkGraphicsPipelineCreateInfo pipelineInfo = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };

    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.layout = pipelineLayoutImpl->m_pipelineLayout;
    pipelineInfo.renderPass = m_renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    VkPipeline pipeline = VK_NULL_HANDLE;
    SLANG_VK_CHECK(m_api.vkCreateGraphicsPipelines(m_device, pipelineCache, 1, &pipelineInfo, nullptr, &pipeline));

    RefPtr<PipelineStateImpl> pipelineStateImpl = new PipelineStateImpl(m_api);
    pipelineStateImpl->m_pipeline = pipeline;
    pipelineStateImpl->m_pipelineLayout = pipelineLayoutImpl;
    pipelineStateImpl->m_shaderProgram = programImpl;
    *outState = pipelineStateImpl.detach();
    return SLANG_OK;
}

Result VKRenderer::createComputePipelineState(const ComputePipelineStateDesc& desc, PipelineState** outState)
{
    VkPipelineCache pipelineCache = VK_NULL_HANDLE;

    auto programImpl = (ShaderProgramImpl*) desc.program;
    auto pipelineLayoutImpl = (PipelineLayoutImpl*) desc.pipelineLayout;

    VkComputePipelineCreateInfo computePipelineInfo = { VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO };
    computePipelineInfo.stage = programImpl->m_compute;
    computePipelineInfo.layout = pipelineLayoutImpl->m_pipelineLayout;

    VkPipeline pipeline = VK_NULL_HANDLE;
    SLANG_VK_CHECK(m_api.vkCreateComputePipelines(m_device, pipelineCache, 1, &computePipelineInfo, nullptr, &pipeline));

    RefPtr<PipelineStateImpl> pipelineStateImpl = new PipelineStateImpl(m_api);
    pipelineStateImpl->m_pipeline = pipeline;
    pipelineStateImpl->m_pipelineLayout = pipelineLayoutImpl;
    pipelineStateImpl->m_shaderProgram = programImpl;
    *outState = pipelineStateImpl.detach();
    return SLANG_OK;
}


#if 0
    else if (m_currentProgram->m_pipelineType == PipelineType::Graphics)
    {
        // Create the graphics pipeline

        const int width = m_swapChain.getWidth();
        const int height = m_swapChain.getHeight();





        //


    }
    else
    {
        assert(!"Unhandled program type");
        return SLANG_FAIL;
    }

    pipelineOut = pipeline;
    return SLANG_OK;


#endif

} // renderer_test
