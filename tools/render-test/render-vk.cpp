// render-vk.cpp
#include "render-vk.h"

#include "options.h"
#include "render.h"

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

#define RETURN_ON_VK_FAIL(x) { VkResult _vkRes = x; if (_vkRes != VK_SUCCESS) { SLANG_RETURN_ON_FAIL(toSlangResult(_vkRes)); }}

class VKRenderer : public Renderer, public ShaderCompiler
{
public:

    VkInstance                          instance;
    VkPhysicalDevice                    physicalDevice;
    VkPhysicalDeviceProperties          deviceProperties;
    VkPhysicalDeviceFeatures            deviceFeatures;
    VkPhysicalDeviceMemoryProperties    deviceMemoryProperties;
    VkPhysicalDeviceFeatures            enabledFeatures;
    VkDevice                            device;
    VkQueue                             queue;
    VkCommandPool                       commandPool;
    VkSubmitInfo                        submitInfo;
    VkDebugReportCallbackEXT            debugReportCallback;


#define DECLARE_PROC(NAME) PFN_##NAME NAME;
    DECLARE_PROC(vkGetInstanceProcAddr);
    FOREACH_GLOBAL_PROC(DECLARE_PROC)
    FOREACH_INSTANCE_PROC(DECLARE_PROC)
    FOREACH_DEVICE_PROC(DECLARE_PROC)
#undef DECLARE_PROC

    // Renderer interface

	static SlangResult toSlangResult(VkResult res)
	{
		return (res == VK_SUCCESS) ? SLANG_OK : SLANG_FAIL;
	}
	void checkResult(VkResult result)
	{
		assert(result == VK_SUCCESS);
	}

    VkBool32 handleDebugMessage(
        VkDebugReportFlagsEXT flags,
        VkDebugReportObjectTypeEXT objType,
        uint64_t srcObject,
        size_t location,
        int32_t msgCode,
        const char* pLayerPrefix,
        const char* pMsg)
    {
        char const* severity = "message";
        if(flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
            severity = "warning";
        if(flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
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

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugMessageCallback(
        VkDebugReportFlagsEXT flags,
        VkDebugReportObjectTypeEXT objType,
        uint64_t srcObject,
        size_t location,
        int32_t msgCode,
        const char* pLayerPrefix,
        const char* pMsg,
        void* pUserData)
    {
        return ((VKRenderer*) pUserData)->handleDebugMessage(
            flags, objType, srcObject, location, msgCode, pLayerPrefix, pMsg);
    }

    virtual SlangResult initialize(void* inWindowHandle) override
    {
        char const* dynamicLibraryName = "vulkan-1.dll";
        HMODULE vulkan = LoadLibraryA(dynamicLibraryName);
        if(!vulkan)
        {
            fprintf(stderr, "error: failed load '%s'\n", dynamicLibraryName);
            return SLANG_FAIL;
        }

        vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr) GetProcAddress(vulkan, "vkGetInstanceProcAddr");
        if(!vkGetInstanceProcAddr)
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

        instance = 0;

#define LOAD_INSTANCE_PROC(NAME) NAME = (PFN_##NAME) vkGetInstanceProcAddr(instance, #NAME);

        FOREACH_GLOBAL_PROC(LOAD_INSTANCE_PROC);

		RETURN_ON_VK_FAIL(vkCreateInstance(&instanceCreateInfo, nullptr, &instance));

        FOREACH_INSTANCE_PROC(LOAD_INSTANCE_PROC);

#undef LOAD_INSTANCE_PROC


#if ENABLE_VALIDATION_LAYER
        VkDebugReportFlagsEXT debugFlags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;

        VkDebugReportCallbackCreateInfoEXT debugCreateInfo = { VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT };
        debugCreateInfo.pfnCallback = &debugMessageCallback;
        debugCreateInfo.pUserData = this;
        debugCreateInfo.flags = debugFlags;

		RETURN_ON_VK_FAIL(vkCreateDebugReportCallbackEXT(instance, &debugCreateInfo, nullptr, &debugReportCallback));

#endif

        uint32_t physicalDeviceCount = 0;
		RETURN_ON_VK_FAIL(vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr));

        VkPhysicalDevice* physicalDevices = (VkPhysicalDevice*)alloca(
            physicalDeviceCount * sizeof(VkPhysicalDevice));
		RETURN_ON_VK_FAIL(vkEnumeratePhysicalDevices(
            instance, &physicalDeviceCount, physicalDevices));

        uint32_t selectedDeviceIndex = 0;
        // TODO: allow override of selected device
        physicalDevice = physicalDevices[selectedDeviceIndex];

        vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
        vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &deviceMemoryProperties);

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(
            physicalDevice, &queueFamilyCount, nullptr);

        VkQueueFamilyProperties* queueFamilies = (VkQueueFamilyProperties*)alloca(
            queueFamilyCount * sizeof(VkQueueFamilyProperties));
        vkGetPhysicalDeviceQueueFamilyProperties(
            physicalDevice, &queueFamilyCount, queueFamilies);

        // Find a queue that can service our needs
        VkQueueFlags reqQueueFlags =
            VK_QUEUE_GRAPHICS_BIT
            | VK_QUEUE_COMPUTE_BIT;

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
        deviceCreateInfo.pEnabledFeatures = &enabledFeatures;

        deviceCreateInfo.enabledExtensionCount = sizeof(deviceExtensions) / sizeof(deviceExtensions[0]);
        deviceCreateInfo.ppEnabledExtensionNames = &deviceExtensions[0];

		RETURN_ON_VK_FAIL(vkCreateDevice(
            physicalDevice, &deviceCreateInfo, nullptr, &device));

#define LOAD_DEVICE_PROC(NAME) NAME = (PFN_##NAME) vkGetDeviceProcAddr(device, #NAME);
        FOREACH_DEVICE_PROC(LOAD_DEVICE_PROC)
#undef LOAD_DEVICE_PROC

        // Create a command pool

        VkCommandPoolCreateInfo commandPoolCreateInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
        commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndex;
        commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		RETURN_ON_VK_FAIL(vkCreateCommandPool(
            device, &commandPoolCreateInfo, nullptr, &commandPool));

        vkGetDeviceQueue(
            device,
            queueFamilyIndex,
            0,
            &queue);

        // set up swap chain



        // create command buffers

        // depth/stencil?

        // render pass?

        // pipeline cache

        // frame buffer



		// create semaphores for sync

		return SLANG_OK;
    }

    float clearColor[4];
    virtual void setClearColor(float const* color) override
    {
        for(int ii = 0; ii < 4; ++ii)
            clearColor[ii] = color[ii];
    }

    virtual void clearFrame() override
    {
    }

    virtual void presentFrame() override
    {
    }

    virtual SlangResult captureScreenShot(char const* outputPath) override
    {
		return SLANG_FAIL;
    }

    virtual ShaderCompiler* getShaderCompiler() override
    {
        return this;
    }

    VkCommandBuffer getCommandBuffer()
    {
        VkCommandBufferAllocateInfo info = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
        info.commandPool = commandPool;
        info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        info.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        checkResult(vkAllocateCommandBuffers(
            device, &info, &commandBuffer));

        return commandBuffer;
    }

    VkCommandBuffer beginCommandBuffer()
    {
        VkCommandBuffer commandBuffer = getCommandBuffer();

        VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
        checkResult(vkBeginCommandBuffer(commandBuffer, &beginInfo));

        return commandBuffer;
    }

    void flushCommandBuffer(VkCommandBuffer commandBuffer)
    {
        checkResult(vkEndCommandBuffer(commandBuffer));

        VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        checkResult(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));
        checkResult(vkQueueWaitIdle(queue));

        vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
    }

    struct BufferImpl
    {
        VkBuffer        buffer;
        VkDeviceMemory  memory;
    };

    BufferImpl createBufferImpl(
        size_t                  bufferSize,
        VkBufferUsageFlags      usage,
        VkMemoryPropertyFlags   reqMemoryProperties,
        void const*             initData = nullptr)
    {
        if( initData )
        {
            // TODO: what if we are allocating it as CPU-writable anyway?
            usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        }

        VkBufferCreateInfo bufferCreateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        bufferCreateInfo.size = bufferSize;
        bufferCreateInfo.usage = usage;

        VkBuffer buffer;
        checkResult(vkCreateBuffer(
            device, &bufferCreateInfo, nullptr, &buffer));

        VkMemoryRequirements memoryReqs = {};
        vkGetBufferMemoryRequirements(device, buffer, &memoryReqs);

        uint32_t memoryTypeIndex = getMemoryTypeIndex(
            memoryReqs.memoryTypeBits, reqMemoryProperties);

        VkMemoryPropertyFlags actualMemoryProperites = deviceMemoryProperties.memoryTypes[memoryTypeIndex].propertyFlags;

        VkMemoryAllocateInfo allocateInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
        allocateInfo.allocationSize = memoryReqs.size;
        allocateInfo.memoryTypeIndex = memoryTypeIndex;

        VkDeviceMemory memory;
        checkResult(vkAllocateMemory(
            device, &allocateInfo, nullptr, &memory));

        checkResult(vkBindBufferMemory(
            device, buffer, memory, 0));

        if( initData )
        {
            // TODO: only create staging buffer if the memory type
            // used for the buffer doesn't let us fill things in
            // directly.

            BufferImpl staging = createBufferImpl(
                bufferSize,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

            // Copy into staging buffer
            void* mappedData = nullptr;
            checkResult(vkMapMemory(device, staging.memory, 0, bufferSize, 0, &mappedData));
            memcpy(mappedData, initData, bufferSize);
            vkUnmapMemory(device, staging.memory);

            // Copy from staging buffer to real buffer
            VkCommandBuffer commandBuffer = beginCommandBuffer();

            VkBufferCopy copyInfo = {};
            copyInfo.size = bufferSize;
            vkCmdCopyBuffer(
                commandBuffer,
                staging.buffer,
                buffer,
                1,
                &copyInfo);

            flushCommandBuffer(commandBuffer);

            // Now destroy the staging buffer
            vkDestroyBuffer(device, staging.buffer, nullptr);
            vkFreeMemory(device, staging.memory, nullptr);
        }

        BufferImpl impl;
        impl.buffer = buffer;
        impl.memory = memory;
        return impl;
    }

    virtual Buffer* createBuffer(BufferDesc const& desc) override
    {
        size_t bufferSize = desc.size;

        VkBufferUsageFlags usage = 0;
        VkMemoryPropertyFlags reqMemoryProperties = 0;

        switch( desc.flavor )
        {
        case BufferFlavor::Constant:
            usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
            reqMemoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
            break;

        case BufferFlavor::Vertex:
            usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            reqMemoryProperties = 0;
            break;
        }

        BufferImpl bufferImpl = createBufferImpl(
            bufferSize,
            usage,
            reqMemoryProperties,
            desc.initData);

        BufferImpl* bufferPtr = new BufferImpl();
        *bufferPtr = bufferImpl;
        return (Buffer*) bufferPtr;
    }

    struct InputLayoutImpl
    {
    };

    virtual InputLayout* createInputLayout(InputElementDesc const* inputElements, UInt inputElementCount) override
    {
        InputLayoutImpl* impl = new InputLayoutImpl();

        // TODO: actually initialize things

        return (InputLayout*) impl;
    }

    virtual void* map(Buffer* buffer, MapFlavor flavor) override
    {
        return nullptr;
    }

    virtual void unmap(Buffer* buffer) override
    {
    }

    virtual void setInputLayout(InputLayout* inputLayout) override
    {
    }

    virtual void setPrimitiveTopology(PrimitiveTopology topology) override
    {
    }

    virtual void setVertexBuffers(UInt startSlot, UInt slotCount, Buffer* const* buffers, UInt const* strides, UInt const* offsets) override
    {
    }

    struct ShaderProgramImpl
    {
        VkPipelineShaderStageCreateInfo compute;
        VkPipelineShaderStageCreateInfo vertex;
        VkPipelineShaderStageCreateInfo fragment;
    };

    ShaderProgramImpl* currentProgram = nullptr;
    virtual void setShaderProgram(ShaderProgram* program) override
    {
        currentProgram = (ShaderProgramImpl*) program;
    }

    virtual void setConstantBuffers(UInt startSlot, UInt slotCount, Buffer* const* buffers, UInt const* offsets) override
    {
    }

    virtual void draw(UInt vertexCount, UInt startVertex = 0) override
    {
    }

    struct BindingImpl
    {
        ShaderInputType type;
        InputBufferType bufferType; // Only valid if `type` is `Buffer`

        VkImageView     srv;
        VkBufferView    uav;
        VkBuffer        buffer;
        VkSampler       samplerState;

        int binding = 0;
        bool isOutput = false;
        int bufferLength = 0;
    };

    struct BindingStateImpl
    {
        Slang::List<BindingImpl> bindings;
        int numRenderTargets;
    };

    uint32_t getMemoryTypeIndex(
        uint32_t                inTypeBits,
        VkMemoryPropertyFlags   properties)
    {
        uint32_t typeBits = inTypeBits;
        uint32_t typeIndex = 0;
        while( typeBits )
        {
            if((deviceMemoryProperties.memoryTypes[typeIndex].propertyFlags & properties) == properties)
            {
                return typeIndex;
            }
            typeIndex++;
            typeBits >>= 1;
        }

        assert(!"failed to find a usable memory type");
        return uint32_t(-1);
    }

    void createInputBuffer(
        ShaderInputLayoutEntry const& entry,
        InputBufferDesc const& bufferDesc, 
        Slang::List<unsigned int> const& bufferData,
        VkBuffer &bufferOut,
        VkBufferView &uavOut,
        VkImageView  &srvOut)
    {
        size_t bufferSize = bufferData.Count() * sizeof(unsigned int);
        void const* initData = bufferData.Buffer();

        VkBufferUsageFlags usage = 0;
        VkMemoryPropertyFlags reqMemoryProperties = 0;

        switch( bufferDesc.type )
        {
        case InputBufferType::ConstantBuffer:
            usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
            reqMemoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
            break;

        case InputBufferType::StorageBuffer:
            usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
            reqMemoryProperties = 0;
            break;
        }

        // If we are going to read back from the buffer, be sure to request
        // the required access.
        if(entry.isOutput)
        {
            usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        }

        BufferImpl bufferImpl = createBufferImpl(
            bufferSize,
            usage,
            reqMemoryProperties,
            initData);

        // TODO: need to hang onto the `memory` field so
        // that we can release it when we are done.

        bufferOut = bufferImpl.buffer;

        // Fill in any views needed
        switch( bufferDesc.type )
        {
        case InputBufferType::ConstantBuffer:
            break;

        case InputBufferType::StorageBuffer:
            {
            }
            break;
        }
    }

    void createInputTexture(
        InputTextureDesc const& inputDesc,
        VkImageView&            viewOut)
    {
        TextureData texData;
        generateTextureData(texData, inputDesc);
        assert(!"unimplemented");
    }

    void createInputSampler(
        InputSamplerDesc const& inputDesc,
        VkSampler&              stateOut)
    {
        assert(!"unimplemented");
    }

    virtual BindingState* createBindingState(const ShaderInputLayout & layout)
    {
        BindingStateImpl* bindingState = new BindingStateImpl();
        bindingState->numRenderTargets = layout.numRenderTargets;
        for (auto & entry : layout.entries)
        {
            BindingImpl binding;
            binding.type = entry.type;
            binding.binding = entry.hlslBinding;
            binding.isOutput = entry.isOutput;
            switch (entry.type)
            {
            case ShaderInputType::Buffer:
            {
                createInputBuffer(entry, entry.bufferDesc, entry.bufferData, binding.buffer, binding.uav, binding.srv);
                binding.bufferLength = (int)(entry.bufferData.Count() * sizeof(unsigned int));
                binding.bufferType = entry.bufferDesc.type;
            }
            break;
            case ShaderInputType::Texture:
            {
                createInputTexture(entry.textureDesc, binding.srv);
            }
            break;
            case ShaderInputType::Sampler:
            {
                createInputSampler(entry.samplerDesc, binding.samplerState);
            }
            break;
            case ShaderInputType::CombinedTextureSampler:
            {
                throw "not implemented";
            }
            break;
            }
            bindingState->bindings.Add(binding);
        }

        return (BindingState*) bindingState;
    }

    BindingStateImpl* currentBindingState = nullptr;
    virtual void setBindingState(BindingState * state)
    {
        currentBindingState = (BindingStateImpl*) state;
    }

    virtual void serializeOutput(BindingState* s, const char * fileName)
    {
        auto state = (BindingStateImpl*) s;

        FILE * f = fopen(fileName, "wb");
        int id = 0;
        for (auto& bb: state->bindings)
        {
            if (bb.isOutput)
            {
                if (bb.buffer)
                {
                    // create staging buffer
                    size_t bufferSize = bb.bufferLength;
                    BufferImpl staging = createBufferImpl(
                        bufferSize,
                        VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

                    // Copy from real buffer to staging buffer
                    VkCommandBuffer commandBuffer = beginCommandBuffer();

                    VkBufferCopy copyInfo = {};
                    copyInfo.size = bufferSize;
                    vkCmdCopyBuffer(
                        commandBuffer,
                        bb.buffer,
                        staging.buffer,
                        1,
                        &copyInfo);

                    flushCommandBuffer(commandBuffer);

                    // Write out the data from the buffer
                    void* mappedData = nullptr;
                    checkResult(vkMapMemory(device, staging.memory, 0, bufferSize, 0, &mappedData));

                    auto ptr = (unsigned int *) mappedData;
                    for (auto i = 0u; i < bufferSize / sizeof(unsigned int); i++)
                        fprintf(f, "%X\n", ptr[i]);

                    vkUnmapMemory(device, staging.memory);

                    // Now destroy the staging buffer
                    vkDestroyBuffer(device, staging.buffer, nullptr);
                    vkFreeMemory(device, staging.memory, nullptr);
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

    virtual void dispatchCompute(int x, int y, int z) override
    {


        // HACK: create a new pipeline for every call

        // First create a pipeline layout based on what is bound

        Slang::List<VkDescriptorSetLayoutBinding> bindings;

        for( auto bb : currentBindingState->bindings )
        {
            switch( bb.type )
            {
            case ShaderInputType::Buffer:
                {
                    switch(bb.bufferType)
                    {
                    case InputBufferType::StorageBuffer:
                        {
                            VkDescriptorSetLayoutBinding binding = {};
                            binding.binding = bb.binding;
                            binding.descriptorCount = 1;
                            binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                            binding.stageFlags = VK_SHADER_STAGE_ALL;

                            bindings.Add(binding);
                        }
                        break;

                    default:
                        // handle other cases
                        break;
                    }
                }
                break;

            default:
                // TODO: handle the other cases
                break;
            }
        }

        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
        descriptorSetLayoutInfo.bindingCount = uint32_t(bindings.Count());
        descriptorSetLayoutInfo.pBindings = bindings.Buffer();

        VkDescriptorSetLayout descriptorSetLayout = 0;
        checkResult(vkCreateDescriptorSetLayout(
            device, &descriptorSetLayoutInfo, nullptr, &descriptorSetLayout));

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
        checkResult(vkCreateDescriptorPool(
            device, &descriptorPoolInfo, nullptr, &descriptorPool));

        // Create a descriptor set based on our layout

        VkDescriptorSetAllocateInfo descriptorSetAllocInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
        descriptorSetAllocInfo.descriptorPool = descriptorPool;
        descriptorSetAllocInfo.descriptorSetCount = 1;
        descriptorSetAllocInfo.pSetLayouts = &descriptorSetLayout;

        VkDescriptorSet descriptorSet;
        checkResult(vkAllocateDescriptorSets(
            device, &descriptorSetAllocInfo, &descriptorSet));

        // Fill in the descritpor set, using our binding information
        for( auto bb : currentBindingState->bindings )
        {
            switch( bb.type )
            {
            case ShaderInputType::Buffer:
                {
                    switch(bb.bufferType)
                    {
                    case InputBufferType::StorageBuffer:
                        {
                            VkDescriptorBufferInfo bufferInfo;
                            bufferInfo.buffer = bb.buffer;
                            bufferInfo.offset = 0;
                            bufferInfo.range = bb.bufferLength;

                            VkWriteDescriptorSet writeInfo = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
                            writeInfo.descriptorCount = 1;
                            writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                            writeInfo.dstSet = descriptorSet;
                            writeInfo.dstBinding = bb.binding;
                            writeInfo.dstArrayElement = 0;
                            writeInfo.pBufferInfo = &bufferInfo;

                            vkUpdateDescriptorSets(
                                device,
                                1,
                                &writeInfo,
                                0,
                                nullptr);
                        }
                        break;

                    default:
                        // handle other cases
                        break;
                    }
                }
                break;

            default:
                // TODO: handle the other cases
                break;
            }
        }


        // Create a pipeline layout based on our descriptor set layout(s)

        VkPipelineLayoutCreateInfo pipelineLayoutInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

        VkPipelineLayout pipelineLayout = 0;
        checkResult(vkCreatePipelineLayout(
            device, &pipelineLayoutInfo, nullptr, &pipelineLayout));

        // Then create a pipeline to use that layout

        VkComputePipelineCreateInfo computePipelineInfo = { VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO };
        computePipelineInfo.stage = currentProgram->compute;
        computePipelineInfo.layout = pipelineLayout;

        VkPipelineCache pipelineCache = 0;

        VkPipeline pipeline;
        checkResult(vkCreateComputePipelines(
            device, pipelineCache, 1, &computePipelineInfo, nullptr, &pipeline));

        // Also create descriptor sets based on the given pipeline layout

        VkCommandBuffer commandBuffer = beginCommandBuffer();

        vkCmdBindPipeline(
            commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);

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

        vkDestroyPipeline(device, pipeline, nullptr);

        // TODO: need to free up the other resources too...
    }

    // ShaderCompiler interface

    VkPipelineShaderStageCreateInfo compileEntryPoint(
        ShaderCompileRequest::EntryPoint const& entryPointRequest,
        VkShaderStageFlagBits                   stage)
    {
        char const* dataBegin = entryPointRequest.source.dataBegin;
        char const* dataEnd = entryPointRequest.source.dataEnd;

        // We need to make a copy of the code, since the Slang compiler
        // will free the memory after a compile request is closed.
        size_t codeSize = dataEnd - dataBegin;
        char* codeBegin = (char*) malloc(codeSize);
        memcpy(codeBegin, dataBegin, codeSize);

        VkShaderModuleCreateInfo moduleCreateInfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
        moduleCreateInfo.pCode = (uint32_t*) codeBegin;
        moduleCreateInfo.codeSize = codeSize;

        VkShaderModule module;
        checkResult(vkCreateShaderModule(
            device,
            &moduleCreateInfo,
            nullptr,
            &module));

        VkPipelineShaderStageCreateInfo shaderStageCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
        shaderStageCreateInfo.stage = stage;

        shaderStageCreateInfo.module = module;
        shaderStageCreateInfo.pName = "main";
        return shaderStageCreateInfo;
    }

    virtual ShaderProgram* compileProgram(ShaderCompileRequest const& request) override
    {
        ShaderProgramImpl* impl = new ShaderProgramImpl();

        if( request.computeShader.name )
        {
            impl->compute = compileEntryPoint(
                request.computeShader,
                VK_SHADER_STAGE_COMPUTE_BIT);
        }
        else
        {
            impl->vertex = compileEntryPoint(
                request.vertexShader,
                VK_SHADER_STAGE_VERTEX_BIT);

            impl->fragment = compileEntryPoint(
                request.fragmentShader,
                VK_SHADER_STAGE_FRAGMENT_BIT);
        }

        return (ShaderProgram*) impl;
    }
};



Renderer* createVKRenderer()
{
    return new VKRenderer();
}

} // renderer_test
