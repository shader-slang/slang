// render-vk.cpp
#include "render-vk.h"

//WORKING:#include "options.h"
#include "render.h"

#include "../../source/core/smart-pointer.h"

#include "vk-api.h"
#include "vk-util.h"
#include "vk-device-queue.h"
#include "vk-swap-chain.h"

#include "surface.h"

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

namespace slang_graphics {
using namespace Slang;

class VKRenderer : public Renderer
{
public:
    enum { kMaxRenderTargets = 8, kMaxAttachments = kMaxRenderTargets + 1 };

    // Renderer    implementation
    virtual SlangResult initialize(const Desc& desc, void* inWindowHandle) override;
    virtual void setClearColor(const float color[4]) override;
    virtual void clearFrame() override;
    virtual void presentFrame() override;
    virtual TextureResource* createTextureResource(Resource::Usage initialUsage, const TextureResource::Desc& desc, const TextureResource::Data* initData) override;
    virtual BufferResource* createBufferResource(Resource::Usage initialUsage, const BufferResource::Desc& bufferDesc, const void* initData) override;
    virtual SlangResult captureScreenSurface(Surface& surface) override;
    virtual InputLayout* createInputLayout(const InputElementDesc* inputElements, UInt inputElementCount) override;
    virtual BindingState* createBindingState(const BindingState::Desc& bindingStateDesc) override;
    virtual ShaderProgram* createProgram(const ShaderProgram::Desc& desc) override;
    virtual void* map(BufferResource* buffer, MapFlavor flavor) override;
    virtual void unmap(BufferResource* buffer) override;
    virtual void setInputLayout(InputLayout* inputLayout) override;
    virtual void setPrimitiveTopology(PrimitiveTopology topology) override;
    virtual void setBindingState(BindingState* state);
    virtual void setVertexBuffers(UInt startSlot, UInt slotCount, BufferResource*const* buffers, const UInt* strides, const UInt* offsets) override;
    virtual void setShaderProgram(ShaderProgram* inProgram) override;
    virtual void draw(UInt vertexCount, UInt startVertex) override;
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

    struct BindingDetail
    {
        VkImageView     m_srv = VK_NULL_HANDLE;
        VkBufferView    m_uav = VK_NULL_HANDLE;
        VkSampler       m_sampler = VK_NULL_HANDLE;
    };

    class BindingStateImpl: public BindingState
    {
		public:
        typedef BindingState Parent;

		BindingStateImpl(const Desc& desc, const VulkanApi* api):
            Parent(desc),
			m_api(api)
		{
		}
        ~BindingStateImpl()
        {
            for (int i = 0; i < int(m_bindingDetails.Count()); ++i)
            {
                BindingDetail& detail = m_bindingDetails[i];
                if (detail.m_sampler != VK_NULL_HANDLE)
                {
                    m_api->vkDestroySampler(m_api->m_device, detail.m_sampler, nullptr);
                }
                if (detail.m_srv != VK_NULL_HANDLE)
                {
                    m_api->vkDestroyImageView(m_api->m_device, detail.m_srv, nullptr);
                }
                if (detail.m_uav != VK_NULL_HANDLE)
                {
                    m_api->vkDestroyBufferView(m_api->m_device, detail.m_uav, nullptr);
                }
            }
        }

        const VulkanApi* m_api;
        List<BindingDetail> m_bindingDetails;
    };

    struct BoundVertexBuffer
    {
        RefPtr<BufferResourceImpl> m_buffer;
        int m_stride;
        int m_offset;
    };

    class Pipeline : public RefObject
    {
    public:
        Pipeline(const VulkanApi& api):
            m_api(&api)
        {
        }
        ~Pipeline()
        {
            if (m_pipeline != VK_NULL_HANDLE)
            {
                m_api->vkDestroyPipeline(m_api->m_device, m_pipeline, nullptr);
            }
            if (m_descriptorPool != VK_NULL_HANDLE)
            {
                m_api->vkDestroyDescriptorPool(m_api->m_device, m_descriptorPool, nullptr);
            }
            if (m_pipelineLayout != VK_NULL_HANDLE)
            {
                m_api->vkDestroyPipelineLayout(m_api->m_device, m_pipelineLayout, nullptr);
            }
            if(m_descriptorSetLayout != VK_NULL_HANDLE)
            {
                m_api->vkDestroyDescriptorSetLayout(m_api->m_device, m_descriptorSetLayout, nullptr);
            }
        }

        const VulkanApi* m_api;

        VkPrimitiveTopology m_primitiveTopology;
        RefPtr<BindingStateImpl> m_bindingState;
        RefPtr<InputLayoutImpl> m_inputLayout;
        RefPtr<ShaderProgramImpl> m_shaderProgram;

        VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
        VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
        VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
        VkDescriptorSet m_descriptorSet = VK_NULL_HANDLE;
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
    Pipeline* _getPipeline();
    bool _isEqual(const Pipeline& pipeline) const;
    Slang::Result _createPipeline(RefPtr<Pipeline>& pipelineOut);
    void _beginRender();
    void _endRender();

    Slang::Result _beginPass();
    void _endPass();
    void _transitionImageLayout(VkImage image, VkFormat format, const TextureResource::Desc& desc, VkImageLayout oldLayout, VkImageLayout newLayout);

    VkDebugReportCallbackEXT m_debugReportCallback;

    RefPtr<InputLayoutImpl> m_currentInputLayout;
    RefPtr<BindingStateImpl> m_currentBindingState;
    RefPtr<ShaderProgramImpl> m_currentProgram;

    List<RefPtr<Pipeline> > m_pipelineCache;
    Pipeline* m_currentPipeline = nullptr;

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

bool VKRenderer::_isEqual(const Pipeline& pipeline) const
{
    return
        pipeline.m_bindingState == m_currentBindingState &&
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
    pipeline->m_bindingState = m_currentBindingState;
    pipeline->m_shaderProgram = m_currentProgram;
    pipeline->m_inputLayout = m_currentInputLayout;

    // Must be equal at this point if all the items are correctly set in pipeline
    assert(_isEqual(*pipeline));

    // First create a pipeline layout based on what is bound

    const auto& srcDetails = m_currentBindingState->m_bindingDetails;
    const auto& srcBindings = m_currentBindingState->getDesc().m_bindings;

    const int numBindings = int(srcBindings.Count());

    int numBuffers = 0;
    int numImages = 0;

    int numDescriptorByType[VK_DESCRIPTOR_TYPE_RANGE_SIZE] = { 0, };

    Slang::List<VkDescriptorSetLayoutBinding> dstBindings;
    for (int i = 0; i < numBindings; ++i)
    {
        const auto& srcDetail = srcDetails[i];
        const auto& srcBinding = srcBindings[i];

        VkDescriptorSetLayoutBinding dstBinding = {};

        dstBinding.descriptorCount = 1;

        switch (srcBinding.bindingType)
        {
            case BindingType::Buffer:
            {
                BufferResourceImpl* bufferResource = static_cast<BufferResourceImpl*>(srcBinding.resource.Ptr());
                const BufferResource::Desc& bufferResourceDesc = bufferResource->getDesc();

                if (bufferResourceDesc.bindFlags & Resource::BindFlag::UnorderedAccess)
                {
                    dstBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    dstBinding.stageFlags = VK_SHADER_STAGE_ALL;
                    dstBindings.Add(dstBinding);

                    numDescriptorByType[dstBinding.descriptorType] ++;
                    numBuffers++;
                }
                else if (bufferResourceDesc.bindFlags & Resource::BindFlag::ConstantBuffer)
                {
                    dstBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    dstBinding.stageFlags = VK_SHADER_STAGE_ALL;
                    dstBindings.Add(dstBinding);

                    numDescriptorByType[dstBinding.descriptorType] ++;
                    numBuffers++;
                }
                break;
            }
            case BindingType::Texture:
            {
                dstBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
                dstBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
                dstBindings.Add(dstBinding);

                numDescriptorByType[dstBinding.descriptorType] ++;
                numImages++;
                break;
            }
            case BindingType::Sampler:
            {
                dstBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
                dstBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
                dstBindings.Add(dstBinding);

                numDescriptorByType[dstBinding.descriptorType] ++;
                numImages++;
                break;
            }

            case BindingType::CombinedTextureSampler:
            {
                dstBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                dstBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
                dstBindings.Add(dstBinding);

                numDescriptorByType[dstBinding.descriptorType] ++;
                numImages++;
                break;
            }
            default:
            {
                assert(!"Unhandled type");
                return SLANG_FAIL;
            }
        }
    }

    // Create a descriptor pool for allocating sets
    {
#if 0
        VkDescriptorPoolSize poolSizes[] =
        {
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 128 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 128 },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 128 },
    };
#endif

        List<VkDescriptorPoolSize> poolSizes;
        for (int i = 0; i < SLANG_COUNT_OF(numDescriptorByType); ++i)
        {
            int numDescriptors = numDescriptorByType[i];
            if (numDescriptors > 0)
            {
                const VkDescriptorPoolSize poolSize = { VkDescriptorType(i), uint32_t(numDescriptors) };
                poolSizes.Add(poolSize);
            }
        }
        VkDescriptorPoolCreateInfo descriptorPoolInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };

        descriptorPoolInfo.maxSets = 128; // TODO: actually pick a size.
        descriptorPoolInfo.poolSizeCount = uint32_t(poolSizes.Count());
        descriptorPoolInfo.pPoolSizes = poolSizes.Buffer();

        SLANG_VK_CHECK(m_api.vkCreateDescriptorPool(m_device, &descriptorPoolInfo, nullptr, &pipeline->m_descriptorPool));
    }

    // Create the layout
    {
        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
        descriptorSetLayoutInfo.bindingCount = uint32_t(dstBindings.Count());
        descriptorSetLayoutInfo.pBindings = dstBindings.Buffer();

        SLANG_VK_CHECK(m_api.vkCreateDescriptorSetLayout(m_device, &descriptorSetLayoutInfo, nullptr, &pipeline->m_descriptorSetLayout));
    }

    // Create a descriptor set based on our layout
    {
        VkDescriptorSetAllocateInfo descriptorSetAllocInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
        descriptorSetAllocInfo.descriptorPool = pipeline->m_descriptorPool;
        descriptorSetAllocInfo.descriptorSetCount = 1;
        descriptorSetAllocInfo.pSetLayouts = &pipeline->m_descriptorSetLayout;

        SLANG_VK_CHECK(m_api.vkAllocateDescriptorSets(m_device, &descriptorSetAllocInfo, &pipeline->m_descriptorSet));
    }

    // Fill in the descriptor set, using our binding information

    List<VkDescriptorImageInfo> imageInfos;
    List<VkDescriptorBufferInfo> bufferInfos;
    List<VkWriteDescriptorSet> writes;

    // Make sure there is enough space...
    imageInfos.Reserve(numImages);
    bufferInfos.Reserve(numBuffers);

    int elementIndex = 0;

    for (int i = 0; i < numBindings; ++i)
    {
        const auto& srcDetail = srcDetails[i];
        const auto& srcBinding = srcBindings[i];

        const int bindingIndex = srcBinding.registerRange.getSingleIndex();

        VkWriteDescriptorSet writeInfo = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
        writeInfo.descriptorCount = 1;
        writeInfo.dstSet = pipeline->m_descriptorSet;
        writeInfo.dstBinding = bindingIndex;
        writeInfo.dstArrayElement = 0;

        switch (srcBinding.bindingType)
        {
            case BindingType::Buffer:
            {
                assert(srcBinding.resource && srcBinding.resource->isBuffer());
                BufferResourceImpl* bufferResource = static_cast<BufferResourceImpl*>(srcBinding.resource.Ptr());
                const BufferResource::Desc& bufferResourceDesc = bufferResource->getDesc();

                {
                    VkDescriptorBufferInfo bufferInfo;
                    bufferInfo.buffer = bufferResource->m_buffer.m_buffer;
                    bufferInfo.offset = 0;
                    bufferInfo.range = bufferResourceDesc.sizeInBytes;

                    bufferInfos.Add(bufferInfo);
                }

                writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                if (bufferResource->m_initialUsage == Resource::Usage::UnorderedAccess)
                {
                    writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                }
                else if (bufferResource->m_initialUsage == Resource::Usage::ConstantBuffer)
                {
                    writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                }

                writeInfo.pBufferInfo = &bufferInfos.Last();

                writes.Add(writeInfo);
                break;
            }
            case BindingType::Texture:
            {
                assert(srcBinding.resource && srcBinding.resource->isTexture());

                TextureResourceImpl* textureResource = static_cast<TextureResourceImpl*>(srcBinding.resource.Ptr());
                const TextureResource::Desc& textureResourceDesc = textureResource->getDesc();

                {
                    VkDescriptorImageInfo imageInfo = {};
                    imageInfo.imageView = srcDetail.m_srv;
                    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    imageInfos.Add(imageInfo);
                }

                writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
                writeInfo.pImageInfo = &imageInfos.Last();

                writes.Add(writeInfo);
                break;
            }
            case BindingType::Sampler:
            {
                {
                    VkDescriptorImageInfo imageInfo = {};
                    imageInfo.sampler = srcDetail.m_sampler;
                    //imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    imageInfos.Add(imageInfo);
                }

                writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
                writeInfo.pImageInfo = &imageInfos.Last();

                writes.Add(writeInfo);
                break;
            }
            default:
            {
                assert(!"Binding not currently handled");
                return SLANG_FAIL;
            }
        }
    }

    assert(imageInfos.Count() == numImages);
    assert(bufferInfos.Count() == numBuffers);

    // Write into the descriptor set
    {
        m_api.vkUpdateDescriptorSets(m_device, uint32_t(writes.Count()), writes.Buffer(), 0, nullptr);
    }

    // Create a pipeline layout based on our descriptor set layout(s)

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &pipeline->m_descriptorSetLayout;

    SLANG_VK_CHECK(m_api.vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, nullptr, &pipeline->m_pipelineLayout));

    VkPipelineCache pipelineCache = VK_NULL_HANDLE;

    if (m_currentProgram->m_pipelineType == PipelineType::Compute)
    {
        // Then create a pipeline to use that layout

        VkComputePipelineCreateInfo computePipelineInfo = { VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO };
        computePipelineInfo.stage = m_currentProgram->m_compute;
        computePipelineInfo.layout = pipeline->m_pipelineLayout;

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
            vertexInputInfo.pVertexAttributeDescriptions = srcAttributeDescs.Buffer();
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
        pipelineInfo.layout = pipeline->m_pipelineLayout;
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
    if (m_renderPass != VK_NULL_HANDLE)
    {
        m_api.vkDestroyRenderPass(m_device, m_renderPass, nullptr);
        m_renderPass = VK_NULL_HANDLE;
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
    bufferArray.SetSize(bufferSize);
    char* buffer = bufferArray.Buffer();

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

	bufferOut.InsertRange(0, dataBegin, codeSize);

    char* codeBegin = bufferOut.Buffer();

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
    physicalDevices.SetSize(numPhysicalDevices);
    SLANG_VK_RETURN_ON_FAIL(m_api.vkEnumeratePhysicalDevices(instance, &numPhysicalDevices, physicalDevices.Buffer()));

    // TODO: allow override of selected device
    uint32_t selectedDeviceIndex = 0;

    SLANG_RETURN_ON_FAIL(m_api.initPhysicalDevice(physicalDevices[selectedDeviceIndex]));

    int queueFamilyIndex = m_api.findQueue(VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT);
    assert(queueFamilyIndex >= 0);

    float queuePriority = 0.0f;
    VkDeviceQueueCreateInfo queueCreateInfo = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
    queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    char const* const deviceExtensions[] =
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };

    VkDeviceCreateInfo deviceCreateInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
    deviceCreateInfo.pEnabledFeatures = &m_api.m_deviceFeatures;

    deviceCreateInfo.enabledExtensionCount = SLANG_COUNT_OF(deviceExtensions);
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions;

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

TextureResource* VKRenderer::createTextureResource(Resource::Usage initialUsage, const TextureResource::Desc& descIn, const TextureResource::Data* initData)
{
    TextureResource::Desc desc(descIn);
    desc.setDefaults(initialUsage);

    const VkFormat format = VulkanUtil::getVkFormat(desc.format);
    if (format == VK_FORMAT_UNDEFINED)
    {
        assert(!"Unhandled image format");
        return nullptr;
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
                return nullptr;
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

        SLANG_VK_RETURN_NULL_ON_FAIL(m_api.vkCreateImage(m_device, &imageInfo, nullptr, &texture->m_image));
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

        SLANG_VK_RETURN_NULL_ON_FAIL(m_api.vkAllocateMemory(m_device, &allocInfo, nullptr, &texture->m_imageMemory));
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

            mipSizes.Add(mipSize);

            bufferSize += (rowSizeInBytes * numRows) * mipSize.depth;
        }


        // Calculate the total size taking into account the array
        bufferSize *= arraySize;

        Buffer uploadBuffer;
        SLANG_RETURN_NULL_ON_FAIL(uploadBuffer.init(m_api, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));

        assert(mipSizes.Count() == numMipMaps);

        // Copy into upload buffer
        {
            int subResourceIndex = 0;

            uint8_t* dstData;
            m_api.vkMapMemory(m_device, uploadBuffer.m_memory, 0, bufferSize, 0, (void**)&dstData);

            for (int i = 0; i < arraySize; ++i)
            {
                for (int j = 0; j < int(mipSizes.Count()); ++j)
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
                for (int j = 0; j < int(mipSizes.Count()); ++j)
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
                    region.imageSubresource.mipLevel = j;
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

    return texture.detach();
}

BufferResource* VKRenderer::createBufferResource(Resource::Usage initialUsage, const BufferResource::Desc& descIn, const void* initData)
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
    SLANG_RETURN_NULL_ON_FAIL(buffer->m_buffer.init(m_api, desc.sizeInBytes, usage, reqMemoryProperties));

    if ((desc.cpuAccessFlags & Resource::AccessFlag::Write) || initData)
    {
        SLANG_RETURN_NULL_ON_FAIL(buffer->m_uploadBuffer.init(m_api, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
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

    return buffer.detach();
}

InputLayout* VKRenderer::createInputLayout(const InputElementDesc* elements, UInt numElements)
{
    RefPtr<InputLayoutImpl> layout(new InputLayoutImpl);

    List<VkVertexInputAttributeDescription>& dstVertexDescs = layout->m_vertexDescs;

    size_t vertexSize = 0;
    dstVertexDescs.SetSize(numElements);

    for (UInt i = 0; i <  numElements; ++i)
    {
        const InputElementDesc& srcDesc = elements[i];
        VkVertexInputAttributeDescription& dstDesc = dstVertexDescs[i];

        dstDesc.location = uint32_t(i);
        dstDesc.binding = 0;
        dstDesc.format = VulkanUtil::getVkFormat(srcDesc.format);
        if (dstDesc.format == VK_FORMAT_UNDEFINED)
        {
            return nullptr;
        }

        dstDesc.offset = uint32_t(srcDesc.offset);

        const size_t elementSize = RendererUtil::getFormatSize(srcDesc.format);
        assert(elementSize > 0);
        const size_t endElement = srcDesc.offset + elementSize;

        vertexSize = (vertexSize < endElement) ? endElement : vertexSize;
    }

    // Work out the overall size
    layout->m_vertexSize = int(vertexSize);
    return layout.detach();
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
            buffer->m_readBuffer.SetSize(bufferSize);

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

            ::memcpy(buffer->m_readBuffer.Buffer(), mappedData, bufferSize);
            m_api.vkUnmapMemory(m_device, staging.m_memory);

            buffer->m_mapFlavor = flavor;

            return buffer->m_readBuffer.Buffer();
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

void VKRenderer::setInputLayout(InputLayout* inputLayout)
{
    m_currentInputLayout = static_cast<InputLayoutImpl*>(inputLayout);
}

void VKRenderer::setPrimitiveTopology(PrimitiveTopology topology)
{
    m_primitiveTopology = VulkanUtil::getVkPrimitiveTopology(topology);
}

void VKRenderer::setVertexBuffers(UInt startSlot, UInt slotCount, BufferResource*const* buffers, const UInt* strides, const UInt* offsets)
{
    {
        const UInt num = startSlot + slotCount;
        if (num > m_boundVertexBuffers.Count())
        {
            m_boundVertexBuffers.SetSize(num);
        }
    }

    for (UInt i = 0; i < slotCount; i++)
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

void VKRenderer::setShaderProgram(ShaderProgram* program)
{
    m_currentProgram = (ShaderProgramImpl*)program;
}

void VKRenderer::draw(UInt vertexCount, UInt startVertex = 0)
{
    Pipeline* pipeline = _getPipeline();
    if (!pipeline || pipeline->m_shaderProgram->m_pipelineType != PipelineType::Graphics)
    {
        assert(!"Invalid render pipeline");
        return;
    }

    SLANG_RETURN_VOID_ON_FAIL(_beginPass());

    // Also create descriptor sets based on the given pipeline layout
    VkCommandBuffer commandBuffer = m_deviceQueue.getCommandBuffer();

    m_api.vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->m_pipeline);
    m_api.vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->m_pipelineLayout,
        0, 1, &pipeline->m_descriptorSet, 0, nullptr);

    // Bind the vertex buffer
    if (m_boundVertexBuffers.Count() > 0 && m_boundVertexBuffers[0].m_buffer)
    {
        const BoundVertexBuffer& boundVertexBuffer = m_boundVertexBuffers[0];

        VkBuffer vertexBuffers[] = { boundVertexBuffer.m_buffer->m_buffer.m_buffer };
        VkDeviceSize offsets[] = { VkDeviceSize(boundVertexBuffer.m_offset) };

        m_api.vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    }

    m_api.vkCmdDraw(commandBuffer, static_cast<uint32_t>(vertexCount), 1, 0, 0);

    _endPass();
}

void VKRenderer::dispatchCompute(int x, int y, int z)
{
    Pipeline* pipeline = _getPipeline();
    if (!pipeline || pipeline->m_shaderProgram->m_pipelineType != PipelineType::Compute)
    {
        assert(!"Invalid render pipeline");
        return;
    }

    // Also create descriptor sets based on the given pipeline layout
    VkCommandBuffer commandBuffer = m_deviceQueue.getCommandBuffer();

    m_api.vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->m_pipeline);

    m_api.vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->m_pipelineLayout,
        0, 1, &pipeline->m_descriptorSet, 0, nullptr);

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

void VKRenderer::setBindingState(BindingState* state)
{
    m_currentBindingState = static_cast<BindingStateImpl*>(state);
}

ShaderProgram* VKRenderer::createProgram(const ShaderProgram::Desc& desc)
{
    ShaderProgramImpl* impl = new ShaderProgramImpl(desc.pipelineType);
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
    return impl;
}

} // renderer_test
