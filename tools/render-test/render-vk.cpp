// render-vk.cpp
#include "render-vk.h"

#include "options.h"
#include "render.h"

#include "../../source/core/smart-pointer.h"
#include "slang-support.h"

#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR 1
#endif

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

#define ENABLE_VALIDATION_LAYER 1


#ifdef _MSC_VER

#include <stddef.h>

#pragma warning(disable: 4996)

#if (_MSC_VER < 1900)
#define snprintf sprintf_s
#endif
#endif

#define FOREACH_GLOBAL_PROC(M)          \
    M(vkCreateInstance)                 \
    /* */

#define FOREACH_INSTANCE_PROC(M)                \
    M(vkCreateDevice)                           \
    M(vkCreateDebugReportCallbackEXT)           \
    M(vkDestroyDebugReportCallbackEXT)          \
    M(vkDebugReportMessageEXT)                  \
    M(vkEnumeratePhysicalDevices)               \
    M(vkGetPhysicalDeviceProperties)            \
    M(vkGetPhysicalDeviceFeatures)              \
    M(vkGetPhysicalDeviceMemoryProperties)      \
    M(vkGetPhysicalDeviceQueueFamilyProperties) \
    M(vkGetDeviceProcAddr)                      \
    /* */

#define FOREACH_DEVICE_PROC(M)          \
    M(vkCreateDescriptorPool)           \
    M(vkCreateCommandPool)              \
    M(vkGetDeviceQueue)                 \
    M(vkAllocateCommandBuffers)         \
    M(vkBeginCommandBuffer)             \
    M(vkEndCommandBuffer)               \
    M(vkQueueSubmit)                    \
    M(vkQueueWaitIdle)                  \
    M(vkFreeCommandBuffers)             \
    M(vkCreateBuffer)                   \
    M(vkGetBufferMemoryRequirements)    \
    M(vkAllocateMemory)                 \
    M(vkBindBufferMemory)               \
    M(vkMapMemory)                      \
    M(vkUnmapMemory)                    \
    M(vkCmdCopyBuffer)                  \
    M(vkDestroyBuffer)                  \
    M(vkFreeMemory)                     \
    M(vkCreateDescriptorSetLayout)      \
    M(vkAllocateDescriptorSets)         \
    M(vkUpdateDescriptorSets)           \
    M(vkCreatePipelineLayout)           \
    M(vkCreateComputePipelines)         \
    M(vkCmdBindPipeline)                \
    M(vkCmdBindDescriptorSets)          \
    M(vkCmdDispatch)                    \
    M(vkDestroyPipeline)                \
    M(vkCreateShaderModule)             \
    /* */


namespace renderer_test {
using namespace Slang;

#define RETURN_ON_VK_FAIL(x) { VkResult _vkRes = x; if (_vkRes != VK_SUCCESS) { SLANG_RETURN_ON_FAIL(toSlangResult(_vkRes)); }}

class VKRenderer : public Renderer, public ShaderCompiler
{
public:
    // Renderer    implementation
    virtual SlangResult initialize(void* inWindowHandle) override;
    virtual void setClearColor(const float color[4]) override;
    virtual void clearFrame() override;
    virtual void presentFrame() override;
    virtual TextureResource* createTextureResource(Resource::Type type, Resource::Usage initialUsage, const TextureResource::Desc& desc, const TextureResource::Data* initData) override;
    virtual BufferResource* createBufferResource(Resource::Usage initialUsage, const BufferResource::Desc& bufferDesc, const void* initData) override;
    virtual SlangResult captureScreenShot(const char* outputPath) override;
    virtual void serializeOutput(BindingState* state, const char * fileName) override;
    virtual InputLayout* createInputLayout(const InputElementDesc* inputElements, UInt inputElementCount) override;
    virtual BindingState* createBindingState(const ShaderInputLayout& layout) override;
    virtual BindingState* createBindingState(const BindingState::Desc& bindingStateDesc) override;
    virtual ShaderCompiler* getShaderCompiler() override;
    virtual void* map(BufferResource* buffer, MapFlavor flavor) override;
    virtual void unmap(BufferResource* buffer) override;
    virtual void setInputLayout(InputLayout* inputLayout) override;
    virtual void setPrimitiveTopology(PrimitiveTopology topology) override;
    virtual void setBindingState(BindingState* state);
    virtual void setVertexBuffers(UInt startSlot, UInt slotCount, BufferResource*const* buffers, const UInt* strides, const UInt* offsets) override;
    virtual void setShaderProgram(ShaderProgram* inProgram) override;
    virtual void setConstantBuffers(UInt startSlot, UInt slotCount, BufferResource*const* buffers,  const UInt* offsets) override;
    virtual void draw(UInt vertexCount, UInt startVertex) override;
    virtual void dispatchCompute(int x, int y, int z) override;
    virtual void submitGpuWork() override {}
    virtual void waitForGpu() override {}

    // ShaderCompiler implementation
    virtual ShaderProgram* compileProgram(const ShaderCompileRequest& request) override;

    protected:

    class Buffer
    {
        public:
        
            /// Returns true if has been initialized
        bool isInitialized() const { return m_renderer != nullptr; }
        
            // Default Ctor
        Buffer():
            m_renderer(nullptr)
        {}

            /// Dtor
        ~Buffer()
        {
            if (m_renderer)
            {
                m_renderer->vkDestroyBuffer(m_renderer->m_device, m_buffer, nullptr);
                m_renderer->vkFreeMemory(m_renderer->m_device, m_memory, nullptr);
            }
        }

        VkBuffer m_buffer;
        VkDeviceMemory m_memory;
        VKRenderer* m_renderer;
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
    };

	class ShaderProgramImpl: public ShaderProgram
    {
		public:

        VkPipelineShaderStageCreateInfo m_compute;
        VkPipelineShaderStageCreateInfo m_vertex;
        VkPipelineShaderStageCreateInfo m_fragment;
		
		List<char> m_buffers[2];								//< To keep storage of code in scope	
    };

    struct Binding
    {
        BindingType    bindingType;
        
        VkImageView     srv;
        VkBufferView    uav;
        VkSampler       samplerState;

        RefPtr<Resource> resource;                              ///< Holds either the BufferResource, or TextureResource

        int binding = 0;
        bool isOutput = false;
    };

    class BindingStateImpl: public BindingState
    {
		public:
		BindingStateImpl(VKRenderer* renderer):
			m_renderer(renderer),
			m_numRenderTargets(0)
		{
		}

		VKRenderer* m_renderer;
        List<Binding> m_bindings;
        int m_numRenderTargets;
    };
    
	class InputLayoutImpl: public InputLayout
    {
		public:
    };

    VkBool32 handleDebugMessage(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t srcObject,
        size_t location, int32_t msgCode, const char* pLayerPrefix, const char* pMsg);
    
    VkCommandBuffer getCommandBuffer();
    VkCommandBuffer beginCommandBuffer();
    void flushCommandBuffer(VkCommandBuffer commandBuffer);
    
    uint32_t getMemoryTypeIndex(uint32_t inTypeBits, VkMemoryPropertyFlags properties);

    VkPipelineShaderStageCreateInfo compileEntryPoint(const ShaderCompileRequest::EntryPoint& entryPointRequest, VkShaderStageFlagBits stage, List<char>& bufferOut);

    void createInputTexture(const InputTextureDesc& inputDesc, VkImageView& viewOut);
    void createInputSampler(const InputSamplerDesc& inputDesc, VkSampler& stateOut);
    SlangResult createInputBuffer(const ShaderInputLayoutEntry& entry, const InputBufferDesc& bufferDesc, const Slang::List<unsigned int>& bufferData,
        RefPtr<BufferResource>& bufferOut, VkBufferView& uavOut, VkImageView& srvOut);

    SlangResult _initBuffer(size_t bufferSize, VkBufferUsageFlags usage, VkMemoryPropertyFlags reqMemoryProperties, Buffer& bufferOut);

    static SlangResult toSlangResult(VkResult res);
    static void checkResult(VkResult result);
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugMessageCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t srcObject,
        size_t location, int32_t msgCode, const char* pLayerPrefix, const char* pMsg, void* pUserData);

    VkInstance                          m_instance;
    VkPhysicalDevice                    m_physicalDevice;
    VkPhysicalDeviceProperties          m_deviceProperties;
    VkPhysicalDeviceFeatures            m_deviceFeatures;
    VkPhysicalDeviceMemoryProperties    m_deviceMemoryProperties;
    VkDevice                            m_device;
    VkQueue                             m_queue;
    VkCommandPool                       m_commandPool;
    VkSubmitInfo                        m_submitInfo;
    VkDebugReportCallbackEXT            m_debugReportCallback;

    RefPtr<BindingStateImpl> m_currentBindingState;
    RefPtr<ShaderProgramImpl> m_currentProgram;

    float m_clearColor[4] = { 0, 0, 0, 0 };;

#define DECLARE_PROC(NAME) PFN_##NAME NAME;
    DECLARE_PROC(vkGetInstanceProcAddr);
    FOREACH_GLOBAL_PROC(DECLARE_PROC)
    FOREACH_INSTANCE_PROC(DECLARE_PROC)
    FOREACH_DEVICE_PROC(DECLARE_PROC)
#undef DECLARE_PROC
    
};

Renderer* createVKRenderer()
{
    return new VKRenderer;
}

/* static */SlangResult VKRenderer::toSlangResult(VkResult res)
{
    return (res == VK_SUCCESS) ? SLANG_OK : SLANG_FAIL;
}

void VKRenderer::checkResult(VkResult result)
{
    assert(result == VK_SUCCESS);
}

VkBool32 VKRenderer::handleDebugMessage(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t srcObject,
    size_t location, int32_t msgCode, const char* pLayerPrefix, const char* pMsg)
{
    char const* severity = "message";
    if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
        severity = "warning";
    if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
        severity = "error";

    char buffer[1024];
    sprintf_s(buffer,
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

VkCommandBuffer VKRenderer::getCommandBuffer()
{
    VkCommandBufferAllocateInfo info = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
    info.commandPool = m_commandPool;
    info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    info.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    checkResult(vkAllocateCommandBuffers(m_device, &info, &commandBuffer));

    return commandBuffer;
}

VkCommandBuffer VKRenderer::beginCommandBuffer()
{
    VkCommandBuffer commandBuffer = getCommandBuffer();

    VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    checkResult(vkBeginCommandBuffer(commandBuffer, &beginInfo));

    return commandBuffer;
}

void VKRenderer::flushCommandBuffer(VkCommandBuffer commandBuffer)
{
    checkResult(vkEndCommandBuffer(commandBuffer));

    VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    checkResult(vkQueueSubmit(m_queue, 1, &submitInfo, VK_NULL_HANDLE));
    checkResult(vkQueueWaitIdle(m_queue));

    vkFreeCommandBuffers(m_device, m_commandPool, 1, &commandBuffer);
}

SlangResult VKRenderer::_initBuffer(size_t bufferSize, VkBufferUsageFlags usage, VkMemoryPropertyFlags reqMemoryProperties, Buffer& bufferOut)
{
    assert(!bufferOut.isInitialized());

    bufferOut.m_renderer = this;
    bufferOut.m_memory = nullptr;
    bufferOut.m_buffer = nullptr;

    VkBufferCreateInfo bufferCreateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bufferCreateInfo.size = bufferSize;
    bufferCreateInfo.usage = usage;

    checkResult(vkCreateBuffer(m_device, &bufferCreateInfo, nullptr, &bufferOut.m_buffer));

    VkMemoryRequirements memoryReqs = {};
    vkGetBufferMemoryRequirements(m_device, bufferOut.m_buffer, &memoryReqs);

    uint32_t memoryTypeIndex = getMemoryTypeIndex(memoryReqs.memoryTypeBits, reqMemoryProperties);

    VkMemoryPropertyFlags actualMemoryProperites = m_deviceMemoryProperties.memoryTypes[memoryTypeIndex].propertyFlags;

    VkMemoryAllocateInfo allocateInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
    allocateInfo.allocationSize = memoryReqs.size;
    allocateInfo.memoryTypeIndex = memoryTypeIndex;

    checkResult(vkAllocateMemory(m_device, &allocateInfo, nullptr, &bufferOut.m_memory));
    checkResult(vkBindBufferMemory(m_device, bufferOut.m_buffer, bufferOut.m_memory, 0));

    return SLANG_OK;
}

uint32_t VKRenderer::getMemoryTypeIndex(uint32_t inTypeBits, VkMemoryPropertyFlags properties)
{
    uint32_t typeBits = inTypeBits;
    uint32_t typeIndex = 0;
    while (typeBits)
    {
        if ((m_deviceMemoryProperties.memoryTypes[typeIndex].propertyFlags & properties) == properties)
        {
            return typeIndex;
        }
        typeIndex++;
        typeBits >>= 1;
    }

    assert(!"failed to find a usable memory type");
    return uint32_t(-1);
}

void VKRenderer::createInputTexture(const InputTextureDesc& inputDesc, VkImageView& viewOut)
{
    TextureData texData;
    generateTextureData(texData, inputDesc);
    assert(!"unimplemented");
}

void VKRenderer::createInputSampler(const InputSamplerDesc& inputDesc, VkSampler& stateOut)
{
    assert(!"unimplemented");
}

SlangResult VKRenderer::createInputBuffer(const ShaderInputLayoutEntry& entry, const InputBufferDesc& bufferDesc, const Slang::List<unsigned int>& bufferData, 
    RefPtr<BufferResource>& bufferOut, VkBufferView& uavOut, VkImageView& srvOut)
{
    size_t bufferSize = bufferData.Count() * sizeof(unsigned int);

    RefPtr<BufferResource> bufferResource;
    SLANG_RETURN_ON_FAIL(createInputBufferResource(bufferDesc, entry.isOutput, bufferSize, bufferData.Buffer(), this, bufferResource));

    BufferResourceImpl* resourceImpl = static_cast<BufferResourceImpl*>(bufferResource.Ptr());

    // TODO: need to hang onto the `memory` field so
    // that we can release it when we are done.
	
    
    // Fill in any views needed
    switch (bufferDesc.type)
    {
        case InputBufferType::ConstantBuffer:
            break;

        case InputBufferType::StorageBuffer:
        {
        }
        break;
    }

    bufferOut = bufferResource;
    return SLANG_OK;
}

VkPipelineShaderStageCreateInfo VKRenderer::compileEntryPoint(const ShaderCompileRequest::EntryPoint& entryPointRequest, VkShaderStageFlagBits stage, List<char>& bufferOut)
{
    char const* dataBegin = entryPointRequest.source.dataBegin;
    char const* dataEnd = entryPointRequest.source.dataEnd;

    // We need to make a copy of the code, since the Slang compiler
    // will free the memory after a compile request is closed.
    size_t codeSize = dataEnd - dataBegin;

	bufferOut.InsertRange(0, dataBegin, codeSize);

    char* codeBegin = bufferOut.Buffer();
	
    VkShaderModuleCreateInfo moduleCreateInfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
    moduleCreateInfo.pCode = (uint32_t*)codeBegin;
    moduleCreateInfo.codeSize = codeSize;

    VkShaderModule module;
    checkResult(vkCreateShaderModule(m_device, &moduleCreateInfo, nullptr, &module));

    //::free(codeBegin);

    VkPipelineShaderStageCreateInfo shaderStageCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
    shaderStageCreateInfo.stage = stage;

    shaderStageCreateInfo.module = module;
    shaderStageCreateInfo.pName = "main";
    return shaderStageCreateInfo;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!! Renderer interface !!!!!!!!!!!!!!!!!!!!!!!!!!

SlangResult VKRenderer::initialize(void* inWindowHandle)
{
    char const* dynamicLibraryName = "vulkan-1.dll";
    HMODULE vulkan = LoadLibraryA(dynamicLibraryName);
    if (!vulkan)
    {
        fprintf(stderr, "error: failed load '%s'\n", dynamicLibraryName);
        return SLANG_FAIL;
    }

    vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)GetProcAddress(vulkan, "vkGetInstanceProcAddr");
    if (!vkGetInstanceProcAddr)
    {
        fprintf(stderr, "error: failed load symbol 'vkGetInstanceProcAddr'\n");
        return SLANG_FAIL;
    }

    VkApplicationInfo applicationInfo = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
    applicationInfo.pApplicationName = "slang-render-test";
    applicationInfo.pEngineName = "slang-render-test";
    applicationInfo.apiVersion = VK_API_VERSION_1_0;

    char const* instanceExtensions[] = {
        VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef _WIN32
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#else
#endif

#if ENABLE_VALIDATION_LAYER
        VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
#endif
    };

    VkInstanceCreateInfo instanceCreateInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
    instanceCreateInfo.pApplicationInfo = &applicationInfo;

    instanceCreateInfo.enabledExtensionCount = sizeof(instanceExtensions) / sizeof(instanceExtensions[0]);
    instanceCreateInfo.ppEnabledExtensionNames = &instanceExtensions[0];

#if ENABLE_VALIDATION_LAYER

    uint32_t layerCount = 1;
    const char *layerNames[] = {
        "VK_LAYER_LUNARG_standard_validation"
    };

    instanceCreateInfo.enabledLayerCount = layerCount;
    instanceCreateInfo.ppEnabledLayerNames = layerNames;
#endif

    m_instance = 0;

#define LOAD_INSTANCE_PROC(NAME) NAME = (PFN_##NAME) vkGetInstanceProcAddr(m_instance, #NAME);

    FOREACH_GLOBAL_PROC(LOAD_INSTANCE_PROC);

    RETURN_ON_VK_FAIL(vkCreateInstance(&instanceCreateInfo, nullptr, &m_instance));

    FOREACH_INSTANCE_PROC(LOAD_INSTANCE_PROC);

#undef LOAD_INSTANCE_PROC


#if ENABLE_VALIDATION_LAYER
    VkDebugReportFlagsEXT debugFlags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;

    VkDebugReportCallbackCreateInfoEXT debugCreateInfo = { VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT };
    debugCreateInfo.pfnCallback = &debugMessageCallback;
    debugCreateInfo.pUserData = this;
    debugCreateInfo.flags = debugFlags;

    RETURN_ON_VK_FAIL(vkCreateDebugReportCallbackEXT(m_instance, &debugCreateInfo, nullptr, &m_debugReportCallback));

#endif

    uint32_t physicalDeviceCount = 0;
    RETURN_ON_VK_FAIL(vkEnumeratePhysicalDevices(m_instance, &physicalDeviceCount, nullptr));

    VkPhysicalDevice* physicalDevices = (VkPhysicalDevice*)alloca(physicalDeviceCount * sizeof(VkPhysicalDevice));
    RETURN_ON_VK_FAIL(vkEnumeratePhysicalDevices(m_instance, &physicalDeviceCount, physicalDevices));

    uint32_t selectedDeviceIndex = 0;
    // TODO: allow override of selected device
    m_physicalDevice = physicalDevices[selectedDeviceIndex];

    vkGetPhysicalDeviceProperties(m_physicalDevice, &m_deviceProperties);
    vkGetPhysicalDeviceFeatures(m_physicalDevice, &m_deviceFeatures);
    vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &m_deviceMemoryProperties);

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, nullptr);

    VkQueueFamilyProperties* queueFamilies = (VkQueueFamilyProperties*)alloca(queueFamilyCount * sizeof(VkQueueFamilyProperties));
    vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, queueFamilies);

    // Find a queue that can service our needs
    VkQueueFlags reqQueueFlags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT;

    uint32_t queueFamilyIndex = uint32_t(-1);
    for (uint32_t qq = 0; qq < queueFamilyCount; ++qq)
    {
        if ((queueFamilies[qq].queueFlags & reqQueueFlags) == reqQueueFlags)
        {
            queueFamilyIndex = qq;
            break;
        }
    }
    assert(queueFamilyIndex < queueFamilyCount);

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
    deviceCreateInfo.pEnabledFeatures = &m_deviceFeatures;

    deviceCreateInfo.enabledExtensionCount = sizeof(deviceExtensions) / sizeof(deviceExtensions[0]);
    deviceCreateInfo.ppEnabledExtensionNames = &deviceExtensions[0];

    RETURN_ON_VK_FAIL(vkCreateDevice(m_physicalDevice, &deviceCreateInfo, nullptr, &m_device));

#define LOAD_DEVICE_PROC(NAME) NAME = (PFN_##NAME) vkGetDeviceProcAddr(m_device, #NAME);
    FOREACH_DEVICE_PROC(LOAD_DEVICE_PROC)
#undef LOAD_DEVICE_PROC

        // Create a command pool

    VkCommandPoolCreateInfo commandPoolCreateInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
    commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndex;
    commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    RETURN_ON_VK_FAIL(vkCreateCommandPool(m_device, &commandPoolCreateInfo, nullptr, &m_commandPool));

    vkGetDeviceQueue(m_device, queueFamilyIndex, 0, &m_queue);

    // set up swap chain



    // create command buffers

    // depth/stencil?

    // render pass?

    // pipeline cache

    // frame buffer



    // create semaphores for sync

    return SLANG_OK;
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
}

SlangResult VKRenderer::captureScreenShot(char const* outputPath)
{
    return SLANG_FAIL;
}

ShaderCompiler* VKRenderer::getShaderCompiler() 
{
    return this;
}

TextureResource* VKRenderer::createTextureResource(Resource::Type type, Resource::Usage initialUsage, const TextureResource::Desc& desc, const TextureResource::Data* initData)
{
    return nullptr;
}

static VkBufferUsageFlagBits _calcUsageFlagBit(Resource::BindFlag::Enum bind)
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
        case BindFlag::UnorderedAccess:         return VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        case BindFlag::PixelShaderResource:     return VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        case BindFlag::NonPixelShaderResource:  return VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        default:                                return VkBufferUsageFlagBits(0);
    }
}

static VkBufferUsageFlagBits _calcUsageFlagBit(int bindFlags)
{
    int dstFlags = 0;
    while (bindFlags)
    {
        int lsb = bindFlags & -bindFlags;
        dstFlags |= _calcUsageFlagBit(Resource::BindFlag::Enum(lsb));
        bindFlags &= ~lsb;
    }
    return VkBufferUsageFlagBits(dstFlags);
}

BufferResource* VKRenderer::createBufferResource(Resource::Usage initialUsage, const BufferResource::Desc& descIn, const void* initData)
{
    BufferResource::Desc desc(descIn);
    if (desc.bindFlags == 0)
    {
        desc.bindFlags = Resource::s_requiredBinding[int(initialUsage)];
    }

    const size_t bufferSize = desc.sizeInBytes;

    VkBufferUsageFlags usage = _calcUsageFlagBit(desc.bindFlags);
    VkMemoryPropertyFlags reqMemoryProperties = 0;

    if (desc.cpuAccessFlags & Resource::AccessFlag::Read)
    {
        // If it can be read from, set this
        usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    }
    if ((desc.cpuAccessFlags & Resource::AccessFlag::Write) || initData)
    {
        usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    }

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
    SLANG_RETURN_NULL_ON_FAIL(_initBuffer(desc.sizeInBytes, usage, reqMemoryProperties, buffer->m_buffer));

    if ((desc.cpuAccessFlags & Resource::AccessFlag::Write) || initData)
    {
        SLANG_RETURN_NULL_ON_FAIL(_initBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, buffer->m_uploadBuffer));
    }

    if (initData)
    {
        // TODO: only create staging buffer if the memory type
        // used for the buffer doesn't let us fill things in
        // directly.
        // Copy into staging buffer
        void* mappedData = nullptr;
        checkResult(vkMapMemory(m_device, buffer->m_uploadBuffer.m_memory, 0, bufferSize, 0, &mappedData));
        memcpy(mappedData, initData, bufferSize);
        vkUnmapMemory(m_device, buffer->m_uploadBuffer.m_memory);

        // Copy from staging buffer to real buffer
        VkCommandBuffer commandBuffer = beginCommandBuffer();

        VkBufferCopy copyInfo = {};
        copyInfo.size = bufferSize;
        vkCmdCopyBuffer(commandBuffer, buffer->m_uploadBuffer.m_buffer, buffer->m_buffer.m_buffer, 1, &copyInfo);

        flushCommandBuffer(commandBuffer);
    }

    return buffer.detach();
}

InputLayout* VKRenderer::createInputLayout(const InputElementDesc* inputElements, UInt inputElementCount) 
{
    InputLayoutImpl* impl = new InputLayoutImpl;

    // TODO: actually initialize things

    return (InputLayout*)impl;
}

void* VKRenderer::map(BufferResource* buffer, MapFlavor flavor)
{
    return nullptr;
}

void VKRenderer::unmap(BufferResource* buffer)
{
}

void VKRenderer::setInputLayout(InputLayout* inputLayout)
{
}

void VKRenderer::setPrimitiveTopology(PrimitiveTopology topology)
{
}

void VKRenderer::setVertexBuffers(UInt startSlot, UInt slotCount, BufferResource*const* buffers, const UInt* strides, const UInt* offsets)
{
}

void VKRenderer::setShaderProgram(ShaderProgram* program)
{
    m_currentProgram = (ShaderProgramImpl*)program;
}

void VKRenderer::setConstantBuffers(UInt startSlot, UInt slotCount, BufferResource*const* buffers, const UInt* offsets) 
{
}

void VKRenderer::draw(UInt vertexCount, UInt startVertex = 0)
{
}

static BindingType _getBindingType(ShaderInputType type)
{
    switch (type)
    {
        case ShaderInputType::Buffer:                   return BindingType::Buffer;
        case ShaderInputType::Texture:                  return BindingType::Texture;
        case ShaderInputType::CombinedTextureSampler:   return BindingType::CombinedTextureSampler;
        case ShaderInputType::Sampler:                  return BindingType::Sampler;
        default:                                        return BindingType::Unknown;
    }
}

BindingState* VKRenderer::createBindingState(const ShaderInputLayout& layout)
{
    BindingStateImpl* bindingState = new BindingStateImpl(this);
    bindingState->m_numRenderTargets = layout.numRenderTargets;
    for (auto & entry : layout.entries)
    {
        Binding binding;
        
        binding.bindingType = _getBindingType(entry.type);

        binding.binding = entry.hlslBinding;
        binding.isOutput = entry.isOutput;
        switch (entry.type)
        {
            case ShaderInputType::Buffer:
            {
                RefPtr<BufferResource> bufferResource;

                SLANG_RETURN_NULL_ON_FAIL(createInputBuffer(entry, entry.bufferDesc, entry.bufferData, bufferResource, binding.uav, binding.srv));

                binding.resource = bufferResource;

                //binding.bufferType = entry.bufferDesc.type;
            
                break;
            }
            case ShaderInputType::Texture:
            case ShaderInputType::Sampler:
            case ShaderInputType::CombinedTextureSampler:
            {
                assert(!"not implemented");
                break;
            }
        }
        bindingState->m_bindings.Add(binding);
    }

    return bindingState;
}

BindingState* VKRenderer::createBindingState(const BindingState::Desc& bindingStateDesc)
{   
    RefPtr<BindingStateImpl> bindingState(new BindingStateImpl(this));

    bindingState->m_numRenderTargets = bindingStateDesc.m_numRenderTargets;

    const List<BindingState::Desc::Binding>& srcBindings = bindingStateDesc.m_bindings;
    const int numBindings = int(srcBindings.Count());

    List<Binding>& dstBindings = bindingState->m_bindings;
    dstBindings.SetSize(numBindings);

    for (int i = 0; i < numBindings; ++i)
    {
        Binding& dstBinding = dstBindings[i];
        const BindingState::Desc::Binding& srcBinding = srcBindings[i];

        dstBinding.bindingType = srcBinding.bindingType;
        
        //! TODO: Is this correct?! It's using the hlsl binding
        dstBinding.binding = bindingStateDesc.getFirst(BindingState::ShaderStyle::Hlsl, srcBinding.registerDesc);
        
        switch (srcBinding.bindingType)
        {
            case BindingType::Buffer:
            {
                assert(srcBinding.resource && srcBinding.resource->isBuffer());
                BufferResourceImpl* bufferResource = static_cast<BufferResourceImpl*>(srcBinding.resource.Ptr());

                const BufferResource::Desc& bufferResourceDesc = bufferResource->getDesc();

                dstBinding.isOutput = (bufferResourceDesc.cpuAccessFlags & Resource::AccessFlag::Read) != 0;

                // TODO: Setup views. 
                // VkBufferView uav
                // VkImageView srv

                dstBinding.resource = bufferResource;
                break;
            }
            case BindingType::Texture:
            case BindingType::Sampler:
            case BindingType::CombinedTextureSampler:
            {
                assert(!"not implemented");
            }
        }
    }

    return bindingState.detach();;
}

void VKRenderer::setBindingState(BindingState* state)
{
    m_currentBindingState = static_cast<BindingStateImpl*>(state);
}

void VKRenderer::serializeOutput(BindingState* s, const char* fileName)
{
    auto state = static_cast<BindingStateImpl*>(s);

    FILE * f = fopen(fileName, "wb");
    int id = 0;
    for (auto& bb : state->m_bindings)
    {
        if (bb.isOutput)
        {
            if (bb.resource && bb.resource->isBuffer())
            {
                BufferResourceImpl* bufferResource = static_cast<BufferResourceImpl*>(bb.resource.Ptr());
                const BufferResource::Desc& bufferResourceDesc = bufferResource->getDesc();

                size_t bufferSize = bufferResourceDesc.sizeInBytes;
                                
                // create staging buffer
                Buffer staging;
                
                SLANG_RETURN_VOID_ON_FAIL(_initBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging));

                // Copy from real buffer to staging buffer
                VkCommandBuffer commandBuffer = beginCommandBuffer();

                VkBufferCopy copyInfo = {};
                copyInfo.size = bufferSize;
                vkCmdCopyBuffer(commandBuffer, bufferResource->m_buffer.m_buffer, staging.m_buffer, 1, &copyInfo);

                flushCommandBuffer(commandBuffer);

                // Write out the data from the buffer
                void* mappedData = nullptr;
                checkResult(vkMapMemory(m_device, staging.m_memory, 0, bufferSize, 0, &mappedData));

                auto ptr = (unsigned int *)mappedData;
                for (auto i = 0u; i < bufferSize / sizeof(unsigned int); i++)
                    fprintf(f, "%X\n", ptr[i]);

                vkUnmapMemory(m_device, staging.m_memory);
            }
            else
            {
                printf("invalid output type at %d.\n", id);
            }
        }
        id++;
    }
    fclose(f);
}

void VKRenderer::dispatchCompute(int x, int y, int z)
{
    // HACK: create a new pipeline for every call

    // First create a pipeline layout based on what is bound

    Slang::List<VkDescriptorSetLayoutBinding> bindings;

    for (auto bb : m_currentBindingState->m_bindings)
    {
        switch (bb.bindingType)
        {
            case BindingType::Buffer:
            {
                BufferResourceImpl* bufferResource = static_cast<BufferResourceImpl*>(bb.resource.Ptr());
                const BufferResource::Desc& bufferResourceDesc = bufferResource->getDesc();

                if (bufferResourceDesc.bindFlags & Resource::BindFlag::UnorderedAccess)
                {
                    VkDescriptorSetLayoutBinding binding = {};
                    binding.binding = bb.binding;
                    binding.descriptorCount = 1;
                    binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    binding.stageFlags = VK_SHADER_STAGE_ALL;

                    bindings.Add(binding);
                }    
            
                break;
            }
            default:
                // TODO: handle the other cases
                break;
        }
    }

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
    descriptorSetLayoutInfo.bindingCount = uint32_t(bindings.Count());
    descriptorSetLayoutInfo.pBindings = bindings.Buffer();

    VkDescriptorSetLayout descriptorSetLayout = 0;
    checkResult(vkCreateDescriptorSetLayout(m_device, &descriptorSetLayoutInfo, nullptr, &descriptorSetLayout));

    // Create a descriptor pool for allocating sets

    VkDescriptorPoolSize poolSizes[] =
    {
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 128 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 128 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 128 },
    };

    VkDescriptorPoolCreateInfo descriptorPoolInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
    descriptorPoolInfo.maxSets = 128; // TODO: actually pick a size
    descriptorPoolInfo.poolSizeCount = sizeof(poolSizes) / sizeof(poolSizes[0]);
    descriptorPoolInfo.pPoolSizes = poolSizes;

    VkDescriptorPool descriptorPool;
    checkResult(vkCreateDescriptorPool(m_device, &descriptorPoolInfo, nullptr, &descriptorPool));

    // Create a descriptor set based on our layout

    VkDescriptorSetAllocateInfo descriptorSetAllocInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
    descriptorSetAllocInfo.descriptorPool = descriptorPool;
    descriptorSetAllocInfo.descriptorSetCount = 1;
    descriptorSetAllocInfo.pSetLayouts = &descriptorSetLayout;

    VkDescriptorSet descriptorSet;
    checkResult(vkAllocateDescriptorSets(m_device, &descriptorSetAllocInfo, &descriptorSet));

    // Fill in the descriptor set, using our binding information
    for (auto bb : m_currentBindingState->m_bindings)
    {
        switch (bb.bindingType)
        {
            case BindingType::Buffer:
            {
                BufferResourceImpl* bufferResource = static_cast<BufferResourceImpl*>(bb.resource.Ptr());
                const BufferResource::Desc& bufferResourceDesc = bufferResource->getDesc();

                if (bufferResourceDesc.bindFlags & Resource::BindFlag::UnorderedAccess)
                {
                    VkDescriptorBufferInfo bufferInfo;
                    bufferInfo.buffer = bufferResource->m_buffer.m_buffer;
                    bufferInfo.offset = 0;
                    bufferInfo.range = bufferResourceDesc.sizeInBytes;
                        
                    VkWriteDescriptorSet writeInfo = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
                    writeInfo.descriptorCount = 1;
                    writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    writeInfo.dstSet = descriptorSet;
                    writeInfo.dstBinding = bb.binding;
                    writeInfo.dstArrayElement = 0;
                    writeInfo.pBufferInfo = &bufferInfo;

                    vkUpdateDescriptorSets(m_device, 1, &writeInfo, 0, nullptr);
                }
                break;
            }
            default:
            {
                // handle other cases
                break;
            }
        }
    }

    // Create a pipeline layout based on our descriptor set layout(s)

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

    VkPipelineLayout pipelineLayout = 0;
    checkResult(vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, nullptr, &pipelineLayout));

    // Then create a pipeline to use that layout

    VkComputePipelineCreateInfo computePipelineInfo = { VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO };
    computePipelineInfo.stage = m_currentProgram->m_compute;
    computePipelineInfo.layout = pipelineLayout;

    VkPipelineCache pipelineCache = 0;

    VkPipeline pipeline;
    checkResult(vkCreateComputePipelines(m_device, pipelineCache, 1, &computePipelineInfo, nullptr, &pipeline));

    // Also create descriptor sets based on the given pipeline layout

    VkCommandBuffer commandBuffer = beginCommandBuffer();

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);

    vkCmdBindDescriptorSets(
        commandBuffer,
        VK_PIPELINE_BIND_POINT_COMPUTE,
        pipelineLayout,
        0, 1,
        &descriptorSet,
        0,
        nullptr);

    vkCmdDispatch(commandBuffer, x, y, z);

    flushCommandBuffer(commandBuffer);

    vkDestroyPipeline(m_device, pipeline, nullptr);

    // TODO: need to free up the other resources too...
}

// ShaderCompiler interface
ShaderProgram* VKRenderer::compileProgram(const ShaderCompileRequest & request) 
{
    ShaderProgramImpl* impl = new ShaderProgramImpl;
    if (request.computeShader.name)
    {
        impl->m_compute = compileEntryPoint(request.computeShader, VK_SHADER_STAGE_COMPUTE_BIT, impl->m_buffers[0]);
    }
    else
    {
        impl->m_vertex = compileEntryPoint(request.vertexShader, VK_SHADER_STAGE_VERTEX_BIT, impl->m_buffers[0]);
        impl->m_fragment = compileEntryPoint(request.fragmentShader, VK_SHADER_STAGE_FRAGMENT_BIT, impl->m_buffers[1]);
    }
    return impl;
}

} // renderer_test
