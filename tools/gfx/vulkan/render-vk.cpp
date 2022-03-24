// render-vk.cpp
#include "render-vk.h"
#include "core/slang-blob.h"
#include "vk-util.h"

// Vulkan has a different coordinate system to ogl
// http://anki3d.org/vulkan-coordinate-system/
#ifndef ENABLE_VALIDATION_LAYER
#    if _DEBUG
#        define ENABLE_VALIDATION_LAYER 1
#    else
#        define ENABLE_VALIDATION_LAYER 0
#    endif
#endif

#ifdef _MSC_VER
#    include <stddef.h>
#    pragma warning(disable : 4996)
#    if (_MSC_VER < 1900)
#        define snprintf sprintf_s
#    endif
#endif

#if SLANG_WINDOWS_FAMILY
#    include <dxgi1_2.h>
#endif

namespace gfx
{
using namespace Slang;

namespace vk
{
namespace
{
size_t calcRowSize(Format format, int width)
{
    FormatInfo sizeInfo;
    gfxGetFormatInfo(format, &sizeInfo);
    return size_t(
        (width + sizeInfo.blockWidth - 1) / sizeInfo.blockWidth * sizeInfo.blockSizeInBytes);
}

size_t calcNumRows(Format format, int height)
{
    FormatInfo sizeInfo;
    gfxGetFormatInfo(format, &sizeInfo);
    return (size_t)(height + sizeInfo.blockHeight - 1) / sizeInfo.blockHeight;
}
VkAttachmentLoadOp translateLoadOp(IRenderPassLayout::AttachmentLoadOp loadOp)
{
    switch (loadOp)
    {
    case IRenderPassLayout::AttachmentLoadOp::Clear:
        return VK_ATTACHMENT_LOAD_OP_CLEAR;
    case IRenderPassLayout::AttachmentLoadOp::Load:
        return VK_ATTACHMENT_LOAD_OP_LOAD;
    default:
        return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    }
}

VkAttachmentStoreOp translateStoreOp(IRenderPassLayout::AttachmentStoreOp storeOp)
{
    switch (storeOp)
    {
    case IRenderPassLayout::AttachmentStoreOp::Store:
        return VK_ATTACHMENT_STORE_OP_STORE;
    default:
        return VK_ATTACHMENT_STORE_OP_DONT_CARE;
    }
}

VkPipelineCreateFlags translateRayTracingPipelineFlags(RayTracingPipelineFlags::Enum flags)
{
    VkPipelineCreateFlags vkFlags = 0;
    if (flags & RayTracingPipelineFlags::Enum::SkipTriangles)
        vkFlags |= VK_PIPELINE_CREATE_RAY_TRACING_SKIP_TRIANGLES_BIT_KHR;
    if (flags & RayTracingPipelineFlags::Enum::SkipProcedurals)
        vkFlags |= VK_PIPELINE_CREATE_RAY_TRACING_SKIP_AABBS_BIT_KHR;

    return vkFlags;
}

uint32_t getMipLevelSize(uint32_t mipLevel, uint32_t size)
{
    return Math::Max(1u, (size >> mipLevel));
}

VkImageLayout translateImageLayout(ResourceState state)
{
    switch (state)
    {
    case ResourceState::Undefined:
        return VK_IMAGE_LAYOUT_UNDEFINED;
    case ResourceState::PreInitialized:
        return VK_IMAGE_LAYOUT_PREINITIALIZED;
    case ResourceState::UnorderedAccess:
        return VK_IMAGE_LAYOUT_GENERAL;
    case ResourceState::RenderTarget:
        return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    case ResourceState::DepthRead:
        return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    case ResourceState::DepthWrite:
        return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    case ResourceState::ShaderResource:
        return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    case ResourceState::ResolveDestination:
    case ResourceState::CopyDestination:
        return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    case ResourceState::ResolveSource:
    case ResourceState::CopySource:
        return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    case ResourceState::Present:
        return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    default:
        assert(!"Unsupported");
        return VK_IMAGE_LAYOUT_UNDEFINED;
    }
}

VkAccessFlagBits calcAccessFlags(ResourceState state)
{
    switch (state)
    {
    case ResourceState::Undefined:
    case ResourceState::Present:
    case ResourceState::PreInitialized:
        return VkAccessFlagBits(0);
    case ResourceState::VertexBuffer:
        return VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
    case ResourceState::ConstantBuffer:
        return VK_ACCESS_UNIFORM_READ_BIT;
    case ResourceState::IndexBuffer:
        return VK_ACCESS_INDEX_READ_BIT;
    case ResourceState::RenderTarget:
        return VkAccessFlagBits(
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT);
    case ResourceState::ShaderResource:
        return VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
    case ResourceState::UnorderedAccess:
        return VkAccessFlagBits(VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT);
    case ResourceState::DepthRead:
        return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
    case ResourceState::DepthWrite:
        return VkAccessFlagBits(
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);
    case ResourceState::IndirectArgument:
        return VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
    case ResourceState::ResolveDestination:
    case ResourceState::CopyDestination:
        return VK_ACCESS_TRANSFER_WRITE_BIT;
    case ResourceState::ResolveSource:
    case ResourceState::CopySource:
        return VK_ACCESS_TRANSFER_READ_BIT;
    case ResourceState::AccelerationStructure:
        return VkAccessFlagBits(
            VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR |
            VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR);
    case ResourceState::AccelerationStructureBuildInput:
        return VkAccessFlagBits(VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR);
    case ResourceState::General:
        return VkAccessFlagBits(VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT);
    default:
        assert(!"Unsupported");
        return VkAccessFlagBits(0);
    }
}

VkPipelineStageFlagBits calcPipelineStageFlags(ResourceState state, bool src)
{
    switch (state)
    {
    case ResourceState::Undefined:
    case ResourceState::PreInitialized:
        assert(src);
        return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    case ResourceState::VertexBuffer:
    case ResourceState::IndexBuffer:
        return VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
    case ResourceState::ConstantBuffer:
    case ResourceState::UnorderedAccess:
        return VkPipelineStageFlagBits(
            VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
            VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT |
            VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT |
            VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
    case ResourceState::ShaderResource:
        return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    case ResourceState::RenderTarget:
        return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    case ResourceState::DepthRead:
    case ResourceState::DepthWrite:
        return VkPipelineStageFlagBits(
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT);
    case ResourceState::IndirectArgument:
        return VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;
    case ResourceState::CopySource:
    case ResourceState::CopyDestination:
    case ResourceState::ResolveSource:
    case ResourceState::ResolveDestination:
        return VK_PIPELINE_STAGE_TRANSFER_BIT;
    case ResourceState::Present:
        return src ? VkPipelineStageFlagBits(
                         VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT | VK_PIPELINE_STAGE_ALL_COMMANDS_BIT)
                   : VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    case ResourceState::General:
        return VkPipelineStageFlagBits(VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    case ResourceState::AccelerationStructure:
        return VkPipelineStageFlagBits(
            VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
            VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT |
            VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT |
            VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR |
            VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR);
    case ResourceState::AccelerationStructureBuildInput:
        return VkPipelineStageFlagBits(VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR);
    default:
        assert(!"Unsupported");
        return VkPipelineStageFlagBits(0);
    }
}

VkAccessFlags translateAccelerationStructureAccessFlag(AccessFlag access)
{
    VkAccessFlags result = 0;
    if ((uint32_t)access & (uint32_t)AccessFlag::Read)
        result |= VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR | VK_ACCESS_SHADER_READ_BIT |
                  VK_ACCESS_TRANSFER_READ_BIT;
    if ((uint32_t)access & (uint32_t)AccessFlag::Write)
        result |= VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
    return result;
}

VkBufferUsageFlagBits _calcBufferUsageFlags(ResourceState state)
{
    switch (state)
    {
    case ResourceState::VertexBuffer:
        return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    case ResourceState::IndexBuffer:
        return VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    case ResourceState::ConstantBuffer:
        return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    case ResourceState::StreamOutput:
        return VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT;
    case ResourceState::RenderTarget:
    case ResourceState::DepthRead:
    case ResourceState::DepthWrite:
        {
            assert(!"Invalid resource state for buffer resource.");
            return VkBufferUsageFlagBits(0);
        }
    case ResourceState::UnorderedAccess:
        return (
            VkBufferUsageFlagBits)(VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    case ResourceState::ShaderResource:
        return (
            VkBufferUsageFlagBits)(VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    case ResourceState::CopySource:
        return VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    case ResourceState::CopyDestination:
        return VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    case ResourceState::AccelerationStructure:
        return VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
    case ResourceState::IndirectArgument:
        return VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    case ResourceState::AccelerationStructureBuildInput:
        return VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
    default:
        return VkBufferUsageFlagBits(0);
    }
}

VkBufferUsageFlagBits _calcBufferUsageFlags(ResourceStateSet states)
{
    int dstFlags = 0;
    for (uint32_t i = 0; i < (uint32_t)ResourceState::_Count; i++)
    {
        auto state = (ResourceState)i;
        if (states.contains(state))
            dstFlags |= _calcBufferUsageFlags(state);
    }
    return VkBufferUsageFlagBits(dstFlags);
}

VkImageUsageFlagBits _calcImageUsageFlags(ResourceState state)
{
    switch (state)
    {
    case ResourceState::RenderTarget:
        return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    case ResourceState::DepthWrite:
        return VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    case ResourceState::DepthRead:
        return VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    case ResourceState::ShaderResource:
        return VK_IMAGE_USAGE_SAMPLED_BIT;
    case ResourceState::UnorderedAccess:
        return VK_IMAGE_USAGE_STORAGE_BIT;
    case ResourceState::CopySource:
        return VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    case ResourceState::CopyDestination:
        return VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    case ResourceState::ResolveSource:
        return VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    case ResourceState::ResolveDestination:
        return VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    case ResourceState::Present:
        return VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    case ResourceState::General:
        return (VkImageUsageFlagBits)0;
    default:
        {
            assert(!"Unsupported");
            return VkImageUsageFlagBits(0);
        }
    }
}

VkImageViewType _calcImageViewType(ITextureResource::Type type, const ITextureResource::Desc& desc)
{
    switch (type)
    {
    case IResource::Type::Texture1D:
        return desc.arraySize > 1 ? VK_IMAGE_VIEW_TYPE_1D_ARRAY : VK_IMAGE_VIEW_TYPE_1D;
    case IResource::Type::Texture2D:
        return desc.arraySize > 1 ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
    case IResource::Type::TextureCube:
        return desc.arraySize > 1 ? VK_IMAGE_VIEW_TYPE_CUBE_ARRAY : VK_IMAGE_VIEW_TYPE_CUBE;
    case IResource::Type::Texture3D:
        {
            // Can't have an array and 3d texture
            assert(desc.arraySize <= 1);
            if (desc.arraySize <= 1)
            {
                return VK_IMAGE_VIEW_TYPE_3D;
            }
            break;
        }
    default:
        break;
    }

    return VK_IMAGE_VIEW_TYPE_MAX_ENUM;
}

VkImageUsageFlagBits _calcImageUsageFlags(ResourceStateSet states)
{
    int dstFlags = 0;
    for (uint32_t i = 0; i < (uint32_t)ResourceState::_Count; i++)
    {
        auto state = (ResourceState)i;
        if (states.contains(state))
            dstFlags |= _calcImageUsageFlags(state);
    }
    return VkImageUsageFlagBits(dstFlags);
}

VkImageUsageFlags _calcImageUsageFlags(
    ResourceStateSet states, MemoryType memoryType, const void* initData)
{
    VkImageUsageFlags usage = _calcImageUsageFlags(states);

    if (memoryType == MemoryType::Upload || initData)
    {
        usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }

    return usage;
}

VkAccessFlags calcAccessFlagsFromImageLayout(VkImageLayout layout)
{
    switch (layout)
    {
    case VK_IMAGE_LAYOUT_UNDEFINED:
    case VK_IMAGE_LAYOUT_GENERAL:
    case VK_IMAGE_LAYOUT_PREINITIALIZED:
    case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
        return (VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT);
    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        return (VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
    case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
    case VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL:
    case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
    case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
        return (
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT);
    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
    case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL:
    case VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL:
        return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        return VK_ACCESS_SHADER_READ_BIT;
    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
        return VK_ACCESS_TRANSFER_READ_BIT;
    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        return VK_ACCESS_TRANSFER_WRITE_BIT;
    default:
        assert(!"Unsupported VkImageLayout");
        return (VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT);
    }
}

VkPipelineStageFlags calcPipelineStageFlagsFromImageLayout(VkImageLayout layout)
{
    switch (layout)
    {
    case VK_IMAGE_LAYOUT_UNDEFINED:
    case VK_IMAGE_LAYOUT_PREINITIALIZED:
    case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
    case VK_IMAGE_LAYOUT_GENERAL:
        return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        return (VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
        return VK_PIPELINE_STAGE_TRANSFER_BIT;
    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        return VK_PIPELINE_STAGE_TRANSFER_BIT;
    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
    case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
    case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
    case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
    case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL:
    case VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL:
    case VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL:
        return (
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT);
    default:
        assert(!"Unsupported VkImageLayout");
        return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    }
}

void _writeTimestamp(
    VulkanApi* api, VkCommandBuffer vkCmdBuffer, IQueryPool* queryPool, SlangInt index)
{
    auto queryPoolImpl = static_cast<QueryPoolImpl*>(queryPool);
    api->vkCmdResetQueryPool(vkCmdBuffer, queryPoolImpl->m_pool, (uint32_t)index, 1);
    api->vkCmdWriteTimestamp(
        vkCmdBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, queryPoolImpl->m_pool, (uint32_t)index);
}
} // namespace

DeviceImpl::~DeviceImpl()
{
    // Check the device queue is valid else, we can't wait on it..
    if (m_deviceQueue.isValid())
    {
        waitForGpu();
    }

    m_shaderObjectLayoutCache = decltype(m_shaderObjectLayoutCache)();
    shaderCache.free();
    m_deviceObjectsWithPotentialBackReferences.clearAndDeallocate();

    if (m_api.vkDestroySampler)
    {
        m_api.vkDestroySampler(m_device, m_defaultSampler, nullptr);
    }

    m_deviceQueue.destroy();

    descriptorSetAllocator.close();

    m_emptyFramebuffer = nullptr;

    if (m_device != VK_NULL_HANDLE)
    {
        if (m_desc.existingDeviceHandles.handles[2].handleValue == 0)
            m_api.vkDestroyDevice(m_device, nullptr);
        m_device = VK_NULL_HANDLE;
        if (m_debugReportCallback != VK_NULL_HANDLE)
            m_api.vkDestroyDebugReportCallbackEXT(m_api.m_instance, m_debugReportCallback, nullptr);
        if (m_api.m_instance != VK_NULL_HANDLE &&
            m_desc.existingDeviceHandles.handles[0].handleValue == 0)
            m_api.vkDestroyInstance(m_api.m_instance, nullptr);
    }
}

VkBool32 DeviceImpl::handleDebugMessage(
    VkDebugReportFlagsEXT flags,
    VkDebugReportObjectTypeEXT objType,
    uint64_t srcObject,
    size_t location,
    int32_t msgCode,
    const char* pLayerPrefix,
    const char* pMsg)
{
    DebugMessageType msgType = DebugMessageType::Info;

    char const* severity = "message";
    if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
    {
        severity = "warning";
        msgType = DebugMessageType::Warning;
    }
    if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
    {
        severity = "error";
        msgType = DebugMessageType::Error;
    }

    // pMsg can be really big (it can be assembler dump for example)
    // Use a dynamic buffer to store
    size_t bufferSize = strlen(pMsg) + 1 + 1024;
    List<char> bufferArray;
    bufferArray.setCount(bufferSize);
    char* buffer = bufferArray.getBuffer();

    sprintf_s(buffer, bufferSize, "%s: %s %d: %s\n", pLayerPrefix, severity, msgCode, pMsg);

    getDebugCallback()->handleMessage(msgType, DebugMessageSource::Driver, buffer);
    return VK_FALSE;
}

VKAPI_ATTR VkBool32 VKAPI_CALL DeviceImpl::debugMessageCallback(
    VkDebugReportFlagsEXT flags,
    VkDebugReportObjectTypeEXT objType,
    uint64_t srcObject,
    size_t location,
    int32_t msgCode,
    const char* pLayerPrefix,
    const char* pMsg,
    void* pUserData)
{
    return ((DeviceImpl*)pUserData)
        ->handleDebugMessage(flags, objType, srcObject, location, msgCode, pLayerPrefix, pMsg);
}

Result DeviceImpl::getNativeDeviceHandles(InteropHandles* outHandles)
{
    outHandles->handles[0].handleValue = (uint64_t)m_api.m_instance;
    outHandles->handles[0].api = InteropHandleAPI::Vulkan;
    outHandles->handles[1].handleValue = (uint64_t)m_api.m_physicalDevice;
    outHandles->handles[1].api = InteropHandleAPI::Vulkan;
    outHandles->handles[2].handleValue = (uint64_t)m_api.m_device;
    outHandles->handles[2].api = InteropHandleAPI::Vulkan;
    return SLANG_OK;
}

Result DeviceImpl::initVulkanInstanceAndDevice(
    const InteropHandle* handles, bool useValidationLayer)
{
    m_features.clear();

    m_queueAllocCount = 0;

    VkInstance instance = VK_NULL_HANDLE;
    if (handles[0].handleValue == 0)
    {
        VkApplicationInfo applicationInfo = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
        applicationInfo.pApplicationName = "slang-gfx";
        applicationInfo.pEngineName = "slang-gfx";
        applicationInfo.apiVersion = VK_API_VERSION_1_1;
        applicationInfo.engineVersion = 1;
        applicationInfo.applicationVersion = 1;

        Array<const char*, 6> instanceExtensions;

        instanceExtensions.add(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        instanceExtensions.add(VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME);

        // Software (swiftshader) implementation currently does not support surface extension,
        // so only use it with a hardware implementation.
        if (!m_api.m_module->isSoftware())
        {
            instanceExtensions.add(VK_KHR_SURFACE_EXTENSION_NAME);
            // Note: this extension is not yet supported by nvidia drivers, disable for now.
            // instanceExtensions.add("VK_GOOGLE_surfaceless_query");
#if SLANG_WINDOWS_FAMILY
            instanceExtensions.add(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(SLANG_ENABLE_XLIB)
            instanceExtensions.add(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#endif
#if ENABLE_VALIDATION_LAYER
            instanceExtensions.add(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
#endif
        }

        VkInstanceCreateInfo instanceCreateInfo = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
        instanceCreateInfo.pApplicationInfo = &applicationInfo;
        instanceCreateInfo.enabledExtensionCount = (uint32_t)instanceExtensions.getCount();
        instanceCreateInfo.ppEnabledExtensionNames = &instanceExtensions[0];

        if (useValidationLayer)
        {
            // Depending on driver version, validation layer may or may not exist.
            // Newer drivers comes with "VK_LAYER_KHRONOS_validation", while older
            // drivers provide only the deprecated
            // "VK_LAYER_LUNARG_standard_validation" layer.
            // We will check what layers are available, and use the newer
            // "VK_LAYER_KHRONOS_validation" layer when possible.
            uint32_t layerCount;
            m_api.vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

            List<VkLayerProperties> availableLayers;
            availableLayers.setCount(layerCount);
            m_api.vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.getBuffer());

            const char* layerNames[] = {nullptr};
            for (auto& layer : availableLayers)
            {
                if (strncmp(
                        layer.layerName,
                        "VK_LAYER_KHRONOS_validation",
                        sizeof("VK_LAYER_KHRONOS_validation")) == 0)
                {
                    layerNames[0] = "VK_LAYER_KHRONOS_validation";
                    break;
                }
            }
            // On older drivers, only "VK_LAYER_LUNARG_standard_validation" exists,
            // so we try to use it if we can't find "VK_LAYER_KHRONOS_validation".
            if (!layerNames[0])
            {
                for (auto& layer : availableLayers)
                {
                    if (strncmp(
                            layer.layerName,
                            "VK_LAYER_LUNARG_standard_validation",
                            sizeof("VK_LAYER_LUNARG_standard_validation")) == 0)
                    {
                        layerNames[0] = "VK_LAYER_LUNARG_standard_validation";
                        break;
                    }
                }
            }
            if (layerNames[0])
            {
                instanceCreateInfo.enabledLayerCount = SLANG_COUNT_OF(layerNames);
                instanceCreateInfo.ppEnabledLayerNames = layerNames;
            }
        }
        uint32_t apiVersionsToTry[] = {VK_API_VERSION_1_2, VK_API_VERSION_1_1, VK_API_VERSION_1_0};
        for (auto apiVersion : apiVersionsToTry)
        {
            applicationInfo.apiVersion = apiVersion;
            if (m_api.vkCreateInstance(&instanceCreateInfo, nullptr, &instance) == VK_SUCCESS)
            {
                break;
            }
        }
    }
    else
    {
        instance = (VkInstance)handles[0].handleValue;
    }
    if (!instance)
        return SLANG_FAIL;
    SLANG_RETURN_ON_FAIL(m_api.initInstanceProcs(instance));
    if (useValidationLayer && m_api.vkCreateDebugReportCallbackEXT)
    {
        VkDebugReportFlagsEXT debugFlags =
            VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;

        VkDebugReportCallbackCreateInfoEXT debugCreateInfo = {
            VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT};
        debugCreateInfo.pfnCallback = &debugMessageCallback;
        debugCreateInfo.pUserData = this;
        debugCreateInfo.flags = debugFlags;

        SLANG_VK_RETURN_ON_FAIL(m_api.vkCreateDebugReportCallbackEXT(
            instance, &debugCreateInfo, nullptr, &m_debugReportCallback));
    }

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    Index selectedDeviceIndex = 0;
    if (handles[1].handleValue == 0)
    {
        uint32_t numPhysicalDevices = 0;
        SLANG_VK_RETURN_ON_FAIL(
            m_api.vkEnumeratePhysicalDevices(instance, &numPhysicalDevices, nullptr));

        List<VkPhysicalDevice> physicalDevices;
        physicalDevices.setCount(numPhysicalDevices);
        SLANG_VK_RETURN_ON_FAIL(m_api.vkEnumeratePhysicalDevices(
            instance, &numPhysicalDevices, physicalDevices.getBuffer()));

        if (m_desc.adapter)
        {
            selectedDeviceIndex = -1;

            String lowerAdapter = String(m_desc.adapter).toLower();

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

        physicalDevice = physicalDevices[selectedDeviceIndex];
    }
    else
    {
        physicalDevice = (VkPhysicalDevice)handles[1].handleValue;
    }

    SLANG_RETURN_ON_FAIL(m_api.initPhysicalDevice(physicalDevice));

    // Obtain the name of the selected adapter.
    {
        VkPhysicalDeviceProperties basicProps = {};
        m_api.vkGetPhysicalDeviceProperties(physicalDevice, &basicProps);
        m_adapterName = basicProps.deviceName;
        m_info.adapterName = m_adapterName.begin();
    }

    List<const char*> deviceExtensions;
    deviceExtensions.add(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    VkDeviceCreateInfo deviceCreateInfo = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
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

    // Compute timestamp frequency.
    m_info.timestampFrequency = uint64_t(1e9 / basicProps.limits.timestampPeriod);

    // Get the API version
    const uint32_t majorVersion = VK_VERSION_MAJOR(basicProps.apiVersion);
    const uint32_t minorVersion = VK_VERSION_MINOR(basicProps.apiVersion);

    auto& extendedFeatures = m_api.m_extendedFeatures;

    // API version check, can't use vkGetPhysicalDeviceProperties2 yet since this device might not
    // support it
    if (VK_MAKE_VERSION(majorVersion, minorVersion, 0) >= VK_API_VERSION_1_1 &&
        m_api.vkGetPhysicalDeviceProperties2 && m_api.vkGetPhysicalDeviceFeatures2)
    {
        // Get device features
        VkPhysicalDeviceFeatures2 deviceFeatures2 = {};
        deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;

        // Inline uniform block
        extendedFeatures.inlineUniformBlockFeatures.pNext = deviceFeatures2.pNext;
        deviceFeatures2.pNext = &extendedFeatures.inlineUniformBlockFeatures;

        // Buffer device address features
        extendedFeatures.bufferDeviceAddressFeatures.pNext = deviceFeatures2.pNext;
        deviceFeatures2.pNext = &extendedFeatures.bufferDeviceAddressFeatures;

        // Ray query features
        extendedFeatures.rayQueryFeatures.pNext = deviceFeatures2.pNext;
        deviceFeatures2.pNext = &extendedFeatures.rayQueryFeatures;

        // Ray tracing pipeline features
        extendedFeatures.rayTracingPipelineFeatures.pNext = deviceFeatures2.pNext;
        deviceFeatures2.pNext = &extendedFeatures.rayTracingPipelineFeatures;

        // Acceleration structure features
        extendedFeatures.accelerationStructureFeatures.pNext = deviceFeatures2.pNext;
        deviceFeatures2.pNext = &extendedFeatures.accelerationStructureFeatures;

        // Subgroup features
        extendedFeatures.shaderSubgroupExtendedTypeFeatures.pNext = deviceFeatures2.pNext;
        deviceFeatures2.pNext = &extendedFeatures.shaderSubgroupExtendedTypeFeatures;

        // Extended dynamic states
        extendedFeatures.extendedDynamicStateFeatures.pNext = deviceFeatures2.pNext;
        deviceFeatures2.pNext = &extendedFeatures.extendedDynamicStateFeatures;

        // Timeline Semaphore
        extendedFeatures.timelineFeatures.pNext = deviceFeatures2.pNext;
        deviceFeatures2.pNext = &extendedFeatures.timelineFeatures;

        // Float16
        extendedFeatures.float16Features.pNext = deviceFeatures2.pNext;
        deviceFeatures2.pNext = &extendedFeatures.float16Features;

        // 16-bit storage
        extendedFeatures.storage16BitFeatures.pNext = deviceFeatures2.pNext;
        deviceFeatures2.pNext = &extendedFeatures.storage16BitFeatures;

        // Atomic64
        extendedFeatures.atomicInt64Features.pNext = deviceFeatures2.pNext;
        deviceFeatures2.pNext = &extendedFeatures.atomicInt64Features;

        // robustness2 features
        extendedFeatures.robustness2Features.pNext = deviceFeatures2.pNext;
        deviceFeatures2.pNext = &extendedFeatures.robustness2Features;

        // Atomic Float
        // To detect atomic float we need
        // https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkPhysicalDeviceShaderAtomicFloatFeaturesEXT.html

        extendedFeatures.atomicFloatFeatures.pNext = deviceFeatures2.pNext;
        deviceFeatures2.pNext = &extendedFeatures.atomicFloatFeatures;

        m_api.vkGetPhysicalDeviceFeatures2(m_api.m_physicalDevice, &deviceFeatures2);

        if (deviceFeatures2.features.shaderResourceMinLod)
        {
            m_features.add("shader-resource-min-lod");
        }
        if (deviceFeatures2.features.shaderFloat64)
        {
            m_features.add("double");
        }
        if (deviceFeatures2.features.shaderInt64)
        {
            m_features.add("int64");
        }
        if (deviceFeatures2.features.shaderInt16)
        {
            m_features.add("int16");
        }
        // If we have float16 features then enable
        if (extendedFeatures.float16Features.shaderFloat16)
        {
            // Link into the creation features
            extendedFeatures.float16Features.pNext = (void*)deviceCreateInfo.pNext;
            deviceCreateInfo.pNext = &extendedFeatures.float16Features;

            // Add the Float16 extension
            deviceExtensions.add(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME);

            // We have half support
            m_features.add("half");
        }

        if (extendedFeatures.storage16BitFeatures.storageBuffer16BitAccess)
        {
            // Link into the creation features
            extendedFeatures.storage16BitFeatures.pNext = (void*)deviceCreateInfo.pNext;
            deviceCreateInfo.pNext = &extendedFeatures.storage16BitFeatures;

            // Add the 16-bit storage extension
            deviceExtensions.add(VK_KHR_16BIT_STORAGE_EXTENSION_NAME);

            // We have half support
            m_features.add("16-bit-storage");
        }

        if (extendedFeatures.atomicInt64Features.shaderBufferInt64Atomics)
        {
            // Link into the creation features
            extendedFeatures.atomicInt64Features.pNext = (void*)deviceCreateInfo.pNext;
            deviceCreateInfo.pNext = &extendedFeatures.atomicInt64Features;

            deviceExtensions.add(VK_KHR_SHADER_ATOMIC_INT64_EXTENSION_NAME);
            m_features.add("atomic-int64");
        }

        if (extendedFeatures.atomicFloatFeatures.shaderBufferFloat32AtomicAdd)
        {
            // Link into the creation features
            extendedFeatures.atomicFloatFeatures.pNext = (void*)deviceCreateInfo.pNext;
            deviceCreateInfo.pNext = &extendedFeatures.atomicFloatFeatures;

            deviceExtensions.add(VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME);
            m_features.add("atomic-float");
        }

        if (extendedFeatures.timelineFeatures.timelineSemaphore)
        {
            // Link into the creation features
            extendedFeatures.timelineFeatures.pNext = (void*)deviceCreateInfo.pNext;
            deviceCreateInfo.pNext = &extendedFeatures.timelineFeatures;
            deviceExtensions.add(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
            m_features.add("timeline-semaphore");
        }

        if (extendedFeatures.extendedDynamicStateFeatures.extendedDynamicState)
        {
            // Link into the creation features
            extendedFeatures.extendedDynamicStateFeatures.pNext = (void*)deviceCreateInfo.pNext;
            deviceCreateInfo.pNext = &extendedFeatures.extendedDynamicStateFeatures;
            deviceExtensions.add(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
            m_features.add("extended-dynamic-states");
        }

        if (extendedFeatures.shaderSubgroupExtendedTypeFeatures.shaderSubgroupExtendedTypes)
        {
            extendedFeatures.shaderSubgroupExtendedTypeFeatures.pNext =
                (void*)deviceCreateInfo.pNext;
            deviceCreateInfo.pNext = &extendedFeatures.shaderSubgroupExtendedTypeFeatures;
            deviceExtensions.add(VK_KHR_SHADER_SUBGROUP_EXTENDED_TYPES_EXTENSION_NAME);
            m_features.add("shader-subgroup-extended-types");
        }

        if (extendedFeatures.accelerationStructureFeatures.accelerationStructure)
        {
            extendedFeatures.accelerationStructureFeatures.pNext = (void*)deviceCreateInfo.pNext;
            deviceCreateInfo.pNext = &extendedFeatures.accelerationStructureFeatures;
            deviceExtensions.add(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
            deviceExtensions.add(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
            m_features.add("acceleration-structure");
        }

        if (extendedFeatures.rayTracingPipelineFeatures.rayTracingPipeline)
        {
            extendedFeatures.rayTracingPipelineFeatures.pNext = (void*)deviceCreateInfo.pNext;
            deviceCreateInfo.pNext = &extendedFeatures.rayTracingPipelineFeatures;
            deviceExtensions.add(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
            m_features.add("ray-tracing-pipeline");
        }

        if (extendedFeatures.rayQueryFeatures.rayQuery)
        {
            extendedFeatures.rayQueryFeatures.pNext = (void*)deviceCreateInfo.pNext;
            deviceCreateInfo.pNext = &extendedFeatures.rayQueryFeatures;
            deviceExtensions.add(VK_KHR_RAY_QUERY_EXTENSION_NAME);
            m_features.add("ray-query");
            m_features.add("ray-tracing");
            m_features.add("sm_6_6");
        }

        if (extendedFeatures.bufferDeviceAddressFeatures.bufferDeviceAddress)
        {
            extendedFeatures.bufferDeviceAddressFeatures.pNext = (void*)deviceCreateInfo.pNext;
            deviceCreateInfo.pNext = &extendedFeatures.bufferDeviceAddressFeatures;
            deviceExtensions.add(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);

            m_features.add("buffer-device-address");
        }

        if (extendedFeatures.inlineUniformBlockFeatures.inlineUniformBlock)
        {
            extendedFeatures.inlineUniformBlockFeatures.pNext = (void*)deviceCreateInfo.pNext;
            deviceCreateInfo.pNext = &extendedFeatures.inlineUniformBlockFeatures;
            deviceExtensions.add(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
            m_features.add("inline-uniform-block");
        }

        if (extendedFeatures.robustness2Features.nullDescriptor)
        {
            extendedFeatures.robustness2Features.pNext = (void*)deviceCreateInfo.pNext;
            deviceCreateInfo.pNext = &extendedFeatures.robustness2Features;
            deviceExtensions.add(VK_EXT_ROBUSTNESS_2_EXTENSION_NAME);
            m_features.add("robustness2");
        }

        VkPhysicalDeviceProperties2 extendedProps = {
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
        VkPhysicalDeviceRayTracingPipelinePropertiesKHR rtProps = {
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR};
        extendedProps.pNext = &rtProps;
        m_api.vkGetPhysicalDeviceProperties2(m_api.m_physicalDevice, &extendedProps);
        m_api.m_rtProperties = rtProps;

        uint32_t extensionCount = 0;
        m_api.vkEnumerateDeviceExtensionProperties(
            m_api.m_physicalDevice, NULL, &extensionCount, NULL);
        Slang::List<VkExtensionProperties> extensions;
        extensions.setCount(extensionCount);
        m_api.vkEnumerateDeviceExtensionProperties(
            m_api.m_physicalDevice, NULL, &extensionCount, extensions.getBuffer());

        HashSet<String> extensionNames;
        for (const auto& e : extensions)
        {
            extensionNames.Add(e.extensionName);
        }

        if (extensionNames.Contains("VK_KHR_external_memory"))
        {
            deviceExtensions.add(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME);
#if SLANG_WINDOWS_FAMILY
            if (extensionNames.Contains("VK_KHR_external_memory_win32"))
            {
                deviceExtensions.add(VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME);
            }
#endif
            m_features.add("external-memory");
        }
        if (extensionNames.Contains(VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME))
        {
            deviceExtensions.add(VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME);
            m_features.add("conservative-rasterization-3");
            m_features.add("conservative-rasterization-2");
            m_features.add("conservative-rasterization-1");
        }
        if (extensionNames.Contains(VK_EXT_DEBUG_REPORT_EXTENSION_NAME))
        {
            deviceExtensions.add(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
            if (extensionNames.Contains(VK_EXT_DEBUG_MARKER_EXTENSION_NAME))
            {
                deviceExtensions.add(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
            }
        }
        if (extensionNames.Contains(VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME))
        {
            deviceExtensions.add(VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME);
        }
    }
    if (m_api.m_module->isSoftware())
    {
        m_features.add("software-device");
    }
    else
    {
        m_features.add("hardware-device");
    }

    m_queueFamilyIndex = m_api.findQueue(VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT);
    assert(m_queueFamilyIndex >= 0);

    if (handles[2].handleValue == 0)
    {
        float queuePriority = 0.0f;
        VkDeviceQueueCreateInfo queueCreateInfo = {VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
        queueCreateInfo.queueFamilyIndex = m_queueFamilyIndex;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;

        deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;

        deviceCreateInfo.enabledExtensionCount = uint32_t(deviceExtensions.getCount());
        deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.getBuffer();

        if (m_api.vkCreateDevice(m_api.m_physicalDevice, &deviceCreateInfo, nullptr, &m_device) !=
            VK_SUCCESS)
            return SLANG_FAIL;
    }
    else
    {
        m_device = (VkDevice)handles[2].handleValue;
    }

    SLANG_RETURN_ON_FAIL(m_api.initDeviceProcs(m_device));

    return SLANG_OK;
}

SlangResult DeviceImpl::initialize(const Desc& desc)
{
    // Initialize device info.
    {
        m_info.apiName = "Vulkan";
        m_info.bindingStyle = BindingStyle::Vulkan;
        m_info.projectionStyle = ProjectionStyle::Vulkan;
        m_info.deviceType = DeviceType::Vulkan;
        static const float kIdentity[] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
        ::memcpy(m_info.identityProjectionMatrix, kIdentity, sizeof(kIdentity));
    }

    m_desc = desc;

    SLANG_RETURN_ON_FAIL(RendererBase::initialize(desc));
    SlangResult initDeviceResult = SLANG_OK;

    for (int forceSoftware = 0; forceSoftware <= 1; forceSoftware++)
    {
        initDeviceResult = m_module.init(forceSoftware != 0);
        if (initDeviceResult != SLANG_OK)
            continue;
        initDeviceResult = m_api.initGlobalProcs(m_module);
        if (initDeviceResult != SLANG_OK)
            continue;
        descriptorSetAllocator.m_api = &m_api;
        initDeviceResult = initVulkanInstanceAndDevice(
            desc.existingDeviceHandles.handles, ENABLE_VALIDATION_LAYER != 0);
        if (initDeviceResult == SLANG_OK)
            break;
    }
    SLANG_RETURN_ON_FAIL(initDeviceResult);

    {
        VkQueue queue;
        m_api.vkGetDeviceQueue(m_device, m_queueFamilyIndex, 0, &queue);
        SLANG_RETURN_ON_FAIL(m_deviceQueue.init(m_api, queue, m_queueFamilyIndex));
    }

    SLANG_RETURN_ON_FAIL(slangContext.initialize(
        desc.slang,
        SLANG_SPIRV,
        "sm_5_1",
        makeArray(slang::PreprocessorMacroDesc{"__VK__", "1"}).getView()));

    // Create default sampler.
    {
        VkSamplerCreateInfo samplerInfo = {VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
        samplerInfo.magFilter = VK_FILTER_NEAREST;
        samplerInfo.minFilter = VK_FILTER_NEAREST;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        samplerInfo.anisotropyEnable = VK_FALSE;
        samplerInfo.maxAnisotropy = 1;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;
        SLANG_VK_RETURN_ON_FAIL(
            m_api.vkCreateSampler(m_device, &samplerInfo, nullptr, &m_defaultSampler));
    }

    // Create empty frame buffer.
    {
        IFramebufferLayout::Desc layoutDesc = {};
        layoutDesc.renderTargetCount = 0;
        layoutDesc.depthStencil = nullptr;
        ComPtr<IFramebufferLayout> layout;
        SLANG_RETURN_ON_FAIL(createFramebufferLayout(layoutDesc, layout.writeRef()));
        IFramebuffer::Desc desc = {};
        desc.layout = layout;
        ComPtr<IFramebuffer> framebuffer;
        SLANG_RETURN_ON_FAIL(createFramebuffer(desc, framebuffer.writeRef()));
        m_emptyFramebuffer = static_cast<FramebufferImpl*>(framebuffer.get());
        m_emptyFramebuffer->m_renderer.breakStrongReference();
    }

    return SLANG_OK;
}

void DeviceImpl::waitForGpu() { m_deviceQueue.flushAndWait(); }

SLANG_NO_THROW const DeviceInfo& SLANG_MCALL DeviceImpl::getDeviceInfo() const { return m_info; }

Result DeviceImpl::createTransientResourceHeap(
    const ITransientResourceHeap::Desc& desc, ITransientResourceHeap** outHeap)
{
    RefPtr<TransientResourceHeapImpl> result = new TransientResourceHeapImpl();
    SLANG_RETURN_ON_FAIL(result->init(desc, this));
    returnComPtr(outHeap, result);
    return SLANG_OK;
}

Result DeviceImpl::createCommandQueue(const ICommandQueue::Desc& desc, ICommandQueue** outQueue)
{
    // Only support one queue for now.
    if (m_queueAllocCount != 0)
        return SLANG_FAIL;
    auto queueFamilyIndex = m_api.findQueue(VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT);
    VkQueue vkQueue;
    m_api.vkGetDeviceQueue(m_api.m_device, queueFamilyIndex, 0, &vkQueue);
    RefPtr<CommandQueueImpl> result = new CommandQueueImpl();
    result->init(this, vkQueue, queueFamilyIndex);
    returnComPtr(outQueue, result);
    m_queueAllocCount++;
    return SLANG_OK;
}

Result DeviceImpl::createSwapchain(
    const ISwapchain::Desc& desc, WindowHandle window, ISwapchain** outSwapchain)
{
#if !defined(SLANG_ENABLE_XLIB)
    if (window.type == WindowHandle::Type::XLibHandle)
    {
        return SLANG_FAIL;
    }
#endif

    RefPtr<SwapchainImpl> sc = new SwapchainImpl();
    SLANG_RETURN_ON_FAIL(sc->init(this, desc, window));
    returnComPtr(outSwapchain, sc);
    return SLANG_OK;
}

Result DeviceImpl::createFramebufferLayout(
    const IFramebufferLayout::Desc& desc, IFramebufferLayout** outLayout)
{
    RefPtr<FramebufferLayoutImpl> layout = new FramebufferLayoutImpl();
    SLANG_RETURN_ON_FAIL(layout->init(this, desc));
    returnComPtr(outLayout, layout);
    return SLANG_OK;
}

Result DeviceImpl::createRenderPassLayout(
    const IRenderPassLayout::Desc& desc, IRenderPassLayout** outRenderPassLayout)
{
    RefPtr<RenderPassLayoutImpl> result = new RenderPassLayoutImpl();
    SLANG_RETURN_ON_FAIL(result->init(this, desc));
    returnComPtr(outRenderPassLayout, result);
    return SLANG_OK;
}

Result DeviceImpl::createFramebuffer(const IFramebuffer::Desc& desc, IFramebuffer** outFramebuffer)
{
    RefPtr<FramebufferImpl> fb = new FramebufferImpl();
    SLANG_RETURN_ON_FAIL(fb->init(this, desc));
    returnComPtr(outFramebuffer, fb);
    return SLANG_OK;
}

VkImageAspectFlags getAspectMaskFromFormat(VkFormat format)
{
    switch (format)
    {
    case VK_FORMAT_D16_UNORM_S8_UINT:
    case VK_FORMAT_D24_UNORM_S8_UINT:
    case VK_FORMAT_D32_SFLOAT_S8_UINT:
        return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    case VK_FORMAT_D16_UNORM:
    case VK_FORMAT_D32_SFLOAT:
    case VK_FORMAT_X8_D24_UNORM_PACK32:
        return VK_IMAGE_ASPECT_DEPTH_BIT;
    case VK_FORMAT_S8_UINT:
        return VK_IMAGE_ASPECT_STENCIL_BIT;
    default:
        return VK_IMAGE_ASPECT_COLOR_BIT;
    }
}

SlangResult DeviceImpl::readTextureResource(
    ITextureResource* texture,
    ResourceState state,
    ISlangBlob** outBlob,
    size_t* outRowPitch,
    size_t* outPixelSize)
{
    auto textureImpl = static_cast<TextureResourceImpl*>(texture);
    RefPtr<ListBlob> blob = new ListBlob();

    auto desc = textureImpl->getDesc();
    auto width = desc->size.width;
    auto height = desc->size.height;
    FormatInfo sizeInfo;
    SLANG_RETURN_ON_FAIL(gfxGetFormatInfo(desc->format, &sizeInfo));
    size_t pixelSize = sizeInfo.blockSizeInBytes / sizeInfo.pixelsPerBlock;
    size_t rowPitch = width * pixelSize;

    List<TextureResource::Size> mipSizes;

    const int numMipMaps = desc->numMipLevels;
    auto arraySize = calcEffectiveArraySize(*desc);

    // Calculate how large the buffer has to be
    size_t bufferSize = 0;
    // Calculate how large an array entry is
    for (int j = 0; j < numMipMaps; ++j)
    {
        const TextureResource::Size mipSize = calcMipSize(desc->size, j);

        auto rowSizeInBytes = calcRowSize(desc->format, mipSize.width);
        auto numRows = calcNumRows(desc->format, mipSize.height);

        mipSizes.add(mipSize);

        bufferSize += (rowSizeInBytes * numRows) * mipSize.depth;
    }
    // Calculate the total size taking into account the array
    bufferSize *= arraySize;
    blob->m_data.setCount(Index(bufferSize));

    VKBufferHandleRAII staging;
    SLANG_RETURN_ON_FAIL(staging.init(
        m_api,
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));

    VkCommandBuffer commandBuffer = m_deviceQueue.getCommandBuffer();
    VkImage srcImage = textureImpl->m_image;
    VkImageLayout srcImageLayout = VulkanUtil::getImageLayoutFromState(state);

    size_t dstOffset = 0;
    for (int i = 0; i < arraySize; ++i)
    {
        for (Index j = 0; j < mipSizes.getCount(); ++j)
        {
            const auto& mipSize = mipSizes[j];

            auto rowSizeInBytes = calcRowSize(desc->format, mipSize.width);
            auto numRows = calcNumRows(desc->format, mipSize.height);

            VkBufferImageCopy region = {};

            region.bufferOffset = dstOffset;
            region.bufferRowLength = 0;
            region.bufferImageHeight = 0;

            region.imageSubresource.aspectMask = getAspectMaskFromFormat(VulkanUtil::getVkFormat(desc->format));
            region.imageSubresource.mipLevel = uint32_t(j);
            region.imageSubresource.baseArrayLayer = i;
            region.imageSubresource.layerCount = 1;
            region.imageOffset = {0, 0, 0};
            region.imageExtent = {
                uint32_t(mipSize.width), uint32_t(mipSize.height), uint32_t(mipSize.depth)};

            m_api.vkCmdCopyImageToBuffer(
                commandBuffer, srcImage, srcImageLayout, staging.m_buffer, 1, &region);

            dstOffset += rowSizeInBytes * numRows * mipSize.depth;
        }
    }

    m_deviceQueue.flushAndWait();

    // Write out the data from the buffer
    void* mappedData = nullptr;
    SLANG_RETURN_ON_FAIL(
        m_api.vkMapMemory(m_device, staging.m_memory, 0, bufferSize, 0, &mappedData));

    ::memcpy(blob->m_data.getBuffer(), mappedData, bufferSize);
    m_api.vkUnmapMemory(m_device, staging.m_memory);

    *outPixelSize = pixelSize;
    *outRowPitch = rowPitch;
    returnComPtr(outBlob, blob);
    return SLANG_OK;
}

SlangResult DeviceImpl::readBufferResource(
    IBufferResource* inBuffer, size_t offset, size_t size, ISlangBlob** outBlob)
{
    BufferResourceImpl* buffer = static_cast<BufferResourceImpl*>(inBuffer);

    RefPtr<ListBlob> blob = new ListBlob();
    blob->m_data.setCount(size);

    // create staging buffer
    VKBufferHandleRAII staging;

    SLANG_RETURN_ON_FAIL(staging.init(
        m_api,
        size,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));

    // Copy from real buffer to staging buffer
    VkCommandBuffer commandBuffer = m_deviceQueue.getCommandBuffer();

    VkBufferCopy copyInfo = {};
    copyInfo.size = size;
    copyInfo.srcOffset = offset;
    m_api.vkCmdCopyBuffer(commandBuffer, buffer->m_buffer.m_buffer, staging.m_buffer, 1, &copyInfo);

    m_deviceQueue.flushAndWait();

    // Write out the data from the buffer
    void* mappedData = nullptr;
    SLANG_RETURN_ON_FAIL(m_api.vkMapMemory(m_device, staging.m_memory, 0, size, 0, &mappedData));

    ::memcpy(blob->m_data.getBuffer(), mappedData, size);
    m_api.vkUnmapMemory(m_device, staging.m_memory);

    returnComPtr(outBlob, blob);
    return SLANG_OK;
}

Result DeviceImpl::getAccelerationStructurePrebuildInfo(
    const IAccelerationStructure::BuildInputs& buildInputs,
    IAccelerationStructure::PrebuildInfo* outPrebuildInfo)
{
    if (!m_api.vkGetAccelerationStructureBuildSizesKHR)
    {
        return SLANG_E_NOT_AVAILABLE;
    }
    VkAccelerationStructureBuildSizesInfoKHR sizeInfo = {
        VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR};
    AccelerationStructureBuildGeometryInfoBuilder geomInfoBuilder;
    SLANG_RETURN_ON_FAIL(geomInfoBuilder.build(buildInputs, getDebugCallback()));
    m_api.vkGetAccelerationStructureBuildSizesKHR(
        m_api.m_device,
        VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
        &geomInfoBuilder.buildInfo,
        geomInfoBuilder.primitiveCounts.getBuffer(),
        &sizeInfo);
    outPrebuildInfo->resultDataMaxSize = sizeInfo.accelerationStructureSize;
    outPrebuildInfo->scratchDataSize = sizeInfo.buildScratchSize;
    outPrebuildInfo->updateScratchDataSize = sizeInfo.updateScratchSize;
    return SLANG_OK;
}

Result DeviceImpl::createAccelerationStructure(
    const IAccelerationStructure::CreateDesc& desc, IAccelerationStructure** outAS)
{
    if (!m_api.vkCreateAccelerationStructureKHR)
    {
        return SLANG_E_NOT_AVAILABLE;
    }
    RefPtr<AccelerationStructureImpl> resultAS = new AccelerationStructureImpl();
    resultAS->m_offset = desc.offset;
    resultAS->m_size = desc.size;
    resultAS->m_buffer = static_cast<BufferResourceImpl*>(desc.buffer);
    resultAS->m_device = this;
    resultAS->m_desc.type = IResourceView::Type::AccelerationStructure;
    VkAccelerationStructureCreateInfoKHR createInfo = {
        VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR};
    createInfo.buffer = resultAS->m_buffer->m_buffer.m_buffer;
    createInfo.offset = desc.offset;
    createInfo.size = desc.size;
    switch (desc.kind)
    {
    case IAccelerationStructure::Kind::BottomLevel:
        createInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
        break;
    case IAccelerationStructure::Kind::TopLevel:
        createInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
        break;
    default:
        getDebugCallback()->handleMessage(
            DebugMessageType::Error,
            DebugMessageSource::Layer,
            "invalid value of IAccelerationStructure::Kind encountered in desc.kind");
        return SLANG_E_INVALID_ARG;
    }

    SLANG_VK_RETURN_ON_FAIL(m_api.vkCreateAccelerationStructureKHR(
        m_api.m_device, &createInfo, nullptr, &resultAS->m_vkHandle));
    returnComPtr(outAS, resultAS);
    return SLANG_OK;
}

void DeviceImpl::_transitionImageLayout(
    VkCommandBuffer commandBuffer,
    VkImage image,
    VkFormat format,
    const TextureResource::Desc& desc,
    VkImageLayout oldLayout,
    VkImageLayout newLayout)
{
    if (oldLayout == newLayout)
        return;

    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;

    barrier.subresourceRange.aspectMask = getAspectMaskFromFormat(format);

    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = desc.numMipLevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
    barrier.srcAccessMask = calcAccessFlagsFromImageLayout(oldLayout);
    barrier.dstAccessMask = calcAccessFlagsFromImageLayout(newLayout);

    VkPipelineStageFlags sourceStage = calcPipelineStageFlagsFromImageLayout(oldLayout);
    VkPipelineStageFlags destinationStage = calcPipelineStageFlagsFromImageLayout(newLayout);

    m_api.vkCmdPipelineBarrier(
        commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

uint32_t DeviceImpl::getQueueFamilyIndex(ICommandQueue::QueueType queueType)
{
    switch (queueType)
    {
    case ICommandQueue::QueueType::Graphics:
    default:
        return m_queueFamilyIndex;
    }
}

void DeviceImpl::_transitionImageLayout(
    VkImage image,
    VkFormat format,
    const TextureResource::Desc& desc,
    VkImageLayout oldLayout,
    VkImageLayout newLayout)
{
    VkCommandBuffer commandBuffer = m_deviceQueue.getCommandBuffer();
    _transitionImageLayout(commandBuffer, image, format, desc, oldLayout, newLayout);
}

Result DeviceImpl::getTextureAllocationInfo(
    const ITextureResource::Desc& descIn, size_t* outSize, size_t* outAlignment)
{
    TextureResource::Desc desc = fixupTextureDesc(descIn);

    const VkFormat format = VulkanUtil::getVkFormat(desc.format);
    if (format == VK_FORMAT_UNDEFINED)
    {
        assert(!"Unhandled image format");
        return SLANG_FAIL;
    }
    const int arraySize = calcEffectiveArraySize(desc);

    VkImageCreateInfo imageInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    switch (desc.type)
    {
    case IResource::Type::Texture1D:
        {
            imageInfo.imageType = VK_IMAGE_TYPE_1D;
            imageInfo.extent = VkExtent3D{uint32_t(descIn.size.width), 1, 1};
            break;
        }
    case IResource::Type::Texture2D:
        {
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.extent =
                VkExtent3D{uint32_t(descIn.size.width), uint32_t(descIn.size.height), 1};
            break;
        }
    case IResource::Type::TextureCube:
        {
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.extent =
                VkExtent3D{uint32_t(descIn.size.width), uint32_t(descIn.size.height), 1};
            imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
            break;
        }
    case IResource::Type::Texture3D:
        {
            // Can't have an array and 3d texture
            assert(desc.arraySize <= 1);

            imageInfo.imageType = VK_IMAGE_TYPE_3D;
            imageInfo.extent = VkExtent3D{
                uint32_t(descIn.size.width),
                uint32_t(descIn.size.height),
                uint32_t(descIn.size.depth)};
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
    imageInfo.usage = _calcImageUsageFlags(desc.allowedStates, desc.memoryType, nullptr);
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    imageInfo.samples = (VkSampleCountFlagBits)desc.sampleDesc.numSamples;

    VkImage image;
    SLANG_VK_RETURN_ON_FAIL(m_api.vkCreateImage(m_device, &imageInfo, nullptr, &image));

    VkMemoryRequirements memRequirements;
    m_api.vkGetImageMemoryRequirements(m_device, image, &memRequirements);

    *outSize = (size_t)memRequirements.size;
    *outAlignment = (size_t)memRequirements.alignment;

    m_api.vkDestroyImage(m_device, image, nullptr);
    return SLANG_OK;
}

Result DeviceImpl::getTextureRowAlignment(size_t* outAlignment)
{
    *outAlignment = 1;
    return SLANG_OK;
}

Result DeviceImpl::createTextureResource(
    const ITextureResource::Desc& descIn,
    const ITextureResource::SubresourceData* initData,
    ITextureResource** outResource)
{
    TextureResource::Desc desc = fixupTextureDesc(descIn);

    const VkFormat format = VulkanUtil::getVkFormat(desc.format);
    if (format == VK_FORMAT_UNDEFINED)
    {
        assert(!"Unhandled image format");
        return SLANG_FAIL;
    }

    const int arraySize = calcEffectiveArraySize(desc);

    RefPtr<TextureResourceImpl> texture(new TextureResourceImpl(desc, this));
    texture->m_vkformat = format;
    // Create the image

    VkImageCreateInfo imageInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    switch (desc.type)
    {
    case IResource::Type::Texture1D:
        {
            imageInfo.imageType = VK_IMAGE_TYPE_1D;
            imageInfo.extent = VkExtent3D{uint32_t(descIn.size.width), 1, 1};
            break;
        }
    case IResource::Type::Texture2D:
        {
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.extent =
                VkExtent3D{uint32_t(descIn.size.width), uint32_t(descIn.size.height), 1};
            break;
        }
    case IResource::Type::TextureCube:
        {
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.extent =
                VkExtent3D{uint32_t(descIn.size.width), uint32_t(descIn.size.height), 1};
            imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
            break;
        }
    case IResource::Type::Texture3D:
        {
            // Can't have an array and 3d texture
            assert(desc.arraySize <= 1);

            imageInfo.imageType = VK_IMAGE_TYPE_3D;
            imageInfo.extent = VkExtent3D{
                uint32_t(descIn.size.width),
                uint32_t(descIn.size.height),
                uint32_t(descIn.size.depth)};
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
    imageInfo.usage = _calcImageUsageFlags(desc.allowedStates, desc.memoryType, initData);
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    imageInfo.samples = (VkSampleCountFlagBits)desc.sampleDesc.numSamples;

    VkExternalMemoryImageCreateInfo externalMemoryImageCreateInfo = {
        VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO};
#if SLANG_WINDOWS_FAMILY
    VkExternalMemoryHandleTypeFlags extMemoryHandleType =
        VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;
    if (descIn.isShared)
    {
        externalMemoryImageCreateInfo.pNext = nullptr;
        externalMemoryImageCreateInfo.handleTypes = extMemoryHandleType;
        imageInfo.pNext = &externalMemoryImageCreateInfo;
    }
#endif
    SLANG_VK_RETURN_ON_FAIL(m_api.vkCreateImage(m_device, &imageInfo, nullptr, &texture->m_image));

    VkMemoryRequirements memRequirements;
    m_api.vkGetImageMemoryRequirements(m_device, texture->m_image, &memRequirements);

    // Allocate the memory
    VkMemoryPropertyFlags reqMemoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    int memoryTypeIndex =
        m_api.findMemoryTypeIndex(memRequirements.memoryTypeBits, reqMemoryProperties);
    assert(memoryTypeIndex >= 0);

    VkMemoryPropertyFlags actualMemoryProperites =
        m_api.m_deviceMemoryProperties.memoryTypes[memoryTypeIndex].propertyFlags;
    VkMemoryAllocateInfo allocInfo = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = memoryTypeIndex;
#if SLANG_WINDOWS_FAMILY
    VkExportMemoryWin32HandleInfoKHR exportMemoryWin32HandleInfo = {
        VK_STRUCTURE_TYPE_EXPORT_MEMORY_WIN32_HANDLE_INFO_KHR};
    VkExportMemoryAllocateInfoKHR exportMemoryAllocateInfo = {
        VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO_KHR};
    if (descIn.isShared)
    {
        exportMemoryWin32HandleInfo.pNext = nullptr;
        exportMemoryWin32HandleInfo.pAttributes = nullptr;
        exportMemoryWin32HandleInfo.dwAccess =
            DXGI_SHARED_RESOURCE_READ | DXGI_SHARED_RESOURCE_WRITE;
        exportMemoryWin32HandleInfo.name = NULL;

        exportMemoryAllocateInfo.pNext =
            extMemoryHandleType & VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT_KHR
                ? &exportMemoryWin32HandleInfo
                : nullptr;
        exportMemoryAllocateInfo.handleTypes = extMemoryHandleType;
        allocInfo.pNext = &exportMemoryAllocateInfo;
    }
#endif
    SLANG_VK_RETURN_ON_FAIL(
        m_api.vkAllocateMemory(m_device, &allocInfo, nullptr, &texture->m_imageMemory));

    // Bind the memory to the image
    m_api.vkBindImageMemory(m_device, texture->m_image, texture->m_imageMemory, 0);

    VKBufferHandleRAII uploadBuffer;
    if (initData)
    {
        List<TextureResource::Size> mipSizes;

        VkCommandBuffer commandBuffer = m_deviceQueue.getCommandBuffer();

        const int numMipMaps = desc.numMipLevels;

        // Calculate how large the buffer has to be
        size_t bufferSize = 0;
        // Calculate how large an array entry is
        for (int j = 0; j < numMipMaps; ++j)
        {
            const TextureResource::Size mipSize = calcMipSize(desc.size, j);

            auto rowSizeInBytes = calcRowSize(desc.format, mipSize.width);
            auto numRows = calcNumRows(desc.format, mipSize.height);

            mipSizes.add(mipSize);

            bufferSize += (rowSizeInBytes * numRows) * mipSize.depth;
        }

        // Calculate the total size taking into account the array
        bufferSize *= arraySize;

        SLANG_RETURN_ON_FAIL(uploadBuffer.init(
            m_api,
            bufferSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));

        assert(mipSizes.getCount() == numMipMaps);

        // Copy into upload buffer
        {
            int subResourceCounter = 0;

            uint8_t* dstData;
            m_api.vkMapMemory(m_device, uploadBuffer.m_memory, 0, bufferSize, 0, (void**)&dstData);
            uint8_t* dstDataStart;
            dstDataStart = dstData;

            size_t dstSubresourceOffset = 0;
            for (int i = 0; i < arraySize; ++i)
            {
                for (Index j = 0; j < mipSizes.getCount(); ++j)
                {
                    const auto& mipSize = mipSizes[j];

                    int subResourceIndex = subResourceCounter++;
                    auto initSubresource = initData[subResourceIndex];

                    const ptrdiff_t srcRowStride = (ptrdiff_t)initSubresource.strideY;
                    const ptrdiff_t srcLayerStride = (ptrdiff_t)initSubresource.strideZ;

                    auto dstRowSizeInBytes = calcRowSize(desc.format, mipSize.width);
                    auto numRows = calcNumRows(desc.format, mipSize.height);
                    auto dstLayerSizeInBytes = dstRowSizeInBytes * numRows;

                    const uint8_t* srcLayer = (const uint8_t*)initSubresource.data;
                    uint8_t* dstLayer = dstData + dstSubresourceOffset;

                    for (int k = 0; k < mipSize.depth; k++)
                    {
                        const uint8_t* srcRow = srcLayer;
                        uint8_t* dstRow = dstLayer;

                        for (uint32_t l = 0; l < numRows; l++)
                        {
                            ::memcpy(dstRow, srcRow, dstRowSizeInBytes);

                            dstRow += dstRowSizeInBytes;
                            srcRow += srcRowStride;
                        }

                        dstLayer += dstLayerSizeInBytes;
                        srcLayer += srcLayerStride;
                    }

                    dstSubresourceOffset += dstLayerSizeInBytes * mipSize.depth;
                }
            }

            m_api.vkUnmapMemory(m_device, uploadBuffer.m_memory);
        }

        _transitionImageLayout(
            texture->m_image,
            format,
            *texture->getDesc(),
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        {
            size_t srcOffset = 0;
            for (int i = 0; i < arraySize; ++i)
            {
                for (Index j = 0; j < mipSizes.getCount(); ++j)
                {
                    const auto& mipSize = mipSizes[j];

                    auto rowSizeInBytes = calcRowSize(desc.format, mipSize.width);
                    auto numRows = calcNumRows(desc.format, mipSize.height);

                    // https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/VkBufferImageCopy.html
                    // bufferRowLength and bufferImageHeight specify the data in buffer memory as a
                    // subregion of a larger two- or three-dimensional image, and control the
                    // addressing calculations of data in buffer memory. If either of these values
                    // is zero, that aspect of the buffer memory is considered to be tightly packed
                    // according to the imageExtent.

                    VkBufferImageCopy region = {};

                    region.bufferOffset = srcOffset;
                    region.bufferRowLength = 0; // rowSizeInBytes;
                    region.bufferImageHeight = 0;

                    region.imageSubresource.aspectMask = getAspectMaskFromFormat(format);
                    region.imageSubresource.mipLevel = uint32_t(j);
                    region.imageSubresource.baseArrayLayer = i;
                    region.imageSubresource.layerCount = 1;
                    region.imageOffset = {0, 0, 0};
                    region.imageExtent = {
                        uint32_t(mipSize.width), uint32_t(mipSize.height), uint32_t(mipSize.depth)};

                    // Do the copy (do all depths in a single go)
                    m_api.vkCmdCopyBufferToImage(
                        commandBuffer,
                        uploadBuffer.m_buffer,
                        texture->m_image,
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                        1,
                        &region);

                    // Next
                    srcOffset += rowSizeInBytes * numRows * mipSize.depth;
                }
            }
        }
        auto defaultLayout = VulkanUtil::getImageLayoutFromState(desc.defaultState);
        _transitionImageLayout(
            texture->m_image,
            format,
            *texture->getDesc(),
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            defaultLayout);
    }
    else
    {
        auto defaultLayout = VulkanUtil::getImageLayoutFromState(desc.defaultState);
        if (defaultLayout != VK_IMAGE_LAYOUT_UNDEFINED)
        {
            _transitionImageLayout(
                texture->m_image,
                format,
                *texture->getDesc(),
                VK_IMAGE_LAYOUT_UNDEFINED,
                defaultLayout);
        }
    }
    m_deviceQueue.flushAndWait();
    returnComPtr(outResource, texture);
    return SLANG_OK;
}

Result DeviceImpl::createBufferResource(
    const IBufferResource::Desc& descIn, const void* initData, IBufferResource** outResource)
{
    BufferResource::Desc desc = fixupBufferDesc(descIn);

    const size_t bufferSize = desc.sizeInBytes;

    VkMemoryPropertyFlags reqMemoryProperties = 0;

    VkBufferUsageFlags usage = _calcBufferUsageFlags(desc.allowedStates);
    if (m_api.m_extendedFeatures.bufferDeviceAddressFeatures.bufferDeviceAddress)
    {
        usage |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    }
    if (desc.allowedStates.contains(ResourceState::ShaderResource) &&
        m_api.m_extendedFeatures.accelerationStructureFeatures.accelerationStructure)
    {
        usage |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
    }
    if (initData)
    {
        usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    }

    if (desc.allowedStates.contains(ResourceState::ConstantBuffer) ||
        desc.memoryType == MemoryType::Upload || desc.memoryType == MemoryType::ReadBack)
    {
        reqMemoryProperties =
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    }

    RefPtr<BufferResourceImpl> buffer(new BufferResourceImpl(desc, this));
    if (desc.isShared)
    {
        SLANG_RETURN_ON_FAIL(buffer->m_buffer.init(
            m_api,
            desc.sizeInBytes,
            usage,
            reqMemoryProperties,
            desc.isShared,
            VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT));
    }
    else
    {
        SLANG_RETURN_ON_FAIL(
            buffer->m_buffer.init(m_api, desc.sizeInBytes, usage, reqMemoryProperties));
    }

    if (initData)
    {
        if (desc.memoryType == MemoryType::DeviceLocal)
        {
            SLANG_RETURN_ON_FAIL(buffer->m_uploadBuffer.init(
                m_api,
                bufferSize,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
            // Copy into staging buffer
            void* mappedData = nullptr;
            SLANG_VK_CHECK(m_api.vkMapMemory(
                m_device, buffer->m_uploadBuffer.m_memory, 0, bufferSize, 0, &mappedData));
            ::memcpy(mappedData, initData, bufferSize);
            m_api.vkUnmapMemory(m_device, buffer->m_uploadBuffer.m_memory);

            // Copy from staging buffer to real buffer
            VkCommandBuffer commandBuffer = m_deviceQueue.getCommandBuffer();

            VkBufferCopy copyInfo = {};
            copyInfo.size = bufferSize;
            m_api.vkCmdCopyBuffer(
                commandBuffer,
                buffer->m_uploadBuffer.m_buffer,
                buffer->m_buffer.m_buffer,
                1,
                &copyInfo);
            m_deviceQueue.flush();
        }
        else
        {
            // Copy into mapped buffer directly
            void* mappedData = nullptr;
            SLANG_VK_CHECK(m_api.vkMapMemory(
                m_device, buffer->m_buffer.m_memory, 0, bufferSize, 0, &mappedData));
            ::memcpy(mappedData, initData, bufferSize);
            m_api.vkUnmapMemory(m_device, buffer->m_buffer.m_memory);
        }
    }

    returnComPtr(outResource, buffer);
    return SLANG_OK;
}

Result DeviceImpl::createBufferFromNativeHandle(
    InteropHandle handle, const IBufferResource::Desc& srcDesc, IBufferResource** outResource)
{
    RefPtr<BufferResourceImpl> buffer(new BufferResourceImpl(srcDesc, this));

    if (handle.api == InteropHandleAPI::Vulkan)
    {
        buffer->m_buffer.m_buffer = (VkBuffer)handle.handleValue;
    }
    else
    {
        return SLANG_FAIL;
    }

    returnComPtr(outResource, buffer);
    return SLANG_OK;
}

Result DeviceImpl::createSamplerState(ISamplerState::Desc const& desc, ISamplerState** outSampler)
{
    VkSamplerCreateInfo samplerInfo = {VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};

    samplerInfo.magFilter = VulkanUtil::translateFilterMode(desc.minFilter);
    samplerInfo.minFilter = VulkanUtil::translateFilterMode(desc.magFilter);

    samplerInfo.addressModeU = VulkanUtil::translateAddressingMode(desc.addressU);
    samplerInfo.addressModeV = VulkanUtil::translateAddressingMode(desc.addressV);
    samplerInfo.addressModeW = VulkanUtil::translateAddressingMode(desc.addressW);

    samplerInfo.anisotropyEnable = desc.maxAnisotropy > 1;
    samplerInfo.maxAnisotropy = (float)desc.maxAnisotropy;

    // TODO: support translation of border color...
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = desc.reductionOp == TextureReductionOp::Comparison;
    samplerInfo.compareOp = VulkanUtil::translateComparisonFunc(desc.comparisonFunc);
    samplerInfo.mipmapMode = VulkanUtil::translateMipFilterMode(desc.mipFilter);
    samplerInfo.minLod = Math::Max(0.0f, desc.minLOD);
    samplerInfo.maxLod = Math::Clamp(desc.maxLOD, samplerInfo.minLod, VK_LOD_CLAMP_NONE);

    VkSampler sampler;
    SLANG_VK_RETURN_ON_FAIL(m_api.vkCreateSampler(m_device, &samplerInfo, nullptr, &sampler));

    RefPtr<SamplerStateImpl> samplerImpl = new SamplerStateImpl(this);
    samplerImpl->m_sampler = sampler;
    returnComPtr(outSampler, samplerImpl);
    return SLANG_OK;
}

Result DeviceImpl::createTextureView(
    ITextureResource* texture, IResourceView::Desc const& desc, IResourceView** outView)
{
    auto resourceImpl = static_cast<TextureResourceImpl*>(texture);
    RefPtr<TextureResourceViewImpl> view = new TextureResourceViewImpl(this);
    view->m_texture = resourceImpl;
    view->m_desc = desc;
    if (!texture)
    {
        view->m_view = VK_NULL_HANDLE;
        returnComPtr(outView, view);
        return SLANG_OK;
    }

    bool isArray = desc.subresourceRange.layerCount > 1;
    VkImageViewCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.flags = 0;
    createInfo.format = gfxIsTypelessFormat(texture->getDesc()->format)
                            ? VulkanUtil::getVkFormat(desc.format)
                            : resourceImpl->m_vkformat;
    createInfo.image = resourceImpl->m_image;
    createInfo.components = VkComponentMapping{
        VK_COMPONENT_SWIZZLE_R,
        VK_COMPONENT_SWIZZLE_G,
        VK_COMPONENT_SWIZZLE_B,
        VK_COMPONENT_SWIZZLE_A};
    switch (resourceImpl->getType())
    {
    case IResource::Type::Texture1D:
        createInfo.viewType = isArray ? VK_IMAGE_VIEW_TYPE_1D_ARRAY : VK_IMAGE_VIEW_TYPE_1D;
        break;
    case IResource::Type::Texture2D:
        createInfo.viewType = isArray ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
        break;
    case IResource::Type::Texture3D:
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
        break;
    case IResource::Type::TextureCube:
        createInfo.viewType = isArray ? VK_IMAGE_VIEW_TYPE_CUBE_ARRAY : VK_IMAGE_VIEW_TYPE_CUBE;
        break;
    default:
        SLANG_UNIMPLEMENTED_X("Unknown Texture type.");
        break;
    }

    createInfo.subresourceRange.aspectMask = getAspectMaskFromFormat(resourceImpl->m_vkformat);

    createInfo.subresourceRange.baseArrayLayer = desc.subresourceRange.baseArrayLayer;
    createInfo.subresourceRange.baseMipLevel = desc.subresourceRange.mipLevel;
    createInfo.subresourceRange.layerCount = desc.subresourceRange.layerCount == 0
                                                 ? VK_REMAINING_ARRAY_LAYERS
                                                 : desc.subresourceRange.layerCount;
    createInfo.subresourceRange.levelCount = desc.subresourceRange.mipLevelCount == 0
                                                 ? VK_REMAINING_MIP_LEVELS
                                                 : desc.subresourceRange.mipLevelCount;
    switch (desc.type)
    {
    case IResourceView::Type::DepthStencil:
        view->m_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        createInfo.subresourceRange.levelCount = 1;
        break;
    case IResourceView::Type::RenderTarget:
        view->m_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        createInfo.subresourceRange.levelCount = 1;
        break;
    case IResourceView::Type::ShaderResource:
        view->m_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        break;
    case IResourceView::Type::UnorderedAccess:
        view->m_layout = VK_IMAGE_LAYOUT_GENERAL;
        break;
    default:
        SLANG_UNIMPLEMENTED_X("Unknown TextureViewDesc type.");
        break;
    }
    m_api.vkCreateImageView(m_device, &createInfo, nullptr, &view->m_view);
    returnComPtr(outView, view);
    return SLANG_OK;
}

Result DeviceImpl::getFormatSupportedResourceStates(Format format, ResourceStateSet* outStates)
{
    // TODO: Add variables to VkDevice to track supported surface presentable formats

    VkFormat vkFormat = VulkanUtil::getVkFormat(format);

    VkFormatProperties supportedProperties = {};
    m_api.vkGetPhysicalDeviceFormatProperties(
        m_api.m_physicalDevice, vkFormat, &supportedProperties);

    HashSet<VkFormat> presentableFormats;
    // TODO: enable this once we have VK_GOOGLE_surfaceless_query.
#if 0
    List<VkSurfaceFormatKHR> surfaceFormats;

    uint32_t surfaceFormatCount = 0;
    m_api.vkGetPhysicalDeviceSurfaceFormatsKHR(
        m_api.m_physicalDevice, VK_NULL_HANDLE, &surfaceFormatCount, nullptr);

    surfaceFormats.setCount(surfaceFormatCount);
    m_api.vkGetPhysicalDeviceSurfaceFormatsKHR(m_api.m_physicalDevice, VK_NULL_HANDLE, &surfaceFormatCount, surfaceFormats.getBuffer());
    for (auto surfaceFormat : surfaceFormats)
    {
        presentableFormats.Add(surfaceFormat.format);
    }
#else
    // Until we have a solution to query presentable formats without needing a surface,
    // hard code presentable formats that is supported by most drivers.
    presentableFormats.Add(VK_FORMAT_R8G8B8A8_UNORM);
    presentableFormats.Add(VK_FORMAT_B8G8R8A8_UNORM);
    presentableFormats.Add(VK_FORMAT_R8G8B8A8_SRGB);
    presentableFormats.Add(VK_FORMAT_B8G8R8A8_SRGB);
#endif

    ResourceStateSet allowedStates;
    // TODO: Currently only supports VK_IMAGE_TILING_OPTIMAL
    auto imageFeatures = supportedProperties.optimalTilingFeatures;
    auto bufferFeatures = supportedProperties.bufferFeatures;
    // PreInitialized - Only supported for VK_IMAGE_TILING_LINEAR
    // VertexBuffer
    if (bufferFeatures & VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT)
        allowedStates.add(ResourceState::VertexBuffer);
    // IndexBuffer - Without extensions, Vulkan only supports two formats for index buffers.
    switch (format)
    {
    case Format::R32_UINT:
    case Format::R16_UINT:
        allowedStates.add(ResourceState::IndexBuffer);
        break;
    default:
        break;
    }
    // ConstantBuffer
    allowedStates.add(ResourceState::ConstantBuffer);
    // StreamOutput - TODO: Requires VK_EXT_transform_feedback
    // ShaderResource
    if (imageFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)
        allowedStates.add(ResourceState::ShaderResource);
    if (bufferFeatures & VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT)
        allowedStates.add(ResourceState::ShaderResource);
    // UnorderedAccess
    if (imageFeatures &
        (VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT | VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT))
        allowedStates.add(ResourceState::UnorderedAccess);
    if (bufferFeatures & (VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT |
                          VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT))
        allowedStates.add(ResourceState::UnorderedAccess);
    // RenderTarget
    if (imageFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT)
        allowedStates.add(ResourceState::RenderTarget);
    // DepthRead, DepthWrite
    if (imageFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
    {
        allowedStates.add(ResourceState::DepthRead);
        allowedStates.add(ResourceState::DepthWrite);
    }
    // Present
    if (presentableFormats.Contains(vkFormat))
        allowedStates.add(ResourceState::Present);
    // IndirectArgument
    allowedStates.add(ResourceState::IndirectArgument);
    // CopySource, ResolveSource
    if (imageFeatures & VK_FORMAT_FEATURE_TRANSFER_SRC_BIT)
    {
        allowedStates.add(ResourceState::CopySource);
        allowedStates.add(ResourceState::ResolveSource);
    }
    // CopyDestination, ResolveDestination
    if (imageFeatures & VK_FORMAT_FEATURE_TRANSFER_DST_BIT)
    {
        allowedStates.add(ResourceState::CopyDestination);
        allowedStates.add(ResourceState::ResolveDestination);
    }
    // AccelerationStructure
    if (bufferFeatures & VK_FORMAT_FEATURE_ACCELERATION_STRUCTURE_VERTEX_BUFFER_BIT_KHR)
    {
        allowedStates.add(ResourceState::AccelerationStructure);
        allowedStates.add(ResourceState::AccelerationStructureBuildInput);
    }

    *outStates = allowedStates;
    return SLANG_OK;
}

Result DeviceImpl::createBufferView(
    IBufferResource* buffer,
    IBufferResource* counterBuffer,
    IResourceView::Desc const& desc,
    IResourceView** outView)
{
    auto resourceImpl = (BufferResourceImpl*)buffer;

    // TODO: These should come from the `ResourceView::Desc`
    auto stride = desc.bufferElementSize;
    if (stride == 0)
    {
        if (desc.format == Format::Unknown)
        {
            stride = 1;
        }
        else
        {
            FormatInfo info;
            gfxGetFormatInfo(desc.format, &info);
            stride = info.blockSizeInBytes;
            assert(info.pixelsPerBlock == 1);
        }
    }
    VkDeviceSize offset = (VkDeviceSize)desc.bufferRange.firstElement * stride;
    VkDeviceSize size = desc.bufferRange.elementCount == 0
                            ? (buffer ? resourceImpl->getDesc()->sizeInBytes : 0)
                            : (VkDeviceSize)desc.bufferRange.elementCount * stride;

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

    switch (desc.type)
    {
    default:
        assert(!"unhandled");
        return SLANG_FAIL;

    case IResourceView::Type::UnorderedAccess:
    case IResourceView::Type::ShaderResource:
        // Is this a formatted view?
        //
        if (desc.format == Format::Unknown)
        {
            // Buffer usage that doesn't involve formatting doesn't
            // require a view in Vulkan.
            RefPtr<PlainBufferResourceViewImpl> viewImpl = new PlainBufferResourceViewImpl(this);
            viewImpl->m_buffer = resourceImpl;
            viewImpl->offset = offset;
            viewImpl->size = size;
            viewImpl->m_desc = desc;

            returnComPtr(outView, viewImpl);
            return SLANG_OK;
        }
        //
        // If the view is formatted, then we need to handle
        // it just like we would for a "sampled" buffer:
        //
        // FALLTHROUGH
        {
            VkBufferViewCreateInfo info = {VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO};

            VkBufferView view = VK_NULL_HANDLE;

            if (buffer)
            {
                info.format = VulkanUtil::getVkFormat(desc.format);
                info.buffer = resourceImpl->m_buffer.m_buffer;
                info.offset = offset;
                info.range = size;

                SLANG_VK_RETURN_ON_FAIL(m_api.vkCreateBufferView(m_device, &info, nullptr, &view));
            }

            RefPtr<TexelBufferResourceViewImpl> viewImpl = new TexelBufferResourceViewImpl(this);
            viewImpl->m_buffer = resourceImpl;
            viewImpl->m_view = view;
            viewImpl->m_desc = desc;

            returnComPtr(outView, viewImpl);
            return SLANG_OK;
        }
        break;
    }
}

Result DeviceImpl::createInputLayout(IInputLayout::Desc const& desc, IInputLayout** outLayout)
{
    RefPtr<InputLayoutImpl> layout(new InputLayoutImpl);

    List<VkVertexInputAttributeDescription>& dstAttributes = layout->m_attributeDescs;
    List<VkVertexInputBindingDescription>& dstStreams = layout->m_streamDescs;

    auto elements = desc.inputElements;
    Int numElements = desc.inputElementCount;

    auto srcVertexStreams = desc.vertexStreams;
    Int vertexStreamCount = desc.vertexStreamCount;

    dstAttributes.setCount(numElements);
    dstStreams.setCount(vertexStreamCount);

    for (Int i = 0; i < vertexStreamCount; i++)
    {
        auto& dstStream = dstStreams[i];
        auto& srcStream = srcVertexStreams[i];
        dstStream.stride = srcStream.stride;
        dstStream.binding = (uint32_t)i;
        dstStream.inputRate = (srcStream.slotClass == InputSlotClass::PerInstance)
                                  ? VK_VERTEX_INPUT_RATE_INSTANCE
                                  : VK_VERTEX_INPUT_RATE_VERTEX;
    }

    for (Int i = 0; i < numElements; ++i)
    {
        const InputElementDesc& srcDesc = elements[i];
        auto streamIndex = srcDesc.bufferSlotIndex;

        VkVertexInputAttributeDescription& dstDesc = dstAttributes[i];

        dstDesc.location = uint32_t(i);
        dstDesc.binding = (uint32_t)streamIndex;
        dstDesc.format = VulkanUtil::getVkFormat(srcDesc.format);
        if (dstDesc.format == VK_FORMAT_UNDEFINED)
        {
            return SLANG_FAIL;
        }

        dstDesc.offset = uint32_t(srcDesc.offset);
    }

    // Work out the overall size
    returnComPtr(outLayout, layout);
    return SLANG_OK;
}

Result DeviceImpl::createProgram(
    const IShaderProgram::Desc& desc, IShaderProgram** outProgram, ISlangBlob** outDiagnosticBlob)
{
    RefPtr<ShaderProgramImpl> shaderProgram = new ShaderProgramImpl(this);
    shaderProgram->init(desc);

    m_deviceObjectsWithPotentialBackReferences.add(shaderProgram);

    RootShaderObjectLayout::create(
        this,
        shaderProgram->linkedProgram,
        shaderProgram->linkedProgram->getLayout(),
        shaderProgram->m_rootObjectLayout.writeRef());

    returnComPtr(outProgram, shaderProgram);
    return SLANG_OK;
}

Result DeviceImpl::createShaderObjectLayout(
    slang::TypeLayoutReflection* typeLayout, ShaderObjectLayoutBase** outLayout)
{
    RefPtr<ShaderObjectLayoutImpl> layout;
    SLANG_RETURN_ON_FAIL(
        ShaderObjectLayoutImpl::createForElementType(this, typeLayout, layout.writeRef()));
    returnRefPtrMove(outLayout, layout);
    return SLANG_OK;
}

Result DeviceImpl::createShaderObject(ShaderObjectLayoutBase* layout, IShaderObject** outObject)
{
    RefPtr<ShaderObjectImpl> shaderObject;
    SLANG_RETURN_ON_FAIL(ShaderObjectImpl::create(
        this, static_cast<ShaderObjectLayoutImpl*>(layout), shaderObject.writeRef()));
    returnComPtr(outObject, shaderObject);
    return SLANG_OK;
}

Result DeviceImpl::createMutableShaderObject(
    ShaderObjectLayoutBase* layout, IShaderObject** outObject)
{
    auto layoutImpl = static_cast<ShaderObjectLayoutImpl*>(layout);

    RefPtr<MutableShaderObjectImpl> result = new MutableShaderObjectImpl();
    SLANG_RETURN_ON_FAIL(result->init(this, layoutImpl));
    returnComPtr(outObject, result);

    return SLANG_OK;
}

Result DeviceImpl::createMutableRootShaderObject(IShaderProgram* program, IShaderObject** outObject)
{
    RefPtr<MutableRootShaderObject> result =
        new MutableRootShaderObject(this, static_cast<ShaderProgramBase*>(program));
    returnComPtr(outObject, result);
    return SLANG_OK;
}

Result DeviceImpl::createShaderTable(const IShaderTable::Desc& desc, IShaderTable** outShaderTable)
{
    RefPtr<ShaderTableImpl> result = new ShaderTableImpl();
    result->m_device = this;
    result->init(desc);
    returnComPtr(outShaderTable, result);
    return SLANG_OK;
}

Result DeviceImpl::createGraphicsPipelineState(
    const GraphicsPipelineStateDesc& inDesc, IPipelineState** outState)
{
    GraphicsPipelineStateDesc desc = inDesc;
    RefPtr<PipelineStateImpl> pipelineStateImpl = new PipelineStateImpl(this);
    pipelineStateImpl->init(desc);
    pipelineStateImpl->establishStrongDeviceReference();
    m_deviceObjectsWithPotentialBackReferences.add(pipelineStateImpl);
    returnComPtr(outState, pipelineStateImpl);

    return SLANG_OK;
}

Result DeviceImpl::createComputePipelineState(
    const ComputePipelineStateDesc& inDesc, IPipelineState** outState)
{
    ComputePipelineStateDesc desc = inDesc;
    RefPtr<PipelineStateImpl> pipelineStateImpl = new PipelineStateImpl(this);
    pipelineStateImpl->init(desc);
    m_deviceObjectsWithPotentialBackReferences.add(pipelineStateImpl);
    pipelineStateImpl->establishStrongDeviceReference();
    returnComPtr(outState, pipelineStateImpl);
    return SLANG_OK;
}

Result DeviceImpl::createRayTracingPipelineState(
    const RayTracingPipelineStateDesc& desc, IPipelineState** outState)
{
    RefPtr<RayTracingPipelineStateImpl> pipelineStateImpl = new RayTracingPipelineStateImpl(this);
    pipelineStateImpl->init(desc);
    m_deviceObjectsWithPotentialBackReferences.add(pipelineStateImpl);
    pipelineStateImpl->establishStrongDeviceReference();
    returnComPtr(outState, pipelineStateImpl);
    return SLANG_OK;
}

Result DeviceImpl::createQueryPool(const IQueryPool::Desc& desc, IQueryPool** outPool)
{
    RefPtr<QueryPoolImpl> result = new QueryPoolImpl();
    SLANG_RETURN_ON_FAIL(result->init(desc, this));
    returnComPtr(outPool, result);
    return SLANG_OK;
}

Result DeviceImpl::createFence(const IFence::Desc& desc, IFence** outFence)
{
    RefPtr<FenceImpl> fence = new FenceImpl(this);
    SLANG_RETURN_ON_FAIL(fence->init(desc));
    returnComPtr(outFence, fence);
    return SLANG_OK;
}

Result DeviceImpl::waitForFences(
    uint32_t fenceCount, IFence** fences, uint64_t* fenceValues, bool waitForAll, uint64_t timeout)
{
    ShortList<VkSemaphore> semaphores;
    for (uint32_t i = 0; i < fenceCount; ++i)
    {
        auto fenceImpl = static_cast<FenceImpl*>(fences[i]);
        semaphores.add(fenceImpl->m_semaphore);
    }
    VkSemaphoreWaitInfo waitInfo;
    waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
    waitInfo.pNext = NULL;
    waitInfo.flags = 0;
    waitInfo.semaphoreCount = 1;
    waitInfo.pSemaphores = semaphores.getArrayView().getBuffer();
    waitInfo.pValues = fenceValues;
    auto result = m_api.vkWaitSemaphores(m_api.m_device, &waitInfo, timeout);
    if (result == VK_TIMEOUT)
        return SLANG_E_TIME_OUT;
    return result == VK_SUCCESS ? SLANG_OK : SLANG_FAIL;
}

void TransientResourceHeapImpl::advanceFence()
{
    m_fenceIndex++;
    if (m_fenceIndex >= m_fences.getCount())
    {
        m_fences.setCount(m_fenceIndex + 1);
        VkFenceCreateInfo fenceCreateInfo = {};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        m_device->m_api.vkCreateFence(
            m_device->m_api.m_device, &fenceCreateInfo, nullptr, &m_fences[m_fenceIndex]);
    }
}

Result TransientResourceHeapImpl::init(const ITransientResourceHeap::Desc& desc, DeviceImpl* device)
{
    Super::init(
        desc,
        (uint32_t)device->m_api.m_deviceProperties.limits.minUniformBufferOffsetAlignment,
        device);

    m_descSetAllocator.m_api = &device->m_api;

    VkCommandPoolCreateInfo poolCreateInfo = {};
    poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolCreateInfo.queueFamilyIndex =
        device->getQueueFamilyIndex(ICommandQueue::QueueType::Graphics);
    device->m_api.vkCreateCommandPool(
        device->m_api.m_device, &poolCreateInfo, nullptr, &m_commandPool);

    advanceFence();
    return SLANG_OK;
}

TransientResourceHeapImpl::~TransientResourceHeapImpl()
{
    m_commandBufferPool = decltype(m_commandBufferPool)();
    m_device->m_api.vkDestroyCommandPool(m_device->m_api.m_device, m_commandPool, nullptr);
    for (auto fence : m_fences)
    {
        m_device->m_api.vkDestroyFence(m_device->m_api.m_device, fence, nullptr);
    }
    m_descSetAllocator.close();
}

Result TransientResourceHeapImpl::createCommandBuffer(ICommandBuffer** outCmdBuffer)
{
    if (m_commandBufferAllocId < (uint32_t)m_commandBufferPool.getCount())
    {
        auto result = m_commandBufferPool[m_commandBufferAllocId];
        result->beginCommandBuffer();
        m_commandBufferAllocId++;
        returnComPtr(outCmdBuffer, result);
        return SLANG_OK;
    }

    RefPtr<CommandBufferImpl> commandBuffer = new CommandBufferImpl();
    SLANG_RETURN_ON_FAIL(commandBuffer->init(m_device, m_commandPool, this));
    m_commandBufferPool.add(commandBuffer);
    m_commandBufferAllocId++;
    returnComPtr(outCmdBuffer, commandBuffer);
    return SLANG_OK;
}

Result TransientResourceHeapImpl::synchronizeAndReset()
{
    m_commandBufferAllocId = 0;
    auto& api = m_device->m_api;
    if (api.vkWaitForFences(
            api.m_device, (uint32_t)m_fences.getCount(), m_fences.getBuffer(), 1, UINT64_MAX) !=
        VK_SUCCESS)
    {
        return SLANG_FAIL;
    }
    api.vkResetCommandPool(api.m_device, m_commandPool, 0);
    m_descSetAllocator.reset();
    m_fenceIndex = 0;
    Super::reset();
    return SLANG_OK;
}

Result VKBufferHandleRAII::init(
    const VulkanApi& api,
    size_t bufferSize,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags reqMemoryProperties,
    bool isShared,
    VkExternalMemoryHandleTypeFlagsKHR extMemHandleType)
{
    assert(!isInitialized());

    m_api = &api;
    m_memory = VK_NULL_HANDLE;
    m_buffer = VK_NULL_HANDLE;

    VkBufferCreateInfo bufferCreateInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bufferCreateInfo.size = bufferSize;
    bufferCreateInfo.usage = usage;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkExternalMemoryBufferCreateInfo externalMemoryBufferCreateInfo = {
        VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_BUFFER_CREATE_INFO};
    if (isShared)
    {
        externalMemoryBufferCreateInfo.handleTypes = extMemHandleType;
        bufferCreateInfo.pNext = &externalMemoryBufferCreateInfo;
    }

    SLANG_VK_CHECK(api.vkCreateBuffer(api.m_device, &bufferCreateInfo, nullptr, &m_buffer));

    VkMemoryRequirements memoryReqs = {};
    api.vkGetBufferMemoryRequirements(api.m_device, m_buffer, &memoryReqs);

    int memoryTypeIndex = api.findMemoryTypeIndex(memoryReqs.memoryTypeBits, reqMemoryProperties);
    assert(memoryTypeIndex >= 0);

    VkMemoryPropertyFlags actualMemoryProperites =
        api.m_deviceMemoryProperties.memoryTypes[memoryTypeIndex].propertyFlags;
    VkMemoryAllocateInfo allocateInfo = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    allocateInfo.allocationSize = memoryReqs.size;
    allocateInfo.memoryTypeIndex = memoryTypeIndex;
#if SLANG_WINDOWS_FAMILY
    VkExportMemoryWin32HandleInfoKHR exportMemoryWin32HandleInfo = {
        VK_STRUCTURE_TYPE_EXPORT_MEMORY_WIN32_HANDLE_INFO_KHR};
    VkExportMemoryAllocateInfoKHR exportMemoryAllocateInfo = {
        VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO_KHR};
    if (isShared)
    {
        exportMemoryWin32HandleInfo.pNext = nullptr;
        exportMemoryWin32HandleInfo.pAttributes = nullptr;
        exportMemoryWin32HandleInfo.dwAccess =
            DXGI_SHARED_RESOURCE_READ | DXGI_SHARED_RESOURCE_WRITE;
        exportMemoryWin32HandleInfo.name = NULL;

        exportMemoryAllocateInfo.pNext =
            extMemHandleType & VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT_KHR
                ? &exportMemoryWin32HandleInfo
                : nullptr;
        exportMemoryAllocateInfo.handleTypes = extMemHandleType;
        allocateInfo.pNext = &exportMemoryAllocateInfo;
    }
#endif
    VkMemoryAllocateFlagsInfo flagInfo = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO};
    if (usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
    {
        flagInfo.deviceMask = 1;
        flagInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;

        flagInfo.pNext = allocateInfo.pNext;
        allocateInfo.pNext = &flagInfo;
    }

    SLANG_VK_CHECK(api.vkAllocateMemory(api.m_device, &allocateInfo, nullptr, &m_memory));
    SLANG_VK_CHECK(api.vkBindBufferMemory(api.m_device, m_buffer, m_memory, 0));

    return SLANG_OK;
}

BufferResourceImpl::BufferResourceImpl(const IBufferResource::Desc& desc, DeviceImpl* renderer)
    : Parent(desc)
    , m_renderer(renderer)
{
    assert(renderer);
}

BufferResourceImpl::~BufferResourceImpl()
{
    if (sharedHandle.handleValue != 0)
    {
#if SLANG_WINDOWS_FAMILY
        CloseHandle((HANDLE)sharedHandle.handleValue);
#endif
    }
}

DeviceAddress BufferResourceImpl::getDeviceAddress()
{
    if (!m_buffer.m_api->vkGetBufferDeviceAddress)
        return 0;
    VkBufferDeviceAddressInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    info.buffer = m_buffer.m_buffer;
    return (DeviceAddress)m_buffer.m_api->vkGetBufferDeviceAddress(m_buffer.m_api->m_device, &info);
}

Result BufferResourceImpl::getNativeResourceHandle(InteropHandle* outHandle)
{
    outHandle->handleValue = (uint64_t)m_buffer.m_buffer;
    outHandle->api = InteropHandleAPI::Vulkan;
    return SLANG_OK;
}

Result BufferResourceImpl::getSharedHandle(InteropHandle* outHandle)
{
    // Check if a shared handle already exists for this resource.
    if (sharedHandle.handleValue != 0)
    {
        *outHandle = sharedHandle;
        return SLANG_OK;
    }

    // If a shared handle doesn't exist, create one and store it.
#if SLANG_WINDOWS_FAMILY
    VkMemoryGetWin32HandleInfoKHR info = {};
    info.sType = VK_STRUCTURE_TYPE_MEMORY_GET_WIN32_HANDLE_INFO_KHR;
    info.pNext = nullptr;
    info.memory = m_buffer.m_memory;
    info.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;

    auto api = m_buffer.m_api;
    PFN_vkGetMemoryWin32HandleKHR vkCreateSharedHandle;
    vkCreateSharedHandle = api->vkGetMemoryWin32HandleKHR;
    if (!vkCreateSharedHandle)
    {
        return SLANG_FAIL;
    }
    SLANG_VK_RETURN_ON_FAIL(
        vkCreateSharedHandle(api->m_device, &info, (HANDLE*)&outHandle->handleValue));
#endif
    outHandle->api = InteropHandleAPI::Vulkan;
    return SLANG_OK;
}

Result BufferResourceImpl::map(MemoryRange* rangeToRead, void** outPointer)
{
    SLANG_UNUSED(rangeToRead);
    auto api = m_buffer.m_api;
    SLANG_VK_RETURN_ON_FAIL(
        api->vkMapMemory(api->m_device, m_buffer.m_memory, 0, VK_WHOLE_SIZE, 0, outPointer));
    return SLANG_OK;
}

Result BufferResourceImpl::unmap(MemoryRange* writtenRange)
{
    SLANG_UNUSED(writtenRange);
    auto api = m_buffer.m_api;
    api->vkUnmapMemory(api->m_device, m_buffer.m_memory);
    return SLANG_OK;
}

Result BufferResourceImpl::setDebugName(const char* name)
{
    Parent::setDebugName(name);
    auto api = m_buffer.m_api;
    if (api->vkDebugMarkerSetObjectNameEXT)
    {
        VkDebugMarkerObjectNameInfoEXT nameDesc = {};
        nameDesc.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT;
        nameDesc.object = (uint64_t)m_buffer.m_buffer;
        nameDesc.objectType = VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT;
        nameDesc.pObjectName = name;
        api->vkDebugMarkerSetObjectNameEXT(api->m_device, &nameDesc);
    }
    return SLANG_OK;
}

FenceImpl::FenceImpl(DeviceImpl* device)
    : m_device(device)
{}

FenceImpl::~FenceImpl()
{
    if (m_semaphore)
    {
        m_device->m_api.vkDestroySemaphore(m_device->m_api.m_device, m_semaphore, nullptr);
    }
}

Result FenceImpl::init(const IFence::Desc& desc)
{
    if (!m_device->m_api.m_extendedFeatures.timelineFeatures.timelineSemaphore)
        return SLANG_E_NOT_AVAILABLE;

    VkSemaphoreTypeCreateInfo timelineCreateInfo;
    timelineCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
    timelineCreateInfo.pNext = nullptr;
    timelineCreateInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
    timelineCreateInfo.initialValue = desc.initialValue;

    VkSemaphoreCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    createInfo.pNext = &timelineCreateInfo;
    createInfo.flags = 0;

    SLANG_VK_RETURN_ON_FAIL(m_device->m_api.vkCreateSemaphore(
        m_device->m_api.m_device, &createInfo, nullptr, &m_semaphore));

    return SLANG_OK;
}

Result FenceImpl::getCurrentValue(uint64_t* outValue)
{
    SLANG_VK_RETURN_ON_FAIL(m_device->m_api.vkGetSemaphoreCounterValue(
        m_device->m_api.m_device, m_semaphore, outValue));
    return SLANG_OK;
}

Result FenceImpl::setCurrentValue(uint64_t value)
{
    uint64_t currentValue = 0;
    SLANG_VK_RETURN_ON_FAIL(m_device->m_api.vkGetSemaphoreCounterValue(
        m_device->m_api.m_device, m_semaphore, &currentValue));
    if (currentValue < value)
    {
        VkSemaphoreSignalInfo signalInfo;
        signalInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO;
        signalInfo.pNext = NULL;
        signalInfo.semaphore = m_semaphore;
        signalInfo.value = value;

        SLANG_VK_RETURN_ON_FAIL(
            m_device->m_api.vkSignalSemaphore(m_device->m_api.m_device, &signalInfo));
    }
    return SLANG_OK;
}

Result FenceImpl::getSharedHandle(InteropHandle* outHandle)
{
    // Check if a shared handle already exists.
    if (sharedHandle.handleValue != 0)
    {
        *outHandle = sharedHandle;
        return SLANG_OK;
    }

#if SLANG_WINDOWS_FAMILY
    VkSemaphoreGetWin32HandleInfoKHR handleInfo = {
        VK_STRUCTURE_TYPE_SEMAPHORE_GET_WIN32_HANDLE_INFO_KHR};
    handleInfo.pNext = nullptr;
    handleInfo.semaphore = m_semaphore;
    handleInfo.handleType = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT;

    SLANG_VK_RETURN_ON_FAIL(m_device->m_api.vkGetSemaphoreWin32HandleKHR(
        m_device->m_api.m_device, &handleInfo, (HANDLE*)&outHandle->handleValue));
#endif
    sharedHandle.api = InteropHandleAPI::Vulkan;
    return SLANG_OK;
}

Result FenceImpl::getNativeHandle(InteropHandle* outNativeHandle)
{
    outNativeHandle->handleValue = 0;
    return SLANG_FAIL;
}

TextureResourceImpl::TextureResourceImpl(const Desc& desc, DeviceImpl* device)
    : Parent(desc)
    , m_device(device)
{}

TextureResourceImpl::~TextureResourceImpl()
{
    auto& vkAPI = m_device->m_api;
    if (!m_isWeakImageReference)
    {
        vkAPI.vkFreeMemory(vkAPI.m_device, m_imageMemory, nullptr);
        vkAPI.vkDestroyImage(vkAPI.m_device, m_image, nullptr);
    }
    if (sharedHandle.handleValue != 0)
    {
#if SLANG_WINDOWS_FAMILY
        CloseHandle((HANDLE)sharedHandle.handleValue);
#endif
    }
}

Result TextureResourceImpl::getNativeResourceHandle(InteropHandle* outHandle)
{
    outHandle->handleValue = (uint64_t)m_image;
    outHandle->api = InteropHandleAPI::Vulkan;
    return SLANG_OK;
}

Result TextureResourceImpl::getSharedHandle(InteropHandle* outHandle)
{
    // Check if a shared handle already exists for this resource.
    if (sharedHandle.handleValue != 0)
    {
        *outHandle = sharedHandle;
        return SLANG_OK;
    }

    // If a shared handle doesn't exist, create one and store it.
#if SLANG_WINDOWS_FAMILY
    VkMemoryGetWin32HandleInfoKHR info = {};
    info.sType = VK_STRUCTURE_TYPE_MEMORY_GET_WIN32_HANDLE_INFO_KHR;
    info.pNext = nullptr;
    info.memory = m_imageMemory;
    info.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;

    auto& api = m_device->m_api;
    PFN_vkGetMemoryWin32HandleKHR vkCreateSharedHandle;
    vkCreateSharedHandle = api.vkGetMemoryWin32HandleKHR;
    if (!vkCreateSharedHandle)
    {
        return SLANG_FAIL;
    }
    SLANG_RETURN_ON_FAIL(
        vkCreateSharedHandle(m_device->m_device, &info, (HANDLE*)&outHandle->handleValue) !=
        VK_SUCCESS);
#endif
    outHandle->api = InteropHandleAPI::Vulkan;
    return SLANG_OK;
}
Result TextureResourceImpl::setDebugName(const char* name)
{
    Parent::setDebugName(name);
    auto& api = m_device->m_api;
    if (api.vkDebugMarkerSetObjectNameEXT)
    {
        VkDebugMarkerObjectNameInfoEXT nameDesc = {};
        nameDesc.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT;
        nameDesc.object = (uint64_t)m_image;
        nameDesc.objectType = VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT;
        nameDesc.pObjectName = name;
        api.vkDebugMarkerSetObjectNameEXT(api.m_device, &nameDesc);
    }
    return SLANG_OK;
}

SamplerStateImpl::SamplerStateImpl(DeviceImpl* device)
    : m_device(device)
{}

SamplerStateImpl::~SamplerStateImpl()
{
    m_device->m_api.vkDestroySampler(m_device->m_api.m_device, m_sampler, nullptr);
}

Result SamplerStateImpl::getNativeHandle(InteropHandle* outHandle)
{
    outHandle->api = InteropHandleAPI::Vulkan;
    outHandle->handleValue = (uint64_t)(m_sampler);
    return SLANG_OK;
}

TextureResourceViewImpl::~TextureResourceViewImpl()
{
    m_device->m_api.vkDestroyImageView(m_device->m_api.m_device, m_view, nullptr);
}

Result TextureResourceViewImpl::getNativeHandle(InteropHandle* outHandle)
{
    outHandle->api = InteropHandleAPI::Vulkan;
    outHandle->handleValue = (uint64_t)(m_view);
    return SLANG_OK;
}

TexelBufferResourceViewImpl::TexelBufferResourceViewImpl(DeviceImpl* device)
    : ResourceViewImpl(ViewType::TexelBuffer, device)
{}

TexelBufferResourceViewImpl::~TexelBufferResourceViewImpl()
{
    m_device->m_api.vkDestroyBufferView(m_device->m_api.m_device, m_view, nullptr);
}

Result TexelBufferResourceViewImpl::getNativeHandle(InteropHandle* outHandle)
{
    outHandle->api = InteropHandleAPI::Vulkan;
    outHandle->handleValue = (uint64_t)(m_view);
    return SLANG_OK;
}

PlainBufferResourceViewImpl::PlainBufferResourceViewImpl(DeviceImpl* device)
    : ResourceViewImpl(ViewType::PlainBuffer, device)
{}

Result PlainBufferResourceViewImpl::getNativeHandle(InteropHandle* outHandle)
{
    return m_buffer->getNativeResourceHandle(outHandle);
}

DeviceAddress AccelerationStructureImpl::getDeviceAddress()
{
    return m_buffer->getDeviceAddress() + m_offset;
}

Result AccelerationStructureImpl::getNativeHandle(InteropHandle* outHandle)
{
    outHandle->api = InteropHandleAPI::Vulkan;
    outHandle->handleValue = (uint64_t)(m_vkHandle);
    return SLANG_OK;
}

AccelerationStructureImpl::~AccelerationStructureImpl()
{
    if (m_device)
    {
        m_device->m_api.vkDestroyAccelerationStructureKHR(
            m_device->m_api.m_device, m_vkHandle, nullptr);
    }
}

FramebufferLayoutImpl::~FramebufferLayoutImpl()
{
    m_renderer->m_api.vkDestroyRenderPass(m_renderer->m_api.m_device, m_renderPass, nullptr);
}

Result FramebufferLayoutImpl::init(DeviceImpl* renderer, const IFramebufferLayout::Desc& desc)
{
    m_renderer = renderer;
    m_renderTargetCount = desc.renderTargetCount;
    // Create render pass.
    int numAttachments = m_renderTargetCount;
    m_hasDepthStencilAttachment = (desc.depthStencil != nullptr);
    if (m_hasDepthStencilAttachment)
    {
        numAttachments++;
    }
    // We need extra space if we have depth buffer
    m_attachmentDescs.setCount(numAttachments);
    for (uint32_t i = 0; i < desc.renderTargetCount; ++i)
    {
        auto& renderTarget = desc.renderTargets[i];
        VkAttachmentDescription& dst = m_attachmentDescs[i];

        dst.flags = 0;
        dst.format = VulkanUtil::getVkFormat(renderTarget.format);
        if (renderTarget.format == Format::Unknown)
            dst.format = VK_FORMAT_R8G8B8A8_UNORM;
        dst.samples = (VkSampleCountFlagBits)renderTarget.sampleCount;

        // The following load/store/layout settings does not matter.
        // In FramebufferLayout we just need a "compatible" render pass that
        // can be used to create a framebuffer. A framebuffer created
        // with this render pass setting can be used with actual render passes
        // that has a different loadOp/storeOp/layout setting.
        dst.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        dst.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        dst.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        dst.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        dst.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        dst.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        m_sampleCount = Math::Max(dst.samples, m_sampleCount);
    }

    if (desc.depthStencil)
    {
        VkAttachmentDescription& dst = m_attachmentDescs[desc.renderTargetCount];
        dst.flags = 0;
        dst.format = VulkanUtil::getVkFormat(desc.depthStencil->format);
        dst.samples = (VkSampleCountFlagBits)desc.depthStencil->sampleCount;
        dst.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        dst.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        dst.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        dst.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
        dst.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        dst.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        m_sampleCount = Math::Max(dst.samples, m_sampleCount);
    }

    Array<VkAttachmentReference, kMaxRenderTargets>& colorReferences = m_colorReferences;
    colorReferences.setCount(desc.renderTargetCount);
    for (uint32_t i = 0; i < desc.renderTargetCount; ++i)
    {
        VkAttachmentReference& dst = colorReferences[i];
        dst.attachment = i;
        dst.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }

    m_depthReference = VkAttachmentReference{};
    m_depthReference.attachment = desc.renderTargetCount;
    m_depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpassDesc = {};
    subpassDesc.flags = 0;
    subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDesc.inputAttachmentCount = 0u;
    subpassDesc.pInputAttachments = nullptr;
    subpassDesc.colorAttachmentCount = desc.renderTargetCount;
    subpassDesc.pColorAttachments = colorReferences.getBuffer();
    subpassDesc.pResolveAttachments = nullptr;
    subpassDesc.pDepthStencilAttachment = m_hasDepthStencilAttachment ? &m_depthReference : nullptr;
    subpassDesc.preserveAttachmentCount = 0u;
    subpassDesc.pPreserveAttachments = nullptr;

    VkRenderPassCreateInfo renderPassCreateInfo = {};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.attachmentCount = numAttachments;
    renderPassCreateInfo.pAttachments = m_attachmentDescs.getBuffer();
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpassDesc;
    SLANG_VK_RETURN_ON_FAIL(m_renderer->m_api.vkCreateRenderPass(
        m_renderer->m_api.m_device, &renderPassCreateInfo, nullptr, &m_renderPass));
    return SLANG_OK;
}

IRenderPassLayout* RenderPassLayoutImpl::getInterface(const Guid& guid)
{
    if (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_IRenderPassLayout)
        return static_cast<IRenderPassLayout*>(this);
    return nullptr;
}

RenderPassLayoutImpl::~RenderPassLayoutImpl()
{
    m_renderer->m_api.vkDestroyRenderPass(m_renderer->m_api.m_device, m_renderPass, nullptr);
}

Result RenderPassLayoutImpl::init(DeviceImpl* renderer, const IRenderPassLayout::Desc& desc)
{
    m_renderer = renderer;

    // Create render pass using load/storeOp and layouts info from `desc`.
    auto framebufferLayout = static_cast<FramebufferLayoutImpl*>(desc.framebufferLayout);
    assert(desc.renderTargetCount == framebufferLayout->m_renderTargetCount);

    // We need extra space if we have depth buffer
    Array<VkAttachmentDescription, kMaxAttachments> attachmentDescs;
    attachmentDescs = framebufferLayout->m_attachmentDescs;
    for (uint32_t i = 0; i < desc.renderTargetCount; ++i)
    {
        VkAttachmentDescription& dst = attachmentDescs[i];
        auto access = desc.renderTargetAccess[i];
        // Fill in loadOp/storeOp and layout from desc.
        dst.loadOp = translateLoadOp(access.loadOp);
        dst.storeOp = translateStoreOp(access.storeOp);
        dst.stencilLoadOp = translateLoadOp(access.stencilLoadOp);
        dst.stencilStoreOp = translateStoreOp(access.stencilStoreOp);
        dst.initialLayout = VulkanUtil::mapResourceStateToLayout(access.initialState);
        dst.finalLayout = VulkanUtil::mapResourceStateToLayout(access.finalState);
    }

    if (framebufferLayout->m_hasDepthStencilAttachment)
    {
        VkAttachmentDescription& dst = attachmentDescs[desc.renderTargetCount];
        auto access = *desc.depthStencilAccess;
        dst.loadOp = translateLoadOp(access.loadOp);
        dst.storeOp = translateStoreOp(access.storeOp);
        dst.stencilLoadOp = translateLoadOp(access.stencilLoadOp);
        dst.stencilStoreOp = translateStoreOp(access.stencilStoreOp);
        dst.initialLayout = VulkanUtil::mapResourceStateToLayout(access.initialState);
        dst.finalLayout = VulkanUtil::mapResourceStateToLayout(access.finalState);
    }

    VkSubpassDescription subpassDesc = {};
    subpassDesc.flags = 0;
    subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDesc.inputAttachmentCount = 0u;
    subpassDesc.pInputAttachments = nullptr;
    subpassDesc.colorAttachmentCount = desc.renderTargetCount;
    subpassDesc.pColorAttachments = framebufferLayout->m_colorReferences.getBuffer();
    subpassDesc.pResolveAttachments = nullptr;
    subpassDesc.pDepthStencilAttachment = framebufferLayout->m_hasDepthStencilAttachment
                                              ? &framebufferLayout->m_depthReference
                                              : nullptr;
    subpassDesc.preserveAttachmentCount = 0u;
    subpassDesc.pPreserveAttachments = nullptr;

    VkRenderPassCreateInfo renderPassCreateInfo = {};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.attachmentCount = (uint32_t)attachmentDescs.getCount();
    renderPassCreateInfo.pAttachments = attachmentDescs.getBuffer();
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpassDesc;
    SLANG_VK_RETURN_ON_FAIL(m_renderer->m_api.vkCreateRenderPass(
        m_renderer->m_api.m_device, &renderPassCreateInfo, nullptr, &m_renderPass));
    return SLANG_OK;
}

FramebufferImpl::~FramebufferImpl()
{
    m_renderer->m_api.vkDestroyFramebuffer(m_renderer->m_api.m_device, m_handle, nullptr);
}

Result FramebufferImpl::init(DeviceImpl* renderer, const IFramebuffer::Desc& desc)
{
    m_renderer = renderer;
    uint32_t layerCount = 0;

    auto dsv = desc.depthStencilView ? static_cast<TextureResourceViewImpl*>(desc.depthStencilView)
                                     : nullptr;
    // Get frame dimensions from attachments.
    if (dsv)
    {
        // If we have a depth attachment, get frame size from there.
        auto size = dsv->m_texture->getDesc()->size;
        auto viewDesc = dsv->getViewDesc();
        m_width = getMipLevelSize(viewDesc->subresourceRange.mipLevel, size.width);
        m_height = getMipLevelSize(viewDesc->subresourceRange.mipLevel, size.height);
        layerCount = viewDesc->subresourceRange.layerCount;
    }
    else if (desc.renderTargetCount)
    {
        // If we don't have a depth attachment, then we must have at least
        // one color attachment. Get frame dimension from there.
        auto viewImpl = static_cast<TextureResourceViewImpl*>(desc.renderTargetViews[0]);
        auto resourceDesc = viewImpl->m_texture->getDesc();
        auto viewDesc = viewImpl->getViewDesc();
        auto size = resourceDesc->size;
        m_width = getMipLevelSize(viewDesc->subresourceRange.mipLevel, size.width);
        m_height = getMipLevelSize(viewDesc->subresourceRange.mipLevel, size.height);
        layerCount = viewDesc->subresourceRange.layerCount;
    }
    else
    {
        m_width = 1;
        m_height = 1;
        layerCount = 1;
    }
    if (layerCount == 0)
        layerCount = 1;
    // Create render pass.
    int numAttachments = desc.renderTargetCount;
    if (desc.depthStencilView)
        numAttachments++;
    Array<VkImageView, kMaxAttachments> imageViews;
    imageViews.setCount(numAttachments);
    renderTargetViews.setCount(desc.renderTargetCount);
    for (uint32_t i = 0; i < desc.renderTargetCount; ++i)
    {
        auto resourceView = static_cast<TextureResourceViewImpl*>(desc.renderTargetViews[i]);
        renderTargetViews[i] = resourceView;
        imageViews[i] = resourceView->m_view;
        memcpy(
            &m_clearValues[i],
            &resourceView->m_texture->getDesc()->optimalClearValue.color,
            sizeof(gfx::ColorClearValue));
    }

    if (dsv)
    {
        imageViews[desc.renderTargetCount] = dsv->m_view;
        depthStencilView = dsv;
        memcpy(
            &m_clearValues[desc.renderTargetCount],
            &dsv->m_texture->getDesc()->optimalClearValue.depthStencil,
            sizeof(gfx::DepthStencilClearValue));
    }

    // Create framebuffer.
    m_layout = static_cast<FramebufferLayoutImpl*>(desc.layout);
    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = m_layout->m_renderPass;
    framebufferInfo.attachmentCount = numAttachments;
    framebufferInfo.pAttachments = imageViews.getBuffer();
    framebufferInfo.width = m_width;
    framebufferInfo.height = m_height;
    framebufferInfo.layers = layerCount;

    SLANG_VK_RETURN_ON_FAIL(m_renderer->m_api.vkCreateFramebuffer(
        m_renderer->m_api.m_device, &framebufferInfo, nullptr, &m_handle));
    return SLANG_OK;
}

ShaderProgramImpl::ShaderProgramImpl(DeviceImpl* device)
    : m_device(device)
{
    for (auto& shaderModule : m_modules)
        shaderModule = VK_NULL_HANDLE;
}

ShaderProgramImpl::~ShaderProgramImpl()
{
    for (auto shaderModule : m_modules)
    {
        if (shaderModule != VK_NULL_HANDLE)
        {
            m_device->m_api.vkDestroyShaderModule(m_device->m_api.m_device, shaderModule, nullptr);
        }
    }
}

void ShaderProgramImpl::comFree() { m_device.breakStrongReference(); }

VkPipelineShaderStageCreateInfo ShaderProgramImpl::compileEntryPoint(
    const char* entryPointName,
    ISlangBlob* code,
    VkShaderStageFlagBits stage,
    VkShaderModule& outShaderModule)
{
    char const* dataBegin = (char const*)code->getBufferPointer();
    char const* dataEnd = (char const*)code->getBufferPointer() + code->getBufferSize();

    // We need to make a copy of the code, since the Slang compiler
    // will free the memory after a compile request is closed.

    VkShaderModuleCreateInfo moduleCreateInfo = {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
    moduleCreateInfo.pCode = (uint32_t*)code->getBufferPointer();
    moduleCreateInfo.codeSize = code->getBufferSize();

    VkShaderModule module;
    SLANG_VK_CHECK(m_device->m_api.vkCreateShaderModule(
        m_device->m_device, &moduleCreateInfo, nullptr, &module));
    outShaderModule = module;

    VkPipelineShaderStageCreateInfo shaderStageCreateInfo = {
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
    shaderStageCreateInfo.stage = stage;

    shaderStageCreateInfo.module = module;
    shaderStageCreateInfo.pName = entryPointName;

    return shaderStageCreateInfo;
}

Result ShaderProgramImpl::createShaderModule(
    slang::EntryPointReflection* entryPointInfo, ComPtr<ISlangBlob> kernelCode)
{
    m_codeBlobs.add(kernelCode);
    VkShaderModule shaderModule;
    // HACK: our direct-spirv-emit path generates SPIRV that respects
    // the original entry point name, while the glslang path always
    // uses "main" as the name. We should introduce a compiler parameter
    // to control the entry point naming behavior in SPIRV-direct path
    // so we can remove the ad-hoc logic here.
    auto realEntryPointName = entryPointInfo->getNameOverride();
    const char* spirvBinaryEntryPointName = "main";
    if (m_device->m_desc.slang.targetFlags & SLANG_TARGET_FLAG_GENERATE_SPIRV_DIRECTLY)
        spirvBinaryEntryPointName = realEntryPointName;
    m_stageCreateInfos.add(compileEntryPoint(
        spirvBinaryEntryPointName,
        kernelCode,
        (VkShaderStageFlagBits)VulkanUtil::getShaderStage(entryPointInfo->getStage()),
        shaderModule));
    m_entryPointNames.add(realEntryPointName);
    m_modules.add(shaderModule);
    return SLANG_OK;
}

PipelineStateImpl::PipelineStateImpl(DeviceImpl* device)
{
    // Only weakly reference `device` at start.
    // We make it a strong reference only when the pipeline state is exposed to the user.
    // Note that `PipelineState`s may also be created via implicit specialization that
    // happens behind the scenes, and the user will not have access to those specialized
    // pipeline states. Only those pipeline states that are returned to the user needs to
    // hold a strong reference to `device`.
    m_device.setWeakReference(device);
}

PipelineStateImpl::~PipelineStateImpl()
{
    if (m_pipeline != VK_NULL_HANDLE)
    {
        m_device->m_api.vkDestroyPipeline(m_device->m_api.m_device, m_pipeline, nullptr);
    }
}

void PipelineStateImpl::establishStrongDeviceReference() { m_device.establishStrongReference(); }

void PipelineStateImpl::comFree() { m_device.breakStrongReference(); }

void PipelineStateImpl::init(const GraphicsPipelineStateDesc& inDesc)
{
    PipelineStateDesc pipelineDesc;
    pipelineDesc.type = PipelineType::Graphics;
    pipelineDesc.graphics = inDesc;
    initializeBase(pipelineDesc);
}

void PipelineStateImpl::init(const ComputePipelineStateDesc& inDesc)
{
    PipelineStateDesc pipelineDesc;
    pipelineDesc.type = PipelineType::Compute;
    pipelineDesc.compute = inDesc;
    initializeBase(pipelineDesc);
}

void PipelineStateImpl::init(const RayTracingPipelineStateDesc& inDesc)
{
    PipelineStateDesc pipelineDesc;
    pipelineDesc.type = PipelineType::RayTracing;
    pipelineDesc.rayTracing.set(inDesc);
    initializeBase(pipelineDesc);
}

Result PipelineStateImpl::createVKGraphicsPipelineState()
{
    VkPipelineCache pipelineCache = VK_NULL_HANDLE;

    auto inputLayoutImpl = (InputLayoutImpl*)desc.graphics.inputLayout;

    // VertexBuffer/s
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;

    if (inputLayoutImpl)
    {
        const auto& srcAttributeDescs = inputLayoutImpl->m_attributeDescs;
        const auto& srcStreamDescs = inputLayoutImpl->m_streamDescs;

        vertexInputInfo.vertexBindingDescriptionCount = (uint32_t)srcStreamDescs.getCount();
        vertexInputInfo.pVertexBindingDescriptions = srcStreamDescs.getBuffer();

        vertexInputInfo.vertexAttributeDescriptionCount = (uint32_t)srcAttributeDescs.getCount();
        vertexInputInfo.pVertexAttributeDescriptions = srcAttributeDescs.getBuffer();
    }

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    // All other forms of primitive toplogies are specified via dynamic state.
    inputAssembly.topology =
        VulkanUtil::translatePrimitiveTypeToListTopology(desc.graphics.primitiveType);
    inputAssembly.primitiveRestartEnable = VK_FALSE; // TODO: Currently unsupported

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    // We are using dynamic viewport and scissor state.
    // Here we specify an arbitrary size, actual viewport will be set at `beginRenderPass`
    // time.
    viewport.width = 16.0f;
    viewport.height = 16.0f;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = {uint32_t(16), uint32_t(16)};

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    auto rasterizerDesc = desc.graphics.rasterizer;

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable =
        VK_TRUE; // TODO: Depth clipping and clamping are different between Vk and D3D12
    rasterizer.rasterizerDiscardEnable = VK_FALSE; // TODO: Currently unsupported
    rasterizer.polygonMode = VulkanUtil::translateFillMode(rasterizerDesc.fillMode);
    rasterizer.cullMode = VulkanUtil::translateCullMode(rasterizerDesc.cullMode);
    rasterizer.frontFace = VulkanUtil::translateFrontFaceMode(rasterizerDesc.frontFace);
    rasterizer.depthBiasEnable = (rasterizerDesc.depthBias == 0) ? VK_FALSE : VK_TRUE;
    rasterizer.depthBiasConstantFactor = (float)rasterizerDesc.depthBias;
    rasterizer.depthBiasClamp = rasterizerDesc.depthBiasClamp;
    rasterizer.depthBiasSlopeFactor = rasterizerDesc.slopeScaledDepthBias;
    rasterizer.lineWidth = 1.0f; // TODO: Currently unsupported

    VkPipelineRasterizationConservativeStateCreateInfoEXT conservativeRasterInfo = {};
    conservativeRasterInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_CONSERVATIVE_STATE_CREATE_INFO_EXT;
    conservativeRasterInfo.conservativeRasterizationMode =
        VK_CONSERVATIVE_RASTERIZATION_MODE_OVERESTIMATE_EXT;
    if (desc.graphics.rasterizer.enableConservativeRasterization)
    {
        rasterizer.pNext = &conservativeRasterInfo;
    }

    auto framebufferLayoutImpl =
        static_cast<FramebufferLayoutImpl*>(desc.graphics.framebufferLayout);
    auto forcedSampleCount = rasterizerDesc.forcedSampleCount;
    auto blendDesc = desc.graphics.blend;

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.rasterizationSamples = (forcedSampleCount == 0)
                                             ? framebufferLayoutImpl->m_sampleCount
                                             : VulkanUtil::translateSampleCount(forcedSampleCount);
    multisampling.sampleShadingEnable =
        VK_FALSE; // TODO: Should check if fragment shader needs this
    // TODO: Sample mask is dynamic in D3D12 but PSO state in Vulkan
    multisampling.alphaToCoverageEnable = blendDesc.alphaToCoverageEnable;
    multisampling.alphaToOneEnable = VK_FALSE;

    auto targetCount =
        Math::Min(framebufferLayoutImpl->m_renderTargetCount, (uint32_t)blendDesc.targetCount);
    List<VkPipelineColorBlendAttachmentState> colorBlendAttachments;

    // Regardless of whether blending is enabled, Vulkan always applies the color write mask
    // operation, so if there is no blending then we need to add an attachment that defines
    // the color write mask to ensure colors are actually written.
    if (targetCount == 0)
    {
        colorBlendAttachments.setCount(1);
        auto& vkBlendDesc = colorBlendAttachments[0];
        memset(&vkBlendDesc, 0, sizeof(vkBlendDesc));
        vkBlendDesc.blendEnable = VK_FALSE;
        vkBlendDesc.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        vkBlendDesc.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
        vkBlendDesc.colorBlendOp = VK_BLEND_OP_ADD;
        vkBlendDesc.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        vkBlendDesc.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        vkBlendDesc.alphaBlendOp = VK_BLEND_OP_ADD;
        vkBlendDesc.colorWriteMask = (VkColorComponentFlags)RenderTargetWriteMask::EnableAll;
    }
    else
    {
        colorBlendAttachments.setCount(targetCount);
        for (UInt i = 0; i < targetCount; ++i)
        {
            auto& gfxBlendDesc = blendDesc.targets[i];
            auto& vkBlendDesc = colorBlendAttachments[i];

            vkBlendDesc.blendEnable = gfxBlendDesc.enableBlend;
            vkBlendDesc.srcColorBlendFactor =
                VulkanUtil::translateBlendFactor(gfxBlendDesc.color.srcFactor);
            vkBlendDesc.dstColorBlendFactor =
                VulkanUtil::translateBlendFactor(gfxBlendDesc.color.dstFactor);
            vkBlendDesc.colorBlendOp = VulkanUtil::translateBlendOp(gfxBlendDesc.color.op);
            vkBlendDesc.srcAlphaBlendFactor =
                VulkanUtil::translateBlendFactor(gfxBlendDesc.alpha.srcFactor);
            vkBlendDesc.dstAlphaBlendFactor =
                VulkanUtil::translateBlendFactor(gfxBlendDesc.alpha.dstFactor);
            vkBlendDesc.alphaBlendOp = VulkanUtil::translateBlendOp(gfxBlendDesc.alpha.op);
            vkBlendDesc.colorWriteMask = (VkColorComponentFlags)gfxBlendDesc.writeMask;
        }
    }

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE; // TODO: D3D12 has per attachment logic op (and
                                            // both have way more than one op)
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = (uint32_t)colorBlendAttachments.getCount();
    colorBlending.pAttachments = colorBlendAttachments.getBuffer();
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    Array<VkDynamicState, 8> dynamicStates;
    dynamicStates.add(VK_DYNAMIC_STATE_VIEWPORT);
    dynamicStates.add(VK_DYNAMIC_STATE_SCISSOR);
    dynamicStates.add(VK_DYNAMIC_STATE_STENCIL_REFERENCE);
    dynamicStates.add(VK_DYNAMIC_STATE_BLEND_CONSTANTS);
    if (m_device->m_api.m_extendedFeatures.extendedDynamicStateFeatures.extendedDynamicState)
    {
        dynamicStates.add(VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY_EXT);
    }
    VkPipelineDynamicStateCreateInfo dynamicStateInfo = {};
    dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateInfo.dynamicStateCount = (uint32_t)dynamicStates.getCount();
    dynamicStateInfo.pDynamicStates = dynamicStates.getBuffer();

    VkPipelineDepthStencilStateCreateInfo depthStencilStateInfo = {};
    depthStencilStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilStateInfo.depthTestEnable = desc.graphics.depthStencil.depthTestEnable ? 1 : 0;
    depthStencilStateInfo.back =
        VulkanUtil::translateStencilState(desc.graphics.depthStencil.backFace);
    depthStencilStateInfo.front =
        VulkanUtil::translateStencilState(desc.graphics.depthStencil.frontFace);
    depthStencilStateInfo.back.compareMask = desc.graphics.depthStencil.stencilReadMask;
    depthStencilStateInfo.back.writeMask = desc.graphics.depthStencil.stencilWriteMask;
    depthStencilStateInfo.front.compareMask = desc.graphics.depthStencil.stencilReadMask;
    depthStencilStateInfo.front.writeMask = desc.graphics.depthStencil.stencilWriteMask;
    depthStencilStateInfo.depthBoundsTestEnable = 0; // TODO: Currently unsupported
    depthStencilStateInfo.depthCompareOp =
        VulkanUtil::translateComparisonFunc(desc.graphics.depthStencil.depthFunc);
    depthStencilStateInfo.depthWriteEnable = desc.graphics.depthStencil.depthWriteEnable ? 1 : 0;
    depthStencilStateInfo.stencilTestEnable = desc.graphics.depthStencil.stencilEnable ? 1 : 0;

    VkGraphicsPipelineCreateInfo pipelineInfo = {VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};

    auto programImpl = static_cast<ShaderProgramImpl*>(m_program.Ptr());
    if (programImpl->m_stageCreateInfos.getCount() == 0)
    {
        SLANG_RETURN_ON_FAIL(programImpl->compileShaders());
    }

    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = (uint32_t)programImpl->m_stageCreateInfos.getCount();
    pipelineInfo.pStages = programImpl->m_stageCreateInfos.getBuffer();
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDepthStencilState = &depthStencilStateInfo;
    pipelineInfo.layout = programImpl->m_rootObjectLayout->m_pipelineLayout;
    pipelineInfo.renderPass = framebufferLayoutImpl->m_renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.pDynamicState = &dynamicStateInfo;

    SLANG_VK_CHECK(m_device->m_api.vkCreateGraphicsPipelines(
        m_device->m_device, pipelineCache, 1, &pipelineInfo, nullptr, &m_pipeline));

    return SLANG_OK;
}

Result PipelineStateImpl::createVKComputePipelineState()
{
    auto programImpl = static_cast<ShaderProgramImpl*>(m_program.Ptr());
    if (programImpl->m_stageCreateInfos.getCount() == 0)
    {
        SLANG_RETURN_ON_FAIL(programImpl->compileShaders());
    }

    VkPipelineCache pipelineCache = VK_NULL_HANDLE;

    VkComputePipelineCreateInfo computePipelineInfo = {
        VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO};
    computePipelineInfo.stage = programImpl->m_stageCreateInfos[0];
    computePipelineInfo.layout = programImpl->m_rootObjectLayout->m_pipelineLayout;
    SLANG_VK_CHECK(m_device->m_api.vkCreateComputePipelines(
        m_device->m_device, pipelineCache, 1, &computePipelineInfo, nullptr, &m_pipeline));
    return SLANG_OK;
}

Result PipelineStateImpl::ensureAPIPipelineStateCreated()
{
    if (m_pipeline)
        return SLANG_OK;

    switch (desc.type)
    {
    case PipelineType::Compute:
        return createVKComputePipelineState();
    case PipelineType::Graphics:
        return createVKGraphicsPipelineState();
    default:
        SLANG_UNREACHABLE("Unknown pipeline type.");
        return SLANG_FAIL;
    }
}
SLANG_NO_THROW Result SLANG_MCALL PipelineStateImpl::getNativeHandle(InteropHandle* outHandle)
{
    SLANG_RETURN_ON_FAIL(ensureAPIPipelineStateCreated());
    outHandle->api = InteropHandleAPI::Vulkan;
    outHandle->handleValue = 0;
    memcpy(&outHandle->handleValue, &m_pipeline, sizeof(m_pipeline));
    return SLANG_OK;
}

RayTracingPipelineStateImpl::RayTracingPipelineStateImpl(DeviceImpl* device)
    : PipelineStateImpl(device)
{}
uint32_t RayTracingPipelineStateImpl::findEntryPointIndexByName(
    const Dictionary<String, Index>& entryPointNameToIndex, const char* name)
{
    if (!name)
        return VK_SHADER_UNUSED_KHR;

    auto indexPtr = entryPointNameToIndex.TryGetValue(String(name));
    if (indexPtr)
        return (uint32_t)*indexPtr;
    // TODO: Error reporting?
    return VK_SHADER_UNUSED_KHR;
}
Result RayTracingPipelineStateImpl::createVKRayTracingPipelineState()
{
    auto programImpl = static_cast<ShaderProgramImpl*>(m_program.Ptr());
    if (programImpl->m_stageCreateInfos.getCount() == 0)
    {
        SLANG_RETURN_ON_FAIL(programImpl->compileShaders());
    }

    VkRayTracingPipelineCreateInfoKHR raytracingPipelineInfo = {
        VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR};
    raytracingPipelineInfo.pNext = nullptr;
    raytracingPipelineInfo.flags = translateRayTracingPipelineFlags(desc.rayTracing.flags);

    raytracingPipelineInfo.stageCount = (uint32_t)programImpl->m_stageCreateInfos.getCount();
    raytracingPipelineInfo.pStages = programImpl->m_stageCreateInfos.getBuffer();

    // Build Dictionary from entry point name to entry point index (stageCreateInfos index)
    // for all hit shaders - findShaderIndexByName
    Dictionary<String, Index> entryPointNameToIndex;

    List<VkRayTracingShaderGroupCreateInfoKHR> shaderGroupInfos;
    for (uint32_t i = 0; i < raytracingPipelineInfo.stageCount; ++i)
    {
        auto stageCreateInfo = programImpl->m_stageCreateInfos[i];
        auto entryPointName = programImpl->m_entryPointNames[i];
        entryPointNameToIndex.Add(entryPointName, i);
        if (stageCreateInfo.stage &
            (VK_SHADER_STAGE_ANY_HIT_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR |
             VK_SHADER_STAGE_INTERSECTION_BIT_KHR))
            continue;

        VkRayTracingShaderGroupCreateInfoKHR shaderGroupInfo = {
            VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR};
        shaderGroupInfo.pNext = nullptr;
        shaderGroupInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        shaderGroupInfo.generalShader = i;
        shaderGroupInfo.closestHitShader = VK_SHADER_UNUSED_KHR;
        shaderGroupInfo.anyHitShader = VK_SHADER_UNUSED_KHR;
        shaderGroupInfo.intersectionShader = VK_SHADER_UNUSED_KHR;
        shaderGroupInfo.pShaderGroupCaptureReplayHandle = nullptr;

        // For groups with a single entry point, the group name is the entry point name.
        auto shaderGroupName = entryPointName;
        auto shaderGroupIndex = shaderGroupInfos.getCount();
        shaderGroupInfos.add(shaderGroupInfo);
        shaderGroupNameToIndex.Add(shaderGroupName, shaderGroupIndex);
    }

    for (int32_t i = 0; i < desc.rayTracing.hitGroupDescs.getCount(); ++i)
    {
        VkRayTracingShaderGroupCreateInfoKHR shaderGroupInfo = {
            VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR};
        auto& groupDesc = desc.rayTracing.hitGroupDescs[i];

        shaderGroupInfo.pNext = nullptr;
        shaderGroupInfo.type = (groupDesc.intersectionEntryPoint)
                                   ? VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR
                                   : VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
        shaderGroupInfo.generalShader = VK_SHADER_UNUSED_KHR;
        shaderGroupInfo.closestHitShader =
            findEntryPointIndexByName(entryPointNameToIndex, groupDesc.closestHitEntryPoint);
        shaderGroupInfo.anyHitShader =
            findEntryPointIndexByName(entryPointNameToIndex, groupDesc.anyHitEntryPoint);
        shaderGroupInfo.intersectionShader =
            findEntryPointIndexByName(entryPointNameToIndex, groupDesc.intersectionEntryPoint);
        shaderGroupInfo.pShaderGroupCaptureReplayHandle = nullptr;

        auto shaderGroupIndex = shaderGroupInfos.getCount();
        shaderGroupInfos.add(shaderGroupInfo);
        shaderGroupNameToIndex.Add(String(groupDesc.hitGroupName), shaderGroupIndex);
    }

    raytracingPipelineInfo.groupCount = (uint32_t)shaderGroupInfos.getCount();
    raytracingPipelineInfo.pGroups = shaderGroupInfos.getBuffer();

    raytracingPipelineInfo.maxPipelineRayRecursionDepth = (uint32_t)desc.rayTracing.maxRecursion;

    raytracingPipelineInfo.pLibraryInfo = nullptr;
    raytracingPipelineInfo.pLibraryInterface = nullptr;

    raytracingPipelineInfo.pDynamicState = nullptr;

    raytracingPipelineInfo.layout = programImpl->m_rootObjectLayout->m_pipelineLayout;
    raytracingPipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    raytracingPipelineInfo.basePipelineIndex = 0;

    VkPipelineCache pipelineCache = VK_NULL_HANDLE;
    SLANG_VK_CHECK(m_device->m_api.vkCreateRayTracingPipelinesKHR(
        m_device->m_device,
        VK_NULL_HANDLE,
        pipelineCache,
        1,
        &raytracingPipelineInfo,
        nullptr,
        &m_pipeline));
    shaderGroupCount = shaderGroupInfos.getCount();
    return SLANG_OK;
}
Result RayTracingPipelineStateImpl::ensureAPIPipelineStateCreated()
{
    if (m_pipeline)
        return SLANG_OK;

    switch (desc.type)
    {
    case PipelineType::RayTracing:
        return createVKRayTracingPipelineState();
    default:
        SLANG_UNREACHABLE("Unknown pipeline type.");
        return SLANG_FAIL;
    }
}
Result RayTracingPipelineStateImpl::getNativeHandle(InteropHandle* outHandle)
{
    SLANG_RETURN_ON_FAIL(ensureAPIPipelineStateCreated());
    outHandle->api = InteropHandleAPI::Vulkan;
    outHandle->handleValue = 0;
    memcpy(&outHandle->handleValue, &m_pipeline, sizeof(m_pipeline));
    return SLANG_OK;
}

Index ShaderObjectLayoutImpl::Builder::findOrAddDescriptorSet(Index space)
{
    Index index;
    if (m_mapSpaceToDescriptorSetIndex.TryGetValue(space, index))
        return index;

    DescriptorSetInfo info = {};
    info.space = space;

    index = m_descriptorSetBuildInfos.getCount();
    m_descriptorSetBuildInfos.add(info);

    m_mapSpaceToDescriptorSetIndex.Add(space, index);
    return index;
}

VkDescriptorType ShaderObjectLayoutImpl::Builder::_mapDescriptorType(
    slang::BindingType slangBindingType)
{
    switch (slangBindingType)
    {
    case slang::BindingType::PushConstant:
    default:
        SLANG_ASSERT("unsupported binding type");
        return VK_DESCRIPTOR_TYPE_MAX_ENUM;

    case slang::BindingType::Sampler:
        return VK_DESCRIPTOR_TYPE_SAMPLER;
    case slang::BindingType::CombinedTextureSampler:
        return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    case slang::BindingType::Texture:
        return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    case slang::BindingType::MutableTexture:
        return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    case slang::BindingType::TypedBuffer:
        return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
    case slang::BindingType::MutableTypedBuffer:
        return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
    case slang::BindingType::RawBuffer:
    case slang::BindingType::MutableRawBuffer:
        return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    case slang::BindingType::InputRenderTarget:
        return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    case slang::BindingType::InlineUniformData:
        return VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT;
    case slang::BindingType::RayTracingAccelerationStructure:
        return VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
    case slang::BindingType::ConstantBuffer:
        return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    }
}

/// Add any descriptor ranges implied by this object containing a leaf
/// sub-object described by `typeLayout`, at the given `offset`.

void ShaderObjectLayoutImpl::Builder::_addDescriptorRangesAsValue(
    slang::TypeLayoutReflection* typeLayout, BindingOffset const& offset)
{
    // First we will scan through all the descriptor sets that the Slang reflection
    // information believes go into making up the given type.
    //
    // Note: We are initializing the sets in order so that their order in our
    // internal data structures should be deterministically based on the order
    // in which they are listed in Slang's reflection information.
    //
    Index descriptorSetCount = typeLayout->getDescriptorSetCount();
    for (Index i = 0; i < descriptorSetCount; ++i)
    {
        SlangInt descriptorRangeCount = typeLayout->getDescriptorSetDescriptorRangeCount(i);
        if (descriptorRangeCount == 0)
            continue;
        auto descriptorSetIndex =
            findOrAddDescriptorSet(offset.bindingSet + typeLayout->getDescriptorSetSpaceOffset(i));
    }

    // For actually populating the descriptor sets we prefer to enumerate
    // the binding ranges of the type instead of the descriptor sets.
    //
    Index bindRangeCount = typeLayout->getBindingRangeCount();
    for (Index i = 0; i < bindRangeCount; ++i)
    {
        auto bindingRangeIndex = i;
        auto bindingRangeType = typeLayout->getBindingRangeType(bindingRangeIndex);
        switch (bindingRangeType)
        {
        default:
            break;

        // We will skip over ranges that represent sub-objects for now, and handle
        // them in a separate pass.
        //
        case slang::BindingType::ParameterBlock:
        case slang::BindingType::ConstantBuffer:
        case slang::BindingType::ExistentialValue:
        case slang::BindingType::PushConstant:
            continue;
        }

        // Given a binding range we are interested in, we will then enumerate
        // its contained descriptor ranges.

        Index descriptorRangeCount =
            typeLayout->getBindingRangeDescriptorRangeCount(bindingRangeIndex);
        if (descriptorRangeCount == 0)
            continue;
        auto slangDescriptorSetIndex =
            typeLayout->getBindingRangeDescriptorSetIndex(bindingRangeIndex);
        auto descriptorSetIndex = findOrAddDescriptorSet(
            offset.bindingSet + typeLayout->getDescriptorSetSpaceOffset(slangDescriptorSetIndex));
        auto& descriptorSetInfo = m_descriptorSetBuildInfos[descriptorSetIndex];

        Index firstDescriptorRangeIndex =
            typeLayout->getBindingRangeFirstDescriptorRangeIndex(bindingRangeIndex);
        for (Index j = 0; j < descriptorRangeCount; ++j)
        {
            Index descriptorRangeIndex = firstDescriptorRangeIndex + j;
            auto slangDescriptorType = typeLayout->getDescriptorSetDescriptorRangeType(
                slangDescriptorSetIndex, descriptorRangeIndex);

            // Certain kinds of descriptor ranges reflected by Slang do not
            // manifest as descriptors at the Vulkan level, so we will skip those.
            //
            switch (slangDescriptorType)
            {
            case slang::BindingType::ExistentialValue:
            case slang::BindingType::InlineUniformData:
            case slang::BindingType::PushConstant:
                continue;
            default:
                break;
            }

            auto vkDescriptorType = _mapDescriptorType(slangDescriptorType);
            VkDescriptorSetLayoutBinding vkBindingRangeDesc = {};
            vkBindingRangeDesc.binding =
                offset.binding + (uint32_t)typeLayout->getDescriptorSetDescriptorRangeIndexOffset(
                                     slangDescriptorSetIndex, descriptorRangeIndex);
            vkBindingRangeDesc.descriptorCount =
                (uint32_t)typeLayout->getDescriptorSetDescriptorRangeDescriptorCount(
                    slangDescriptorSetIndex, descriptorRangeIndex);
            vkBindingRangeDesc.descriptorType = vkDescriptorType;
            vkBindingRangeDesc.stageFlags = VK_SHADER_STAGE_ALL;

            descriptorSetInfo.vkBindings.add(vkBindingRangeDesc);
        }
    }

    // We skipped over the sub-object ranges when adding descriptors above,
    // and now we will address that oversight by iterating over just
    // the sub-object ranges.
    //
    Index subObjectRangeCount = typeLayout->getSubObjectRangeCount();
    for (Index subObjectRangeIndex = 0; subObjectRangeIndex < subObjectRangeCount;
         ++subObjectRangeIndex)
    {
        auto bindingRangeIndex =
            typeLayout->getSubObjectRangeBindingRangeIndex(subObjectRangeIndex);
        auto bindingType = typeLayout->getBindingRangeType(bindingRangeIndex);

        auto subObjectTypeLayout = typeLayout->getBindingRangeLeafTypeLayout(bindingRangeIndex);
        SLANG_ASSERT(subObjectTypeLayout);

        BindingOffset subObjectRangeOffset = offset;
        subObjectRangeOffset +=
            BindingOffset(typeLayout->getSubObjectRangeOffset(subObjectRangeIndex));

        switch (bindingType)
        {
        // A `ParameterBlock<X>` never contributes descripto ranges to the
        // decriptor sets of a parent object.
        //
        case slang::BindingType::ParameterBlock:
        default:
            break;

        case slang::BindingType::ExistentialValue:
            // An interest/existential-typed sub-object range will only contribute
            // descriptor ranges to a parent object in the case where it has been
            // specialied, which is precisely the case where the Slang reflection
            // information will tell us about its "pending" layout.
            //
            if (auto pendingTypeLayout = subObjectTypeLayout->getPendingDataTypeLayout())
            {
                BindingOffset pendingOffset = BindingOffset(subObjectRangeOffset.pending);
                _addDescriptorRangesAsValue(pendingTypeLayout, pendingOffset);
            }
            break;

        case slang::BindingType::ConstantBuffer:
            {
                // A `ConstantBuffer<X>` range will contribute any nested descriptor
                // ranges in `X`, along with a leading descriptor range for a
                // uniform buffer to hold ordinary/uniform data, if there is any.

                SLANG_ASSERT(subObjectTypeLayout);

                auto containerVarLayout = subObjectTypeLayout->getContainerVarLayout();
                SLANG_ASSERT(containerVarLayout);

                auto elementVarLayout = subObjectTypeLayout->getElementVarLayout();
                SLANG_ASSERT(elementVarLayout);

                auto elementTypeLayout = elementVarLayout->getTypeLayout();
                SLANG_ASSERT(elementTypeLayout);

                BindingOffset containerOffset = subObjectRangeOffset;
                containerOffset += BindingOffset(subObjectTypeLayout->getContainerVarLayout());

                BindingOffset elementOffset = subObjectRangeOffset;
                elementOffset += BindingOffset(elementVarLayout);

                _addDescriptorRangesAsConstantBuffer(
                    elementTypeLayout, containerOffset, elementOffset);
            }
            break;

        case slang::BindingType::PushConstant:
            {
                // This case indicates a `ConstantBuffer<X>` that was marked as being
                // used for push constants.
                //
                // Much of the handling is the same as for an ordinary
                // `ConstantBuffer<X>`, but of course we need to handle the ordinary
                // data part differently.

                SLANG_ASSERT(subObjectTypeLayout);

                auto containerVarLayout = subObjectTypeLayout->getContainerVarLayout();
                SLANG_ASSERT(containerVarLayout);

                auto elementVarLayout = subObjectTypeLayout->getElementVarLayout();
                SLANG_ASSERT(elementVarLayout);

                auto elementTypeLayout = elementVarLayout->getTypeLayout();
                SLANG_ASSERT(elementTypeLayout);

                BindingOffset containerOffset = subObjectRangeOffset;
                containerOffset += BindingOffset(subObjectTypeLayout->getContainerVarLayout());

                BindingOffset elementOffset = subObjectRangeOffset;
                elementOffset += BindingOffset(elementVarLayout);

                _addDescriptorRangesAsPushConstantBuffer(
                    elementTypeLayout, containerOffset, elementOffset);
            }
            break;
        }
    }
}

/// Add the descriptor ranges implied by a `ConstantBuffer<X>` where `X` is
/// described by `elementTypeLayout`.
///
/// The `containerOffset` and `elementOffset` are the binding offsets that
/// should apply to the buffer itself and the contents of the buffer, respectively.
///

void ShaderObjectLayoutImpl::Builder::_addDescriptorRangesAsConstantBuffer(
    slang::TypeLayoutReflection* elementTypeLayout,
    BindingOffset const& containerOffset,
    BindingOffset const& elementOffset)
{
    // If the type has ordinary uniform data fields, we need to make sure to create
    // a descriptor set with a constant buffer binding in the case that the shader
    // object is bound as a stand alone parameter block.
    if (elementTypeLayout->getSize(SLANG_PARAMETER_CATEGORY_UNIFORM) != 0)
    {
        auto descriptorSetIndex = findOrAddDescriptorSet(containerOffset.bindingSet);
        auto& descriptorSetInfo = m_descriptorSetBuildInfos[descriptorSetIndex];
        VkDescriptorSetLayoutBinding vkBindingRangeDesc = {};
        vkBindingRangeDesc.binding = containerOffset.binding;
        vkBindingRangeDesc.descriptorCount = 1;
        vkBindingRangeDesc.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        vkBindingRangeDesc.stageFlags = VK_SHADER_STAGE_ALL;
        descriptorSetInfo.vkBindings.add(vkBindingRangeDesc);
    }

    _addDescriptorRangesAsValue(elementTypeLayout, elementOffset);
}

/// Add the descriptor ranges implied by a `PushConstantBuffer<X>` where `X` is
/// described by `elementTypeLayout`.
///
/// The `containerOffset` and `elementOffset` are the binding offsets that
/// should apply to the buffer itself and the contents of the buffer, respectively.
///

void ShaderObjectLayoutImpl::Builder::_addDescriptorRangesAsPushConstantBuffer(
    slang::TypeLayoutReflection* elementTypeLayout,
    BindingOffset const& containerOffset,
    BindingOffset const& elementOffset)
{
    // If the type has ordinary uniform data fields, we need to make sure to create
    // a descriptor set with a constant buffer binding in the case that the shader
    // object is bound as a stand alone parameter block.
    auto ordinaryDataSize = (uint32_t)elementTypeLayout->getSize(SLANG_PARAMETER_CATEGORY_UNIFORM);
    if (ordinaryDataSize != 0)
    {
        auto pushConstantRangeIndex = containerOffset.pushConstantRange;

        VkPushConstantRange vkPushConstantRange = {};
        vkPushConstantRange.size = ordinaryDataSize;
        vkPushConstantRange.stageFlags = VK_SHADER_STAGE_ALL; // TODO: be more clever

        while ((uint32_t)m_ownPushConstantRanges.getCount() <= pushConstantRangeIndex)
        {
            VkPushConstantRange emptyRange = {0};
            m_ownPushConstantRanges.add(emptyRange);
        }

        m_ownPushConstantRanges[pushConstantRangeIndex] = vkPushConstantRange;
    }

    _addDescriptorRangesAsValue(elementTypeLayout, elementOffset);
}

/// Add binding ranges to this shader object layout, as implied by the given
/// `typeLayout`

void ShaderObjectLayoutImpl::Builder::addBindingRanges(slang::TypeLayoutReflection* typeLayout)
{
    SlangInt bindingRangeCount = typeLayout->getBindingRangeCount();
    for (SlangInt r = 0; r < bindingRangeCount; ++r)
    {
        slang::BindingType slangBindingType = typeLayout->getBindingRangeType(r);
        uint32_t count = (uint32_t)typeLayout->getBindingRangeBindingCount(r);
        slang::TypeLayoutReflection* slangLeafTypeLayout =
            typeLayout->getBindingRangeLeafTypeLayout(r);

        Index baseIndex = 0;
        Index subObjectIndex = 0;
        switch (slangBindingType)
        {
        case slang::BindingType::ConstantBuffer:
        case slang::BindingType::ParameterBlock:
        case slang::BindingType::ExistentialValue:
            baseIndex = m_subObjectCount;
            subObjectIndex = baseIndex;
            m_subObjectCount += count;
            break;
        case slang::BindingType::RawBuffer:
        case slang::BindingType::MutableRawBuffer:
            if (slangLeafTypeLayout->getType()->getElementType() != nullptr)
            {
                // A structured buffer occupies both a resource slot and
                // a sub-object slot.
                subObjectIndex = m_subObjectCount;
                m_subObjectCount += count;
            }
            baseIndex = m_resourceViewCount;
            m_resourceViewCount += count;
            break;
        case slang::BindingType::Sampler:
            baseIndex = m_samplerCount;
            m_samplerCount += count;
            m_totalBindingCount += 1;
            break;

        case slang::BindingType::CombinedTextureSampler:
            baseIndex = m_combinedTextureSamplerCount;
            m_combinedTextureSamplerCount += count;
            m_totalBindingCount += 1;
            break;

        case slang::BindingType::VaryingInput:
            baseIndex = m_varyingInputCount;
            m_varyingInputCount += count;
            break;

        case slang::BindingType::VaryingOutput:
            baseIndex = m_varyingOutputCount;
            m_varyingOutputCount += count;
            break;
        default:
            baseIndex = m_resourceViewCount;
            m_resourceViewCount += count;
            m_totalBindingCount += 1;
            break;
        }

        BindingRangeInfo bindingRangeInfo;
        bindingRangeInfo.bindingType = slangBindingType;
        bindingRangeInfo.count = count;
        bindingRangeInfo.baseIndex = baseIndex;
        bindingRangeInfo.subObjectIndex = subObjectIndex;

        // We'd like to extract the information on the GLSL/SPIR-V
        // `binding` that this range should bind into (or whatever
        // other specific kind of offset/index is appropriate to it).
        //
        // A binding range represents a logical member of the shader
        // object type, and it may encompass zero or more *descriptor
        // ranges* that describe how it is physically bound to pipeline
        // state.
        //
        // If the current bindign range is backed by at least one descriptor
        // range then we can query the binding offset of that descriptor
        // range. We expect that in the common case there will be exactly
        // one descriptor range, and we can extract the information easily.
        //
        if (typeLayout->getBindingRangeDescriptorRangeCount(r) != 0)
        {
            SlangInt descriptorSetIndex = typeLayout->getBindingRangeDescriptorSetIndex(r);
            SlangInt descriptorRangeIndex = typeLayout->getBindingRangeFirstDescriptorRangeIndex(r);

            auto set = typeLayout->getDescriptorSetSpaceOffset(descriptorSetIndex);
            auto bindingOffset = typeLayout->getDescriptorSetDescriptorRangeIndexOffset(
                descriptorSetIndex, descriptorRangeIndex);

            bindingRangeInfo.setOffset = uint32_t(set);
            bindingRangeInfo.bindingOffset = uint32_t(bindingOffset);
        }

        m_bindingRanges.add(bindingRangeInfo);
    }

    SlangInt subObjectRangeCount = typeLayout->getSubObjectRangeCount();
    for (SlangInt r = 0; r < subObjectRangeCount; ++r)
    {
        SlangInt bindingRangeIndex = typeLayout->getSubObjectRangeBindingRangeIndex(r);
        auto& bindingRange = m_bindingRanges[bindingRangeIndex];
        auto slangBindingType = typeLayout->getBindingRangeType(bindingRangeIndex);
        slang::TypeLayoutReflection* slangLeafTypeLayout =
            typeLayout->getBindingRangeLeafTypeLayout(bindingRangeIndex);

        // A sub-object range can either represent a sub-object of a known
        // type, like a `ConstantBuffer<Foo>` or `ParameterBlock<Foo>`
        // (in which case we can pre-compute a layout to use, based on
        // the type `Foo`) *or* it can represent a sub-object of some
        // existential type (e.g., `IBar`) in which case we cannot
        // know the appropraite type/layout of sub-object to allocate.
        //
        RefPtr<ShaderObjectLayoutImpl> subObjectLayout;
        switch (slangBindingType)
        {
        default:
            {
                auto varLayout = slangLeafTypeLayout->getElementVarLayout();
                auto subTypeLayout = varLayout->getTypeLayout();
                ShaderObjectLayoutImpl::createForElementType(
                    m_renderer, subTypeLayout, subObjectLayout.writeRef());
            }
            break;

        case slang::BindingType::ExistentialValue:
            if (auto pendingTypeLayout = slangLeafTypeLayout->getPendingDataTypeLayout())
            {
                ShaderObjectLayoutImpl::createForElementType(
                    m_renderer, pendingTypeLayout, subObjectLayout.writeRef());
            }
            break;
        }

        SubObjectRangeInfo subObjectRange;
        subObjectRange.bindingRangeIndex = bindingRangeIndex;
        subObjectRange.layout = subObjectLayout;

        // We will use Slang reflection infromation to extract the offset information
        // for each sub-object range.
        //
        // TODO: We should also be extracting the uniform offset here.
        //
        subObjectRange.offset = SubObjectRangeOffset(typeLayout->getSubObjectRangeOffset(r));
        subObjectRange.stride = SubObjectRangeStride(slangLeafTypeLayout);

        switch (slangBindingType)
        {
        case slang::BindingType::ParameterBlock:
            m_childDescriptorSetCount += subObjectLayout->getTotalDescriptorSetCount();
            m_childPushConstantRangeCount += subObjectLayout->getTotalPushConstantRangeCount();
            break;

        case slang::BindingType::ConstantBuffer:
            m_childDescriptorSetCount += subObjectLayout->getChildDescriptorSetCount();
            m_totalBindingCount += subObjectLayout->getTotalBindingCount();
            m_childPushConstantRangeCount += subObjectLayout->getTotalPushConstantRangeCount();
            break;

        case slang::BindingType::ExistentialValue:
            if (subObjectLayout)
            {
                m_childDescriptorSetCount += subObjectLayout->getChildDescriptorSetCount();
                m_totalBindingCount += subObjectLayout->getTotalBindingCount();
                m_childPushConstantRangeCount += subObjectLayout->getTotalPushConstantRangeCount();

                // An interface-type range that includes ordinary data can
                // increase the size of the ordinary data buffer we need to
                // allocate for the parent object.
                //
                uint32_t ordinaryDataEnd =
                    subObjectRange.offset.pendingOrdinaryData +
                    (uint32_t)bindingRange.count * subObjectRange.stride.pendingOrdinaryData;

                if (ordinaryDataEnd > m_totalOrdinaryDataSize)
                {
                    m_totalOrdinaryDataSize = ordinaryDataEnd;
                }
            }
            break;

        default:
            break;
        }

        m_subObjectRanges.add(subObjectRange);
    }
}

Result ShaderObjectLayoutImpl::Builder::setElementTypeLayout(
    slang::TypeLayoutReflection* typeLayout)
{
    typeLayout = _unwrapParameterGroups(typeLayout, m_containerType);
    m_elementTypeLayout = typeLayout;

    m_totalOrdinaryDataSize = (uint32_t)typeLayout->getSize();

    // Next we will compute the binding ranges that are used to store
    // the logical contents of the object in memory. These will relate
    // to the descriptor ranges in the various sets, but not always
    // in a one-to-one fashion.

    addBindingRanges(typeLayout);

    // Note: This routine does not take responsibility for
    // adding descriptor ranges at all, because the exact way
    // that descriptor ranges need to be added varies between
    // ordinary shader objects, root shader objects, and entry points.

    return SLANG_OK;
}

SlangResult ShaderObjectLayoutImpl::Builder::build(ShaderObjectLayoutImpl** outLayout)
{
    auto layout = RefPtr<ShaderObjectLayoutImpl>(new ShaderObjectLayoutImpl());
    SLANG_RETURN_ON_FAIL(layout->_init(this));

    returnRefPtrMove(outLayout, layout);
    return SLANG_OK;
}

Result ShaderObjectLayoutImpl::createForElementType(
    DeviceImpl* renderer,
    slang::TypeLayoutReflection* elementType,
    ShaderObjectLayoutImpl** outLayout)
{
    Builder builder(renderer);
    builder.setElementTypeLayout(elementType);

    // When constructing a shader object layout directly from a reflected
    // type in Slang, we want to compute the descriptor sets and ranges
    // that would be used if this object were bound as a parameter block.
    //
    // It might seem like we need to deal with the other cases for how
    // the shader object might be bound, but the descriptor ranges we
    // compute here will only ever be used in parameter-block case.
    //
    // One important wrinkle is that we know that the parameter block
    // allocated for `elementType` will potentially need a buffer `binding`
    // for any ordinary data it contains.

    bool needsOrdinaryDataBuffer =
        builder.m_elementTypeLayout->getSize(SLANG_PARAMETER_CATEGORY_UNIFORM) != 0;
    uint32_t ordinaryDataBufferCount = needsOrdinaryDataBuffer ? 1 : 0;

    // When binding the object, we know that the ordinary data buffer will
    // always use a the first available `binding`, so its offset will be
    // all zeroes.
    //
    BindingOffset containerOffset;

    // In contrast, the `binding`s used by all the other entries in the
    // parameter block will need to be offset by one if there was
    // an ordinary data buffer.
    //
    BindingOffset elementOffset;
    elementOffset.binding = ordinaryDataBufferCount;

    // Furthermore, any `binding`s that arise due to "pending" data
    // in the type of the object (due to specialization for existential types)
    // will need to come after all the other `binding`s that were
    // part of the "primary" (unspecialized) data.
    //
    uint32_t primaryDescriptorCount =
        ordinaryDataBufferCount + (uint32_t)builder.m_elementTypeLayout->getSize(
                                      SLANG_PARAMETER_CATEGORY_DESCRIPTOR_TABLE_SLOT);
    elementOffset.pending.binding = primaryDescriptorCount;

    // Once we've computed the offset information, we simply add the
    // descriptor ranges as if things were declared as a `ConstantBuffer<X>`,
    // since that is how things will be laid out inside the parameter block.
    //
    builder._addDescriptorRangesAsConstantBuffer(
        builder.m_elementTypeLayout, containerOffset, elementOffset);
    return builder.build(outLayout);
}

ShaderObjectLayoutImpl::~ShaderObjectLayoutImpl()
{
    for (auto& descSetInfo : m_descriptorSetInfos)
    {
        getDevice()->m_api.vkDestroyDescriptorSetLayout(
            getDevice()->m_api.m_device, descSetInfo.descriptorSetLayout, nullptr);
    }
}

Result ShaderObjectLayoutImpl::_init(Builder const* builder)
{
    auto renderer = builder->m_renderer;

    initBase(renderer, builder->m_elementTypeLayout);

    m_bindingRanges = builder->m_bindingRanges;

    m_descriptorSetInfos = _Move(builder->m_descriptorSetBuildInfos);
    m_ownPushConstantRanges = builder->m_ownPushConstantRanges;
    m_resourceViewCount = builder->m_resourceViewCount;
    m_samplerCount = builder->m_samplerCount;
    m_combinedTextureSamplerCount = builder->m_combinedTextureSamplerCount;
    m_childDescriptorSetCount = builder->m_childDescriptorSetCount;
    m_totalBindingCount = builder->m_totalBindingCount;
    m_subObjectCount = builder->m_subObjectCount;
    m_subObjectRanges = builder->m_subObjectRanges;
    m_totalOrdinaryDataSize = builder->m_totalOrdinaryDataSize;

    m_containerType = builder->m_containerType;

    // Create VkDescriptorSetLayout for all descriptor sets.
    for (auto& descriptorSetInfo : m_descriptorSetInfos)
    {
        VkDescriptorSetLayoutCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        createInfo.pBindings = descriptorSetInfo.vkBindings.getBuffer();
        createInfo.bindingCount = (uint32_t)descriptorSetInfo.vkBindings.getCount();
        VkDescriptorSetLayout vkDescSetLayout;
        SLANG_RETURN_ON_FAIL(renderer->m_api.vkCreateDescriptorSetLayout(
            renderer->m_api.m_device, &createInfo, nullptr, &vkDescSetLayout));
        descriptorSetInfo.descriptorSetLayout = vkDescSetLayout;
    }
    return SLANG_OK;
}

Result EntryPointLayout::Builder::build(EntryPointLayout** outLayout)
{
    RefPtr<EntryPointLayout> layout = new EntryPointLayout();
    SLANG_RETURN_ON_FAIL(layout->_init(this));

    returnRefPtrMove(outLayout, layout);
    return SLANG_OK;
}

void EntryPointLayout::Builder::addEntryPointParams(slang::EntryPointLayout* entryPointLayout)
{
    m_slangEntryPointLayout = entryPointLayout;
    setElementTypeLayout(entryPointLayout->getTypeLayout());
    m_shaderStageFlag = VulkanUtil::getShaderStage(entryPointLayout->getStage());

    // Note: we do not bother adding any descriptor sets/ranges here,
    // because the descriptor ranges of an entry point will simply
    // be allocated as part of the descriptor sets for the root
    // shader object.
}

Result EntryPointLayout::_init(Builder const* builder)
{
    auto renderer = builder->m_renderer;

    SLANG_RETURN_ON_FAIL(Super::_init(builder));

    m_slangEntryPointLayout = builder->m_slangEntryPointLayout;
    m_shaderStageFlag = builder->m_shaderStageFlag;
    return SLANG_OK;
}

RootShaderObjectLayout::~RootShaderObjectLayout()
{
    if (m_pipelineLayout)
    {
        m_renderer->m_api.vkDestroyPipelineLayout(
            m_renderer->m_api.m_device, m_pipelineLayout, nullptr);
    }
}

Index RootShaderObjectLayout::findEntryPointIndex(VkShaderStageFlags stage)
{
    auto entryPointCount = m_entryPoints.getCount();
    for (Index i = 0; i < entryPointCount; ++i)
    {
        auto entryPoint = m_entryPoints[i];
        if (entryPoint.layout->getShaderStageFlag() == stage)
            return i;
    }
    return -1;
}

Result RootShaderObjectLayout::create(
    DeviceImpl* renderer,
    slang::IComponentType* program,
    slang::ProgramLayout* programLayout,
    RootShaderObjectLayout** outLayout)
{
    RootShaderObjectLayout::Builder builder(renderer, program, programLayout);
    builder.addGlobalParams(programLayout->getGlobalParamsVarLayout());

    SlangInt entryPointCount = programLayout->getEntryPointCount();
    for (SlangInt e = 0; e < entryPointCount; ++e)
    {
        auto slangEntryPoint = programLayout->getEntryPointByIndex(e);

        EntryPointLayout::Builder entryPointBuilder(renderer);
        entryPointBuilder.addEntryPointParams(slangEntryPoint);

        RefPtr<EntryPointLayout> entryPointLayout;
        SLANG_RETURN_ON_FAIL(entryPointBuilder.build(entryPointLayout.writeRef()));

        builder.addEntryPoint(entryPointLayout);
    }

    SLANG_RETURN_ON_FAIL(builder.build(outLayout));

    return SLANG_OK;
}

Result RootShaderObjectLayout::_init(Builder const* builder)
{
    auto renderer = builder->m_renderer;

    SLANG_RETURN_ON_FAIL(Super::_init(builder));

    m_program = builder->m_program;
    m_programLayout = builder->m_programLayout;
    m_entryPoints = _Move(builder->m_entryPoints);
    m_pendingDataOffset = builder->m_pendingDataOffset;
    m_renderer = renderer;

    // If the program has unbound specialization parameters,
    // then we will avoid creating a final Vulkan pipeline layout.
    //
    // TODO: We should really create the information necessary
    // for binding as part of a separate object, so that we have
    // a clean seperation between what is needed for writing into
    // a shader object vs. what is needed for binding it to the
    // pipeline. We eventually need to be able to create bindable
    // state objects from unspecialized programs, in order to
    // support dynamic dispatch.
    //
    if (m_program->getSpecializationParamCount() != 0)
        return SLANG_OK;

    // Otherwise, we need to create a final (bindable) layout.
    //
    // We will use a recursive walk to collect all the `VkDescriptorSetLayout`s
    // that are required for the global scope, sub-objects, and entry points.
    //
    SLANG_RETURN_ON_FAIL(addAllDescriptorSets());

    // We will also use a recursive walk to collect all the push-constant
    // ranges needed for this object, sub-objects, and entry points.
    //
    SLANG_RETURN_ON_FAIL(addAllPushConstantRanges());

    // Once we've collected the information across the entire
    // tree of sub-objects

    // Now call Vulkan API to create a pipeline layout.
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = (uint32_t)m_vkDescriptorSetLayouts.getCount();
    pipelineLayoutCreateInfo.pSetLayouts = m_vkDescriptorSetLayouts.getBuffer();
    if (m_allPushConstantRanges.getCount())
    {
        pipelineLayoutCreateInfo.pushConstantRangeCount =
            (uint32_t)m_allPushConstantRanges.getCount();
        pipelineLayoutCreateInfo.pPushConstantRanges = m_allPushConstantRanges.getBuffer();
    }
    SLANG_RETURN_ON_FAIL(m_renderer->m_api.vkCreatePipelineLayout(
        m_renderer->m_api.m_device, &pipelineLayoutCreateInfo, nullptr, &m_pipelineLayout));
    return SLANG_OK;
}

/// Add all the descriptor sets implied by this root object and sub-objects

Result RootShaderObjectLayout::addAllDescriptorSets()
{
    SLANG_RETURN_ON_FAIL(addAllDescriptorSetsRec(this));

    // Note: the descriptor ranges/sets for direct entry point parameters
    // were already enumerated into the ranges/sets of the root object itself,
    // so we don't wnat to add them again.
    //
    // We do however have to deal with the possibility that an entry
    // point could introduce "child" descriptor sets, e.g., because it
    // has a `ParameterBlock<X>` parameter.
    //
    for (auto& entryPoint : getEntryPoints())
    {
        SLANG_RETURN_ON_FAIL(addChildDescriptorSetsRec(entryPoint.layout));
    }

    return SLANG_OK;
}

/// Recurisvely add descriptor sets defined by `layout` and sub-objects

Result RootShaderObjectLayout::addAllDescriptorSetsRec(ShaderObjectLayoutImpl* layout)
{
    // TODO: This logic assumes that descriptor sets are all contiguous
    // and have been allocated in a global order that matches the order
    // of enumeration here.

    for (auto& descSetInfo : layout->getOwnDescriptorSets())
    {
        m_vkDescriptorSetLayouts.add(descSetInfo.descriptorSetLayout);
    }

    SLANG_RETURN_ON_FAIL(addChildDescriptorSetsRec(layout));
    return SLANG_OK;
}

/// Recurisvely add descriptor sets defined by sub-objects of `layout`

Result RootShaderObjectLayout::addChildDescriptorSetsRec(ShaderObjectLayoutImpl* layout)
{
    for (auto& subObject : layout->getSubObjectRanges())
    {
        auto bindingRange = layout->getBindingRange(subObject.bindingRangeIndex);
        switch (bindingRange.bindingType)
        {
        case slang::BindingType::ParameterBlock:
            SLANG_RETURN_ON_FAIL(addAllDescriptorSetsRec(subObject.layout));
            break;

        default:
            if (auto subObjectLayout = subObject.layout)
            {
                SLANG_RETURN_ON_FAIL(addChildDescriptorSetsRec(subObject.layout));
            }
            break;
        }
    }

    return SLANG_OK;
}

/// Add all the push-constant ranges implied by this root object and sub-objects

Result RootShaderObjectLayout::addAllPushConstantRanges()
{
    SLANG_RETURN_ON_FAIL(addAllPushConstantRangesRec(this));

    for (auto& entryPoint : getEntryPoints())
    {
        SLANG_RETURN_ON_FAIL(addChildPushConstantRangesRec(entryPoint.layout));
    }

    return SLANG_OK;
}

/// Recurisvely add push-constant ranges defined by `layout` and sub-objects

Result RootShaderObjectLayout::addAllPushConstantRangesRec(ShaderObjectLayoutImpl* layout)
{
    // TODO: This logic assumes that push-constant ranges are all contiguous
    // and have been allocated in a global order that matches the order
    // of enumeration here.

    for (auto pushConstantRange : layout->getOwnPushConstantRanges())
    {
        pushConstantRange.offset = m_totalPushConstantSize;
        m_totalPushConstantSize += pushConstantRange.size;

        m_allPushConstantRanges.add(pushConstantRange);
    }

    SLANG_RETURN_ON_FAIL(addChildPushConstantRangesRec(layout));
    return SLANG_OK;
}

/// Recurisvely add push-constant ranges defined by sub-objects of `layout`

Result RootShaderObjectLayout::addChildPushConstantRangesRec(ShaderObjectLayoutImpl* layout)
{
    for (auto& subObject : layout->getSubObjectRanges())
    {
        if (auto subObjectLayout = subObject.layout)
        {
            SLANG_RETURN_ON_FAIL(addAllPushConstantRangesRec(subObject.layout));
        }
    }

    return SLANG_OK;
}

Result RootShaderObjectLayout::Builder::build(RootShaderObjectLayout** outLayout)
{
    RefPtr<RootShaderObjectLayout> layout = new RootShaderObjectLayout();
    SLANG_RETURN_ON_FAIL(layout->_init(this));
    returnRefPtrMove(outLayout, layout);
    return SLANG_OK;
}

void RootShaderObjectLayout::Builder::addGlobalParams(
    slang::VariableLayoutReflection* globalsLayout)
{
    setElementTypeLayout(globalsLayout->getTypeLayout());

    // We need to populate our descriptor sets/ranges with information
    // from the layout of the global scope.
    //
    // While we expect that the parameter in the global scope start
    // at an offset of zero, it is also worth querying the offset
    // information because it could impact the locations assigned
    // to "pending" data in the case of static specialization.
    //
    BindingOffset offset(globalsLayout);

    // Note: We are adding descriptor ranges here based directly on
    // the type of the global-scope layout. The type layout for the
    // global scope will either be something like a `struct GlobalParams`
    // that contains all the global-scope parameters or a `ConstantBuffer<GlobalParams>`
    // and in either case the `_addDescriptorRangesAsValue` can properly
    // add all the ranges implied.
    //
    // As a result we don't require any special-case logic here to
    // deal with the possibility of a "default" constant buffer allocated
    // for global-scope parameters of uniform/ordinary type.
    //
    _addDescriptorRangesAsValue(globalsLayout->getTypeLayout(), offset);

    // We want to keep track of the offset that was applied to "pending"
    // data because we will need it again later when it comes time to
    // actually bind things.
    //
    m_pendingDataOffset = offset.pending;
}

void RootShaderObjectLayout::Builder::addEntryPoint(EntryPointLayout* entryPointLayout)
{
    auto slangEntryPointLayout = entryPointLayout->getSlangLayout();
    auto entryPointVarLayout = slangEntryPointLayout->getVarLayout();

    // The offset information for each entry point needs to
    // be adjusted by any offset for "pending" data that
    // was recorded in the global-scope layout.
    //
    // TODO(tfoley): Double-check that this is correct.

    BindingOffset entryPointOffset(entryPointVarLayout);
    entryPointOffset.pending += m_pendingDataOffset;

    EntryPointInfo info;
    info.layout = entryPointLayout;
    info.offset = entryPointOffset;

    // Similar to the case for the global scope, we expect the
    // type layout for the entry point parameters to be either
    // a `struct EntryPointParams` or a `PushConstantBuffer<EntryPointParams>`.
    // Rather than deal with the different cases here, we will
    // trust the `_addDescriptorRangesAsValue` code to handle
    // either case correctly.
    //
    _addDescriptorRangesAsValue(entryPointVarLayout->getTypeLayout(), entryPointOffset);

    m_entryPoints.add(info);
}

Result ShaderObjectImpl::create(
    IDevice* device, ShaderObjectLayoutImpl* layout, ShaderObjectImpl** outShaderObject)
{
    auto object = RefPtr<ShaderObjectImpl>(new ShaderObjectImpl());
    SLANG_RETURN_ON_FAIL(object->init(device, layout));

    returnRefPtrMove(outShaderObject, object);
    return SLANG_OK;
}

RendererBase* ShaderObjectImpl::getDevice() { return m_layout->getDevice(); }

UInt ShaderObjectImpl::getEntryPointCount() { return 0; }

Result ShaderObjectImpl::getEntryPoint(UInt index, IShaderObject** outEntryPoint)
{
    *outEntryPoint = nullptr;
    return SLANG_OK;
}

const void* ShaderObjectImpl::getRawData() { return m_data.getBuffer(); }

size_t ShaderObjectImpl::getSize() { return (size_t)m_data.getCount(); }

Result ShaderObjectImpl::setData(ShaderOffset const& inOffset, void const* data, size_t inSize)
{
    Index offset = inOffset.uniformOffset;
    Index size = inSize;

    char* dest = m_data.getBuffer();
    Index availableSize = m_data.getCount();

    // TODO: We really should bounds-check access rather than silently ignoring sets
    // that are too large, but we have several test cases that set more data than
    // an object actually stores on several targets...
    //
    if (offset < 0)
    {
        size += offset;
        offset = 0;
    }
    if ((offset + size) >= availableSize)
    {
        size = availableSize - offset;
    }

    memcpy(dest + offset, data, size);

    m_isConstantBufferDirty = true;

    return SLANG_OK;
}

Result ShaderObjectImpl::setResource(ShaderOffset const& offset, IResourceView* resourceView)
{
    if (offset.bindingRangeIndex < 0)
        return SLANG_E_INVALID_ARG;
    auto layout = getLayout();
    if (offset.bindingRangeIndex >= layout->getBindingRangeCount())
        return SLANG_E_INVALID_ARG;
    auto& bindingRange = layout->getBindingRange(offset.bindingRangeIndex);
    if (!resourceView)
    {
        m_resourceViews[bindingRange.baseIndex + offset.bindingArrayIndex] = nullptr;
    }
    else
    {
        if (resourceView->getViewDesc()->type == IResourceView::Type::AccelerationStructure)
        {
            m_resourceViews[bindingRange.baseIndex + offset.bindingArrayIndex] =
                static_cast<AccelerationStructureImpl*>(resourceView);
        }
        else
        {
            m_resourceViews[bindingRange.baseIndex + offset.bindingArrayIndex] =
                static_cast<ResourceViewImpl*>(resourceView);
        }
    }
    return SLANG_OK;
}

Result ShaderObjectImpl::setSampler(ShaderOffset const& offset, ISamplerState* sampler)
{
    if (offset.bindingRangeIndex < 0)
        return SLANG_E_INVALID_ARG;
    auto layout = getLayout();
    if (offset.bindingRangeIndex >= layout->getBindingRangeCount())
        return SLANG_E_INVALID_ARG;
    auto& bindingRange = layout->getBindingRange(offset.bindingRangeIndex);

    m_samplers[bindingRange.baseIndex + offset.bindingArrayIndex] =
        static_cast<SamplerStateImpl*>(sampler);
    return SLANG_OK;
}

Result ShaderObjectImpl::setCombinedTextureSampler(
    ShaderOffset const& offset, IResourceView* textureView, ISamplerState* sampler)
{
    if (offset.bindingRangeIndex < 0)
        return SLANG_E_INVALID_ARG;
    auto layout = getLayout();
    if (offset.bindingRangeIndex >= layout->getBindingRangeCount())
        return SLANG_E_INVALID_ARG;
    auto& bindingRange = layout->getBindingRange(offset.bindingRangeIndex);

    auto& slot = m_combinedTextureSamplers[bindingRange.baseIndex + offset.bindingArrayIndex];
    slot.textureView = static_cast<TextureResourceViewImpl*>(textureView);
    slot.sampler = static_cast<SamplerStateImpl*>(sampler);
    return SLANG_OK;
}

Result ShaderObjectImpl::init(IDevice* device, ShaderObjectLayoutImpl* layout)
{
    m_layout = layout;

    m_constantBufferTransientHeap = nullptr;
    m_constantBufferTransientHeapVersion = 0;
    m_isConstantBufferDirty = true;

    // If the layout tells us that there is any uniform data,
    // then we will allocate a CPU memory buffer to hold that data
    // while it is being set from the host.
    //
    // Once the user is done setting the parameters/fields of this
    // shader object, we will produce a GPU-memory version of the
    // uniform data (which includes values from this object and
    // any existential-type sub-objects).
    //
    size_t uniformSize = layout->getElementTypeLayout()->getSize();
    if (uniformSize)
    {
        m_data.setCount(uniformSize);
        memset(m_data.getBuffer(), 0, uniformSize);
    }

#if 0
        // If the layout tells us there are any descriptor sets to
        // allocate, then we do so now.
        //
        for(auto descriptorSetInfo : layout->getDescriptorSets())
        {
            RefPtr<DescriptorSet> descriptorSet;
            SLANG_RETURN_ON_FAIL(renderer->createDescriptorSet(descriptorSetInfo->layout, descriptorSet.writeRef()));
            m_descriptorSets.add(descriptorSet);
        }
#endif

    m_resourceViews.setCount(layout->getResourceViewCount());
    m_samplers.setCount(layout->getSamplerCount());
    m_combinedTextureSamplers.setCount(layout->getCombinedTextureSamplerCount());

    // If the layout specifies that we have any sub-objects, then
    // we need to size the array to account for them.
    //
    Index subObjectCount = layout->getSubObjectCount();
    m_objects.setCount(subObjectCount);

    for (auto subObjectRangeInfo : layout->getSubObjectRanges())
    {
        auto subObjectLayout = subObjectRangeInfo.layout;

        // In the case where the sub-object range represents an
        // existential-type leaf field (e.g., an `IBar`), we
        // cannot pre-allocate the object(s) to go into that
        // range, since we can't possibly know what to allocate
        // at this point.
        //
        if (!subObjectLayout)
            continue;
        //
        // Otherwise, we will allocate a sub-object to fill
        // in each entry in this range, based on the layout
        // information we already have.

        auto& bindingRangeInfo = layout->getBindingRange(subObjectRangeInfo.bindingRangeIndex);
        for (Index i = 0; i < bindingRangeInfo.count; ++i)
        {
            RefPtr<ShaderObjectImpl> subObject;
            SLANG_RETURN_ON_FAIL(
                ShaderObjectImpl::create(device, subObjectLayout, subObject.writeRef()));
            m_objects[bindingRangeInfo.subObjectIndex + i] = subObject;
        }
    }

    return SLANG_OK;
}

Result ShaderObjectImpl::_writeOrdinaryData(
    PipelineCommandEncoder* encoder,
    IBufferResource* buffer,
    size_t offset,
    size_t destSize,
    ShaderObjectLayoutImpl* specializedLayout)
{
    auto src = m_data.getBuffer();
    auto srcSize = size_t(m_data.getCount());

    SLANG_ASSERT(srcSize <= destSize);

    encoder->uploadBufferDataImpl(buffer, offset, srcSize, src);

    // In the case where this object has any sub-objects of
    // existential/interface type, we need to recurse on those objects
    // that need to write their state into an appropriate "pending" allocation.
    //
    // Note: Any values that could fit into the "payload" included
    // in the existential-type field itself will have already been
    // written as part of `setObject()`. This loop only needs to handle
    // those sub-objects that do not "fit."
    //
    // An implementers looking at this code might wonder if things could be changed
    // so that *all* writes related to sub-objects for interface-type fields could
    // be handled in this one location, rather than having some in `setObject()` and
    // others handled here.
    //
    Index subObjectRangeCounter = 0;
    for (auto const& subObjectRangeInfo : specializedLayout->getSubObjectRanges())
    {
        Index subObjectRangeIndex = subObjectRangeCounter++;
        auto const& bindingRangeInfo =
            specializedLayout->getBindingRange(subObjectRangeInfo.bindingRangeIndex);

        // We only need to handle sub-object ranges for interface/existential-type fields,
        // because fields of constant-buffer or parameter-block type are responsible for
        // the ordinary/uniform data of their own existential/interface-type sub-objects.
        //
        if (bindingRangeInfo.bindingType != slang::BindingType::ExistentialValue)
            continue;

        // Each sub-object range represents a single "leaf" field, but might be nested
        // under zero or more outer arrays, such that the number of existential values
        // in the same range can be one or more.
        //
        auto count = bindingRangeInfo.count;

        // We are not concerned with the case where the existential value(s) in the range
        // git into the payload part of the leaf field.
        //
        // In the case where the value didn't fit, the Slang layout strategy would have
        // considered the requirements of the value as a "pending" allocation, and would
        // allocate storage for the ordinary/uniform part of that pending allocation inside
        // of the parent object's type layout.
        //
        // Here we assume that the Slang reflection API can provide us with a single byte
        // offset and stride for the location of the pending data allocation in the
        // specialized type layout, which will store the values for this sub-object range.
        //
        // TODO: The reflection API functions we are assuming here haven't been implemented
        // yet, so the functions being called here are stubs.
        //
        // TODO: It might not be that a single sub-object range can reliably map to a single
        // contiguous array with a single stride; we need to carefully consider what the
        // layout logic does for complex cases with multiple layers of nested arrays and
        // structures.
        //
        size_t subObjectRangePendingDataOffset = subObjectRangeInfo.offset.pendingOrdinaryData;
        size_t subObjectRangePendingDataStride = subObjectRangeInfo.stride.pendingOrdinaryData;

        // If the range doesn't actually need/use the "pending" allocation at all, then
        // we need to detect that case and skip such ranges.
        //
        // TODO: This should probably be handled on a per-object basis by caching a "does it
        // fit?" bit as part of the information for bound sub-objects, given that we already
        // compute the "does it fit?" status as part of `setObject()`.
        //
        if (subObjectRangePendingDataOffset == 0)
            continue;

        for (Slang::Index i = 0; i < count; ++i)
        {
            auto subObject = m_objects[bindingRangeInfo.subObjectIndex + i];

            RefPtr<ShaderObjectLayoutImpl> subObjectLayout;
            SLANG_RETURN_ON_FAIL(subObject->_getSpecializedLayout(subObjectLayout.writeRef()));

            auto subObjectOffset =
                subObjectRangePendingDataOffset + i * subObjectRangePendingDataStride;

            subObject->_writeOrdinaryData(
                encoder,
                buffer,
                offset + subObjectOffset,
                destSize - subObjectOffset,
                subObjectLayout);
        }
    }

    return SLANG_OK;
}

void ShaderObjectImpl::writeDescriptor(
    RootBindingContext& context, VkWriteDescriptorSet const& write)
{
    auto device = context.device;
    device->m_api.vkUpdateDescriptorSets(device->m_device, 1, &write, 0, nullptr);
}

void ShaderObjectImpl::writeBufferDescriptor(
    RootBindingContext& context,
    BindingOffset const& offset,
    VkDescriptorType descriptorType,
    BufferResourceImpl* buffer,
    size_t bufferOffset,
    size_t bufferSize)
{
    auto descriptorSet = context.descriptorSets[offset.bindingSet];

    VkDescriptorBufferInfo bufferInfo = {};
    if (buffer)
    {
        bufferInfo.buffer = buffer->m_buffer.m_buffer;
    }
    bufferInfo.offset = bufferOffset;
    bufferInfo.range = bufferSize;

    VkWriteDescriptorSet write = {};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.descriptorCount = 1;
    write.descriptorType = descriptorType;
    write.dstArrayElement = 0;
    write.dstBinding = offset.binding;
    write.dstSet = descriptorSet;
    write.pBufferInfo = &bufferInfo;

    writeDescriptor(context, write);
}

void ShaderObjectImpl::writeBufferDescriptor(
    RootBindingContext& context,
    BindingOffset const& offset,
    VkDescriptorType descriptorType,
    BufferResourceImpl* buffer)
{
    writeBufferDescriptor(
        context, offset, descriptorType, buffer, 0, buffer->getDesc()->sizeInBytes);
}

void ShaderObjectImpl::writePlainBufferDescriptor(
    RootBindingContext& context,
    BindingOffset const& offset,
    VkDescriptorType descriptorType,
    ArrayView<RefPtr<ResourceViewInternalBase>> resourceViews)
{
    auto descriptorSet = context.descriptorSets[offset.bindingSet];

    Index count = resourceViews.getCount();
    for (Index i = 0; i < count; ++i)
    {
        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.range = VK_WHOLE_SIZE;

        if (resourceViews[i])
        {
            auto boundViewType = static_cast<ResourceViewImpl*>(resourceViews[i].Ptr())->m_type;
            if (boundViewType == ResourceViewImpl::ViewType::PlainBuffer)
            {
                auto bufferView = static_cast<PlainBufferResourceViewImpl*>(resourceViews[i].Ptr());
                bufferInfo.buffer = bufferView->m_buffer->m_buffer.m_buffer;
                bufferInfo.offset = bufferView->offset;
                bufferInfo.range = bufferView->size;
            }
        }

        VkWriteDescriptorSet write = {};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorCount = 1;
        write.descriptorType = descriptorType;
        write.dstArrayElement = uint32_t(i);
        write.dstBinding = offset.binding;
        write.dstSet = descriptorSet;
        write.pBufferInfo = &bufferInfo;

        writeDescriptor(context, write);
    }
}

void ShaderObjectImpl::writeTexelBufferDescriptor(
    RootBindingContext& context,
    BindingOffset const& offset,
    VkDescriptorType descriptorType,
    ArrayView<RefPtr<ResourceViewInternalBase>> resourceViews)
{
    auto descriptorSet = context.descriptorSets[offset.bindingSet];

    Index count = resourceViews.getCount();
    for (Index i = 0; i < count; ++i)
    {
        VkBufferView bufferView = VK_NULL_HANDLE;
        if (resourceViews[i])
        {
            auto boundViewType = static_cast<ResourceViewImpl*>(resourceViews[i].Ptr())->m_type;
            if (boundViewType == ResourceViewImpl::ViewType::TexelBuffer)
            {
                auto resourceView =
                    static_cast<TexelBufferResourceViewImpl*>(resourceViews[i].Ptr());
                bufferView = resourceView->m_view;
            }
        }
        VkWriteDescriptorSet write = {};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType = descriptorType;
        write.dstArrayElement = uint32_t(i);
        write.dstBinding = offset.binding;
        write.dstSet = descriptorSet;
        write.descriptorCount = 1;
        write.pTexelBufferView = &bufferView;
        writeDescriptor(context, write);
    }
}

void ShaderObjectImpl::writeTextureSamplerDescriptor(
    RootBindingContext& context,
    BindingOffset const& offset,
    VkDescriptorType descriptorType,
    ArrayView<CombinedTextureSamplerSlot> slots)
{
    auto descriptorSet = context.descriptorSets[offset.bindingSet];

    Index count = slots.getCount();
    for (Index i = 0; i < count; ++i)
    {
        auto texture = slots[i].textureView;
        auto sampler = slots[i].sampler;
        VkDescriptorImageInfo imageInfo = {};
        if (texture)
        {
            imageInfo.imageView = texture->m_view;
            imageInfo.imageLayout = texture->m_layout;
        }
        if (sampler)
        {
            imageInfo.sampler = sampler->m_sampler;
        }
        else
        {
            imageInfo.sampler = context.device->m_defaultSampler;
        }

        VkWriteDescriptorSet write = {};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorCount = 1;
        write.descriptorType = descriptorType;
        write.dstArrayElement = uint32_t(i);
        write.dstBinding = offset.binding;
        write.dstSet = descriptorSet;
        write.pImageInfo = &imageInfo;

        writeDescriptor(context, write);
    }
}

void ShaderObjectImpl::writeAccelerationStructureDescriptor(
    RootBindingContext& context,
    BindingOffset const& offset,
    VkDescriptorType descriptorType,
    ArrayView<RefPtr<ResourceViewInternalBase>> resourceViews)
{
    auto descriptorSet = context.descriptorSets[offset.bindingSet];

    Index count = resourceViews.getCount();
    for (Index i = 0; i < count; ++i)
    {
        auto accelerationStructure =
            static_cast<AccelerationStructureImpl*>(resourceViews[i].Ptr());
        VkWriteDescriptorSetAccelerationStructureKHR writeAS = {};
        writeAS.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
        VkAccelerationStructureKHR nullHandle = VK_NULL_HANDLE;
        if (accelerationStructure)
        {
            writeAS.accelerationStructureCount = 1;
            writeAS.pAccelerationStructures = &accelerationStructure->m_vkHandle;
        }
        else
        {
            writeAS.accelerationStructureCount = 1;
            writeAS.pAccelerationStructures = &nullHandle;
        }
        VkWriteDescriptorSet write = {};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorCount = 1;
        write.descriptorType = descriptorType;
        write.dstArrayElement = uint32_t(i);
        write.dstBinding = offset.binding;
        write.dstSet = descriptorSet;
        write.pNext = &writeAS;
        writeDescriptor(context, write);
    }
}

void ShaderObjectImpl::writeTextureDescriptor(
    RootBindingContext& context,
    BindingOffset const& offset,
    VkDescriptorType descriptorType,
    ArrayView<RefPtr<ResourceViewInternalBase>> resourceViews)
{
    auto descriptorSet = context.descriptorSets[offset.bindingSet];

    Index count = resourceViews.getCount();
    for (Index i = 0; i < count; ++i)
    {
        VkDescriptorImageInfo imageInfo = {};
        if (resourceViews[i])
        {
            auto boundViewType = static_cast<ResourceViewImpl*>(resourceViews[i].Ptr())->m_type;
            if (boundViewType == ResourceViewImpl::ViewType::Texture)
            {
                auto texture = static_cast<TextureResourceViewImpl*>(resourceViews[i].Ptr());
                imageInfo.imageView = texture->m_view;
                imageInfo.imageLayout = texture->m_layout;
            }
        }
        imageInfo.sampler = 0;

        VkWriteDescriptorSet write = {};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorCount = 1;
        write.descriptorType = descriptorType;
        write.dstArrayElement = uint32_t(i);
        write.dstBinding = offset.binding;
        write.dstSet = descriptorSet;
        write.pImageInfo = &imageInfo;

        writeDescriptor(context, write);
    }
}

void ShaderObjectImpl::writeSamplerDescriptor(
    RootBindingContext& context,
    BindingOffset const& offset,
    VkDescriptorType descriptorType,
    ArrayView<RefPtr<SamplerStateImpl>> samplers)
{
    auto descriptorSet = context.descriptorSets[offset.bindingSet];

    Index count = samplers.getCount();
    for (Index i = 0; i < count; ++i)
    {
        auto sampler = samplers[i];
        VkDescriptorImageInfo imageInfo = {};
        imageInfo.imageView = 0;
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        if (sampler)
        {
            imageInfo.sampler = sampler->m_sampler;
        }
        else
        {
            imageInfo.sampler = context.device->m_defaultSampler;
        }

        VkWriteDescriptorSet write = {};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorCount = 1;
        write.descriptorType = descriptorType;
        write.dstArrayElement = uint32_t(i);
        write.dstBinding = offset.binding;
        write.dstSet = descriptorSet;
        write.pImageInfo = &imageInfo;

        writeDescriptor(context, write);
    }
}

bool ShaderObjectImpl::shouldAllocateConstantBuffer(TransientResourceHeapImpl* transientHeap)
{
    return m_isConstantBufferDirty || m_constantBufferTransientHeap != transientHeap ||
           m_constantBufferTransientHeapVersion != transientHeap->getVersion();
}

Result ShaderObjectImpl::_ensureOrdinaryDataBufferCreatedIfNeeded(
    PipelineCommandEncoder* encoder, ShaderObjectLayoutImpl* specializedLayout)
{
    // If data has been changed since last allocation/filling of constant buffer,
    // we will need to allocate a new one.
    //
    if (!shouldAllocateConstantBuffer(encoder->m_commandBuffer->m_transientHeap))
    {
        return SLANG_OK;
    }
    m_isConstantBufferDirty = false;
    m_constantBufferTransientHeap = encoder->m_commandBuffer->m_transientHeap;
    m_constantBufferTransientHeapVersion = encoder->m_commandBuffer->m_transientHeap->getVersion();

    m_constantBufferSize = specializedLayout->getTotalOrdinaryDataSize();
    if (m_constantBufferSize == 0)
    {
        return SLANG_OK;
    }

    // Once we have computed how large the buffer should be, we can allocate
    // it from the transient resource heap.
    //
    SLANG_RETURN_ON_FAIL(encoder->m_commandBuffer->m_transientHeap->allocateConstantBuffer(
        m_constantBufferSize, m_constantBuffer, m_constantBufferOffset));

    // Once the buffer is allocated, we can use `_writeOrdinaryData` to fill it in.
    //
    // Note that `_writeOrdinaryData` is potentially recursive in the case
    // where this object contains interface/existential-type fields, so we
    // don't need or want to inline it into this call site.
    //
    SLANG_RETURN_ON_FAIL(_writeOrdinaryData(
        encoder,
        m_constantBuffer,
        m_constantBufferOffset,
        m_constantBufferSize,
        specializedLayout));

    return SLANG_OK;
}

Result ShaderObjectImpl::bindAsValue(
    PipelineCommandEncoder* encoder,
    RootBindingContext& context,
    BindingOffset const& offset,
    ShaderObjectLayoutImpl* specializedLayout)
{
    // We start by iterating over the "simple" (non-sub-object) binding
    // ranges and writing them to the descriptor sets that are being
    // passed down.
    //
    for (auto bindingRangeInfo : specializedLayout->getBindingRanges())
    {
        BindingOffset rangeOffset = offset;

        auto baseIndex = bindingRangeInfo.baseIndex;
        auto count = (uint32_t)bindingRangeInfo.count;
        switch (bindingRangeInfo.bindingType)
        {
        case slang::BindingType::ConstantBuffer:
        case slang::BindingType::ParameterBlock:
        case slang::BindingType::ExistentialValue:
            break;

        case slang::BindingType::Texture:
            rangeOffset.bindingSet += bindingRangeInfo.setOffset;
            rangeOffset.binding += bindingRangeInfo.bindingOffset;
            writeTextureDescriptor(
                context,
                rangeOffset,
                VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                m_resourceViews.getArrayView(baseIndex, count));
            break;
        case slang::BindingType::MutableTexture:
            rangeOffset.bindingSet += bindingRangeInfo.setOffset;
            rangeOffset.binding += bindingRangeInfo.bindingOffset;
            writeTextureDescriptor(
                context,
                rangeOffset,
                VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                m_resourceViews.getArrayView(baseIndex, count));
            break;
        case slang::BindingType::CombinedTextureSampler:
            rangeOffset.bindingSet += bindingRangeInfo.setOffset;
            rangeOffset.binding += bindingRangeInfo.bindingOffset;
            writeTextureSamplerDescriptor(
                context,
                rangeOffset,
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                m_combinedTextureSamplers.getArrayView(baseIndex, count));
            break;

        case slang::BindingType::Sampler:
            rangeOffset.bindingSet += bindingRangeInfo.setOffset;
            rangeOffset.binding += bindingRangeInfo.bindingOffset;
            writeSamplerDescriptor(
                context,
                rangeOffset,
                VK_DESCRIPTOR_TYPE_SAMPLER,
                m_samplers.getArrayView(baseIndex, count));
            break;

        case slang::BindingType::RawBuffer:
        case slang::BindingType::MutableRawBuffer:
            rangeOffset.bindingSet += bindingRangeInfo.setOffset;
            rangeOffset.binding += bindingRangeInfo.bindingOffset;
            writePlainBufferDescriptor(
                context,
                rangeOffset,
                VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                m_resourceViews.getArrayView(baseIndex, count));
            break;

        case slang::BindingType::TypedBuffer:
            rangeOffset.bindingSet += bindingRangeInfo.setOffset;
            rangeOffset.binding += bindingRangeInfo.bindingOffset;
            writeTexelBufferDescriptor(
                context,
                rangeOffset,
                VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,
                m_resourceViews.getArrayView(baseIndex, count));
            break;
        case slang::BindingType::MutableTypedBuffer:
            rangeOffset.bindingSet += bindingRangeInfo.setOffset;
            rangeOffset.binding += bindingRangeInfo.bindingOffset;
            writeTexelBufferDescriptor(
                context,
                rangeOffset,
                VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,
                m_resourceViews.getArrayView(baseIndex, count));
            break;
        case slang::BindingType::RayTracingAccelerationStructure:
            rangeOffset.bindingSet += bindingRangeInfo.setOffset;
            rangeOffset.binding += bindingRangeInfo.bindingOffset;
            writeAccelerationStructureDescriptor(
                context,
                rangeOffset,
                VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
                m_resourceViews.getArrayView(baseIndex, count));
            break;
        case slang::BindingType::VaryingInput:
        case slang::BindingType::VaryingOutput:
            break;

        default:
            SLANG_ASSERT(!"unsupported binding type");
            return SLANG_FAIL;
            break;
        }
    }

    // Once we've handled the simple binding ranges, we move on to the
    // sub-object ranges, which are generally more involved.
    //
    for (auto const& subObjectRange : specializedLayout->getSubObjectRanges())
    {
        auto const& bindingRangeInfo =
            specializedLayout->getBindingRange(subObjectRange.bindingRangeIndex);
        auto count = bindingRangeInfo.count;
        auto subObjectIndex = bindingRangeInfo.subObjectIndex;

        auto subObjectLayout = subObjectRange.layout;

        // The starting offset to use for the sub-object
        // has already been computed and stored as part
        // of the layout, so we can get to the starting
        // offset for the range easily.
        //
        BindingOffset rangeOffset = offset;
        rangeOffset += subObjectRange.offset;

        BindingOffset rangeStride = subObjectRange.stride;

        switch (bindingRangeInfo.bindingType)
        {
        case slang::BindingType::ConstantBuffer:
            {
                BindingOffset objOffset = rangeOffset;
                for (Index i = 0; i < count; ++i)
                {
                    // Binding a constant buffer sub-object is simple enough:
                    // we just call `bindAsConstantBuffer` on it to bind
                    // the ordinary data buffer (if needed) and any other
                    // bindings it recursively contains.
                    //
                    ShaderObjectImpl* subObject = m_objects[subObjectIndex + i];
                    subObject->bindAsConstantBuffer(encoder, context, objOffset, subObjectLayout);

                    // When dealing with arrays of sub-objects, we need to make
                    // sure to increment the offset for each subsequent object
                    // by the appropriate stride.
                    //
                    objOffset += rangeStride;
                }
            }
            break;
        case slang::BindingType::ParameterBlock:
            {
                BindingOffset objOffset = rangeOffset;
                for (Index i = 0; i < count; ++i)
                {
                    // The case for `ParameterBlock<X>` is not that different
                    // from `ConstantBuffer<X>`, except that we call `bindAsParameterBlock`
                    // instead (understandably).
                    //
                    ShaderObjectImpl* subObject = m_objects[subObjectIndex + i];
                    subObject->bindAsParameterBlock(encoder, context, objOffset, subObjectLayout);
                }
            }
            break;

        case slang::BindingType::ExistentialValue:
            // Interface/existential-type sub-object ranges are the most complicated case.
            //
            // First, we can only bind things if we have static specialization information
            // to work with, which is exactly the case where `subObjectLayout` will be
            // non-null.
            //
            if (subObjectLayout)
            {
                // Second, the offset where we want to start binding for existential-type
                // ranges is a bit different, because we don't wnat to bind at the "primary"
                // offset that got passed down, but instead at the "pending" offset.
                //
                // For the purposes of nested binding, what used to be the pending offset
                // will now be used as the primary offset.
                //
                SimpleBindingOffset objOffset = rangeOffset.pending;
                SimpleBindingOffset objStride = rangeStride.pending;
                for (Index i = 0; i < count; ++i)
                {
                    // An existential-type sub-object is always bound just as a value,
                    // which handles its nested bindings and descriptor sets, but
                    // does not deal with ordianry data. The ordinary data should
                    // have been handled as part of the buffer for a parent object
                    // already.
                    //
                    ShaderObjectImpl* subObject = m_objects[subObjectIndex + i];
                    subObject->bindAsValue(
                        encoder, context, BindingOffset(objOffset), subObjectLayout);
                    objOffset += objStride;
                }
            }
            break;
        case slang::BindingType::RawBuffer:
        case slang::BindingType::MutableRawBuffer:
            // No action needed for sub-objects bound though a `StructuredBuffer`.
            break;
        default:
            SLANG_ASSERT(!"unsupported sub-object type");
            return SLANG_FAIL;
            break;
        }
    }

    return SLANG_OK;
}

Result ShaderObjectImpl::allocateDescriptorSets(
    PipelineCommandEncoder* encoder,
    RootBindingContext& context,
    BindingOffset const& offset,
    ShaderObjectLayoutImpl* specializedLayout)
{
    assert(specializedLayout->getOwnDescriptorSets().getCount() <= 1);
    // The number of sets to allocate and their layouts was already pre-computed
    // as part of the shader object layout, so we use that information here.
    //
    for (auto descriptorSetInfo : specializedLayout->getOwnDescriptorSets())
    {
        auto descriptorSetHandle =
            context.descriptorSetAllocator->allocate(descriptorSetInfo.descriptorSetLayout).handle;

        // For each set, we need to write it into the set of descriptor sets
        // being used for binding. This is done both so that other steps
        // in binding can find the set to fill it in, but also so that
        // we can bind all the descriptor sets to the pipeline when the
        // time comes.
        //
        context.descriptorSets[context.descriptorSetCounter] = descriptorSetHandle;
        context.descriptorSetCounter++;
    }

    return SLANG_OK;
}

Result ShaderObjectImpl::bindAsParameterBlock(
    PipelineCommandEncoder* encoder,
    RootBindingContext& context,
    BindingOffset const& inOffset,
    ShaderObjectLayoutImpl* specializedLayout)
{
    // Because we are binding into a nested parameter block,
    // any texture/buffer/sampler bindings will now want to
    // write into the sets we allocate for this object and
    // not the sets for any parent object(s).
    //
    BindingOffset offset = inOffset;
    offset.bindingSet = context.descriptorSetCounter;
    offset.binding = 0;

    // TODO: We should also be writing to `offset.pending` here,
    // because any resource/sampler bindings related to "pending"
    // data should *also* be writing into the chosen set.
    //
    // The challenge here is that we need to compute the right
    // value for `offset.pending.binding`, so that it writes after
    // all the other bindings.

    // Writing the bindings for a parameter block is relatively easy:
    // we just need to allocate the descriptor set(s) needed for this
    // object and then fill it in like a `ConstantBuffer<X>`.
    //
    SLANG_RETURN_ON_FAIL(allocateDescriptorSets(encoder, context, offset, specializedLayout));

    assert(offset.bindingSet < context.descriptorSetCounter);
    SLANG_RETURN_ON_FAIL(bindAsConstantBuffer(encoder, context, offset, specializedLayout));

    return SLANG_OK;
}

Result ShaderObjectImpl::bindOrdinaryDataBufferIfNeeded(
    PipelineCommandEncoder* encoder,
    RootBindingContext& context,
    BindingOffset& ioOffset,
    ShaderObjectLayoutImpl* specializedLayout)
{
    // We start by ensuring that the buffer is created, if it is needed.
    //
    SLANG_RETURN_ON_FAIL(_ensureOrdinaryDataBufferCreatedIfNeeded(encoder, specializedLayout));

    // If we did indeed need/create a buffer, then we must bind it into
    // the given `descriptorSet` and update the base range index for
    // subsequent binding operations to account for it.
    //
    if (m_constantBuffer)
    {
        auto bufferImpl = static_cast<BufferResourceImpl*>(m_constantBuffer);
        writeBufferDescriptor(
            context,
            ioOffset,
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            bufferImpl,
            m_constantBufferOffset,
            m_constantBufferSize);
        ioOffset.binding++;
    }

    return SLANG_OK;
}

Result ShaderObjectImpl::bindAsConstantBuffer(
    PipelineCommandEncoder* encoder,
    RootBindingContext& context,
    BindingOffset const& inOffset,
    ShaderObjectLayoutImpl* specializedLayout)
{
    // To bind an object as a constant buffer, we first
    // need to bind its ordinary data (if any) into an
    // ordinary data buffer, and then bind it as a "value"
    // which handles any of its recursively-contained bindings.
    //
    // The one detail is taht when binding the ordinary data
    // buffer we need to adjust the `binding` index used for
    // subsequent operations based on whether or not an ordinary
    // data buffer was used (and thus consumed a `binding`).
    //
    BindingOffset offset = inOffset;
    SLANG_RETURN_ON_FAIL(
        bindOrdinaryDataBufferIfNeeded(encoder, context, /*inout*/ offset, specializedLayout));
    SLANG_RETURN_ON_FAIL(bindAsValue(encoder, context, offset, specializedLayout));
    return SLANG_OK;
}

Result ShaderObjectImpl::_getSpecializedLayout(ShaderObjectLayoutImpl** outLayout)
{
    if (!m_specializedLayout)
    {
        SLANG_RETURN_ON_FAIL(_createSpecializedLayout(m_specializedLayout.writeRef()));
    }
    returnRefPtr(outLayout, m_specializedLayout);
    return SLANG_OK;
}

Result ShaderObjectImpl::_createSpecializedLayout(ShaderObjectLayoutImpl** outLayout)
{
    ExtendedShaderObjectType extendedType;
    SLANG_RETURN_ON_FAIL(getSpecializedShaderObjectType(&extendedType));

    auto device = getDevice();
    RefPtr<ShaderObjectLayoutImpl> layout;
    SLANG_RETURN_ON_FAIL(device->getShaderObjectLayout(
        extendedType.slangType,
        m_layout->getContainerType(),
        (ShaderObjectLayoutBase**)layout.writeRef()));

    returnRefPtrMove(outLayout, layout);
    return SLANG_OK;
}

Result EntryPointShaderObject::create(
    IDevice* device, EntryPointLayout* layout, EntryPointShaderObject** outShaderObject)
{
    RefPtr<EntryPointShaderObject> object = new EntryPointShaderObject();
    SLANG_RETURN_ON_FAIL(object->init(device, layout));

    returnRefPtrMove(outShaderObject, object);
    return SLANG_OK;
}

EntryPointLayout* EntryPointShaderObject::getLayout()
{
    return static_cast<EntryPointLayout*>(m_layout.Ptr());
}

Result EntryPointShaderObject::bindAsEntryPoint(
    PipelineCommandEncoder* encoder,
    RootBindingContext& context,
    BindingOffset const& inOffset,
    EntryPointLayout* layout)
{
    BindingOffset offset = inOffset;

    // Any ordinary data in an entry point is assumed to be allocated
    // as a push-constant range.
    //
    // TODO: Can we make this operation not bake in that assumption?
    //
    // TODO: Can/should this function be renamed as just `bindAsPushConstantBuffer`?
    //
    if (m_data.getCount())
    {
        // The index of the push constant range to bind should be
        // passed down as part of the `offset`, and we will increment
        // it here so that any further recursively-contained push-constant
        // ranges use the next index.
        //
        auto pushConstantRangeIndex = offset.pushConstantRange++;

        // Information about the push constant ranges (including offsets
        // and stage flags) was pre-computed for the entire program and
        // stored on the binding context.
        //
        auto const& pushConstantRange = context.pushConstantRanges[pushConstantRangeIndex];

        // We expect that the size of the range as reflected matches the
        // amount of ordinary data stored on this object.
        //
        // TODO: This would not be the case if specialization for interface-type
        // parameters led to the entry point having "pending" ordinary data.
        //
        SLANG_ASSERT(pushConstantRange.size == (uint32_t)m_data.getCount());

        auto pushConstantData = m_data.getBuffer();

        encoder->m_api->vkCmdPushConstants(
            encoder->m_commandBuffer->m_commandBuffer,
            context.pipelineLayout,
            pushConstantRange.stageFlags,
            pushConstantRange.offset,
            pushConstantRange.size,
            pushConstantData);
    }

    // Any remaining bindings in the object can be handled through the
    // "value" case.
    //
    SLANG_RETURN_ON_FAIL(bindAsValue(encoder, context, offset, layout));
    return SLANG_OK;
}

Result EntryPointShaderObject::init(IDevice* device, EntryPointLayout* layout)
{
    SLANG_RETURN_ON_FAIL(Super::init(device, layout));
    return SLANG_OK;
}

RootShaderObjectLayout* RootShaderObjectImpl::getLayout()
{
    return static_cast<RootShaderObjectLayout*>(m_layout.Ptr());
}

RootShaderObjectLayout* RootShaderObjectImpl::getSpecializedLayout()
{
    RefPtr<ShaderObjectLayoutImpl> specializedLayout;
    _getSpecializedLayout(specializedLayout.writeRef());
    return static_cast<RootShaderObjectLayout*>(m_specializedLayout.Ptr());
}

List<RefPtr<EntryPointShaderObject>> const& RootShaderObjectImpl::getEntryPoints() const
{
    return m_entryPoints;
}

UInt RootShaderObjectImpl::getEntryPointCount() { return (UInt)m_entryPoints.getCount(); }

Result RootShaderObjectImpl::getEntryPoint(UInt index, IShaderObject** outEntryPoint)
{
    returnComPtr(outEntryPoint, m_entryPoints[index]);
    return SLANG_OK;
}

Result RootShaderObjectImpl::copyFrom(IShaderObject* object, ITransientResourceHeap* transientHeap)
{
    SLANG_RETURN_ON_FAIL(Super::copyFrom(object, transientHeap));
    if (auto srcObj = dynamic_cast<MutableRootShaderObject*>(object))
    {
        for (Index i = 0; i < srcObj->m_entryPoints.getCount(); i++)
        {
            m_entryPoints[i]->copyFrom(srcObj->m_entryPoints[i], transientHeap);
        }
        return SLANG_OK;
    }
    return SLANG_FAIL;
}

Result RootShaderObjectImpl::bindAsRoot(
    PipelineCommandEncoder* encoder, RootBindingContext& context, RootShaderObjectLayout* layout)
{
    BindingOffset offset = {};
    offset.pending = layout->getPendingDataOffset();

    // Note: the operations here are quite similar to what `bindAsParameterBlock` does.
    // The key difference in practice is that we do *not* make use of the adjustment
    // that `bindOrdinaryDataBufferIfNeeded` applied to the offset passed into it.
    //
    // The reason for this difference in behavior is that the layout information
    // for root shader parameters is in practice *already* offset appropriately
    // (so that it ends up using absolute offsets).
    //
    // TODO: One more wrinkle here is that the `ordinaryDataBufferOffset` below
    // might not be correct if `binding=0,set=0` was already claimed via explicit
    // binding information. We should really be getting the offset information for
    // the ordinary data buffer directly from the reflection information for
    // the global scope.

    SLANG_RETURN_ON_FAIL(allocateDescriptorSets(encoder, context, offset, layout));

    BindingOffset ordinaryDataBufferOffset = offset;
    SLANG_RETURN_ON_FAIL(
        bindOrdinaryDataBufferIfNeeded(encoder, context, ordinaryDataBufferOffset, layout));

    SLANG_RETURN_ON_FAIL(bindAsValue(encoder, context, offset, layout));

    auto entryPointCount = layout->getEntryPoints().getCount();
    for (Index i = 0; i < entryPointCount; ++i)
    {
        auto entryPoint = m_entryPoints[i];
        auto const& entryPointInfo = layout->getEntryPoint(i);

        // Note: we do *not* need to add the entry point offset
        // information to the global `offset` because the
        // `RootShaderObjectLayout` has already baked any offsets
        // from the global layout into the `entryPointInfo`.

        entryPoint->bindAsEntryPoint(
            encoder, context, entryPointInfo.offset, entryPointInfo.layout);
    }

    return SLANG_OK;
}

Result RootShaderObjectImpl::collectSpecializationArgs(ExtendedShaderObjectTypeList& args)
{
    SLANG_RETURN_ON_FAIL(ShaderObjectImpl::collectSpecializationArgs(args));
    for (auto& entryPoint : m_entryPoints)
    {
        SLANG_RETURN_ON_FAIL(entryPoint->collectSpecializationArgs(args));
    }
    return SLANG_OK;
}

Result RootShaderObjectImpl::init(IDevice* device, RootShaderObjectLayout* layout)
{
    SLANG_RETURN_ON_FAIL(Super::init(device, layout));
    m_specializedLayout = nullptr;
    m_entryPoints.clear();
    for (auto entryPointInfo : layout->getEntryPoints())
    {
        RefPtr<EntryPointShaderObject> entryPoint;
        SLANG_RETURN_ON_FAIL(
            EntryPointShaderObject::create(device, entryPointInfo.layout, entryPoint.writeRef()));
        m_entryPoints.add(entryPoint);
    }

    return SLANG_OK;
}

Result RootShaderObjectImpl::_createSpecializedLayout(ShaderObjectLayoutImpl** outLayout)
{
    ExtendedShaderObjectTypeList specializationArgs;
    SLANG_RETURN_ON_FAIL(collectSpecializationArgs(specializationArgs));

    // Note: There is an important policy decision being made here that we need
    // to approach carefully.
    //
    // We are doing two different things that affect the layout of a program:
    //
    // 1. We are *composing* one or more pieces of code (notably the shared global/module
    //    stuff and the per-entry-point stuff).
    //
    // 2. We are *specializing* code that includes generic/existential parameters
    //    to concrete types/values.
    //
    // We need to decide the relative *order* of these two steps, because of how it impacts
    // layout. The layout for `specialize(compose(A,B), X, Y)` is potentially different
    // form that of `compose(specialize(A,X), speciealize(B,Y))`, even when both are
    // semantically equivalent programs.
    //
    // Right now we are using the first option: we are first generating a full composition
    // of all the code we plan to use (global scope plus all entry points), and then
    // specializing it to the concatenated specialization argumenst for all of that.
    //
    // In some cases, though, this model isn't appropriate. For example, when dealing with
    // ray-tracing shaders and local root signatures, we really want the parameters of each
    // entry point (actually, each entry-point *group*) to be allocated distinct storage,
    // which really means we want to compute something like:
    //
    //      SpecializedGlobals = specialize(compose(ModuleA, ModuleB, ...), X, Y, ...)
    //
    //      SpecializedEP1 = compose(SpecializedGlobals, specialize(EntryPoint1, T, U, ...))
    //      SpecializedEP2 = compose(SpecializedGlobals, specialize(EntryPoint2, A, B, ...))
    //
    // Note how in this case all entry points agree on the layout for the shared/common
    // parmaeters, but their layouts are also independent of one another.
    //
    // Furthermore, in this example, loading another entry point into the system would not
    // rquire re-computing the layouts (or generated kernel code) for any of the entry
    // points that had already been loaded (in contrast to a compose-then-specialize
    // approach).
    //
    ComPtr<slang::IComponentType> specializedComponentType;
    ComPtr<slang::IBlob> diagnosticBlob;
    auto result = getLayout()->getSlangProgram()->specialize(
        specializationArgs.components.getArrayView().getBuffer(),
        specializationArgs.getCount(),
        specializedComponentType.writeRef(),
        diagnosticBlob.writeRef());

    // TODO: print diagnostic message via debug output interface.

    if (result != SLANG_OK)
        return result;

    auto slangSpecializedLayout = specializedComponentType->getLayout();
    RefPtr<RootShaderObjectLayout> specializedLayout;
    RootShaderObjectLayout::create(
        static_cast<DeviceImpl*>(getRenderer()),
        specializedComponentType,
        slangSpecializedLayout,
        specializedLayout.writeRef());

    // Note: Computing the layout for the specialized program will have also computed
    // the layouts for the entry points, and we really need to attach that information
    // to them so that they don't go and try to compute their own specializations.
    //
    // TODO: Well, if we move to the specialization model described above then maybe
    // we *will* want entry points to do their own specialization work...
    //
    auto entryPointCount = m_entryPoints.getCount();
    for (Index i = 0; i < entryPointCount; ++i)
    {
        auto entryPointInfo = specializedLayout->getEntryPoint(i);
        auto entryPointVars = m_entryPoints[i];

        entryPointVars->m_specializedLayout = entryPointInfo.layout;
    }

    returnRefPtrMove(outLayout, specializedLayout);
    return SLANG_OK;
}

RefPtr<BufferResource> ShaderTableImpl::createDeviceBuffer(
    PipelineStateBase* pipeline,
    TransientResourceHeapBase* transientHeap,
    IResourceCommandEncoder* encoder)
{
    auto vkApi = m_device->m_api;
    auto rtProps = vkApi.m_rtProperties;
    uint32_t handleSize = rtProps.shaderGroupHandleSize;
    m_raygenTableSize = (uint32_t)VulkanUtil::calcAligned(
        m_rayGenShaderCount * handleSize, rtProps.shaderGroupBaseAlignment);
    m_missTableSize = (uint32_t)VulkanUtil::calcAligned(
        m_missShaderCount * handleSize, rtProps.shaderGroupBaseAlignment);
    m_hitTableSize = (uint32_t)VulkanUtil::calcAligned(
        m_hitGroupCount * handleSize, rtProps.shaderGroupBaseAlignment);
    m_callableTableSize = 0; // TODO: Are callable shaders needed?
    uint32_t tableSize = m_raygenTableSize + m_missTableSize + m_hitTableSize + m_callableTableSize;

    auto pipelineImpl = static_cast<RayTracingPipelineStateImpl*>(pipeline);
    ComPtr<IBufferResource> bufferResource;
    IBufferResource::Desc bufferDesc = {};
    bufferDesc.memoryType = MemoryType::DeviceLocal;
    bufferDesc.defaultState = ResourceState::General;
    bufferDesc.allowedStates =
        ResourceStateSet(ResourceState::General, ResourceState::CopyDestination);
    bufferDesc.type = IResource::Type::Buffer;
    bufferDesc.sizeInBytes = tableSize;
    m_device->createBufferResource(bufferDesc, nullptr, bufferResource.writeRef());

    TransientResourceHeapImpl* transientHeapImpl =
        static_cast<TransientResourceHeapImpl*>(transientHeap);

    IBufferResource* stagingBuffer = nullptr;
    size_t stagingBufferOffset = 0;
    transientHeapImpl->allocateStagingBuffer(
        tableSize, stagingBuffer, stagingBufferOffset, MemoryType::Upload);

    assert(stagingBuffer);
    void* stagingPtr = nullptr;
    stagingBuffer->map(nullptr, &stagingPtr);

    List<uint8_t> handles;
    auto handleCount = pipelineImpl->shaderGroupCount;
    auto totalHandleSize = handleSize * handleCount;
    handles.setCount(totalHandleSize);
    auto result = vkApi.vkGetRayTracingShaderGroupHandlesKHR(
        m_device->m_device,
        pipelineImpl->m_pipeline,
        0,
        (uint32_t)handleCount,
        totalHandleSize,
        handles.getBuffer());

    uint8_t* stagingBufferPtr = (uint8_t*)stagingPtr + stagingBufferOffset;
    auto subTablePtr = stagingBufferPtr;
    Int shaderTableEntryCounter = 0;

    // Each loop calculates the copy source and destination locations by fetching the name
    // of the shader group from the list of shader group names and getting its corresponding
    // index in the buffer of handles.
    for (uint32_t i = 0; i < m_rayGenShaderCount; i++)
    {
        auto dstHandlePtr = subTablePtr + i * handleSize;
        auto shaderGroupName = m_shaderGroupNames[shaderTableEntryCounter++];
        auto shaderGroupIndexPtr =
            pipelineImpl->shaderGroupNameToIndex.TryGetValue(shaderGroupName);
        if (!shaderGroupIndexPtr)
            continue;

        auto shaderGroupIndex = *shaderGroupIndexPtr;
        auto srcHandlePtr = handles.getBuffer() + shaderGroupIndex * handleSize;
        memcpy(dstHandlePtr, srcHandlePtr, handleSize);
    }
    subTablePtr += m_raygenTableSize;

    for (uint32_t i = 0; i < m_missShaderCount; i++)
    {
        auto dstHandlePtr = subTablePtr + i * handleSize;
        auto shaderGroupName = m_shaderGroupNames[shaderTableEntryCounter++];
        auto shaderGroupIndexPtr =
            pipelineImpl->shaderGroupNameToIndex.TryGetValue(shaderGroupName);
        if (!shaderGroupIndexPtr)
            continue;

        auto shaderGroupIndex = *shaderGroupIndexPtr;
        auto srcHandlePtr = handles.getBuffer() + shaderGroupIndex * handleSize;
        memcpy(dstHandlePtr, srcHandlePtr, handleSize);
    }
    subTablePtr += m_missTableSize;

    for (uint32_t i = 0; i < m_hitGroupCount; i++)
    {
        auto dstHandlePtr = subTablePtr + i * handleSize;
        auto shaderGroupName = m_shaderGroupNames[shaderTableEntryCounter++];
        auto shaderGroupIndexPtr =
            pipelineImpl->shaderGroupNameToIndex.TryGetValue(shaderGroupName);
        if (!shaderGroupIndexPtr)
            continue;

        auto shaderGroupIndex = *shaderGroupIndexPtr;
        auto srcHandlePtr = handles.getBuffer() + shaderGroupIndex * handleSize;
        memcpy(dstHandlePtr, srcHandlePtr, handleSize);
    }
    subTablePtr += m_hitTableSize;

    // TODO: Callable shaders?

    stagingBuffer->unmap(nullptr);
    encoder->copyBuffer(bufferResource, 0, stagingBuffer, stagingBufferOffset, tableSize);
    encoder->bufferBarrier(
        1,
        bufferResource.readRef(),
        gfx::ResourceState::CopyDestination,
        gfx::ResourceState::ShaderResource);
    RefPtr<BufferResource> resultPtr = static_cast<BufferResource*>(bufferResource.get());
    return _Move(resultPtr);
}

// There are a pair of cyclic references between a `TransientResourceHeap` and
// a `CommandBuffer` created from the heap. We need to break the cycle when
// the public reference count of a command buffer drops to 0.

ICommandBuffer* CommandBufferImpl::getInterface(const Guid& guid)
{
    if (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_ICommandBuffer)
        return static_cast<ICommandBuffer*>(this);
    return nullptr;
}

void CommandBufferImpl::comFree() { m_transientHeap.breakStrongReference(); }

Result CommandBufferImpl::init(
    DeviceImpl* renderer, VkCommandPool pool, TransientResourceHeapImpl* transientHeap)
{
    m_renderer = renderer;
    m_transientHeap = transientHeap;
    m_pool = pool;

    auto& api = renderer->m_api;
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = pool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;
    SLANG_VK_RETURN_ON_FAIL(
        api.vkAllocateCommandBuffers(api.m_device, &allocInfo, &m_commandBuffer));

    beginCommandBuffer();
    return SLANG_OK;
}

void CommandBufferImpl::beginCommandBuffer()
{
    auto& api = m_renderer->m_api;
    VkCommandBufferBeginInfo beginInfo = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        nullptr,
        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT};
    api.vkBeginCommandBuffer(m_commandBuffer, &beginInfo);
    if (m_preCommandBuffer)
    {
        api.vkBeginCommandBuffer(m_preCommandBuffer, &beginInfo);
    }
    m_isPreCommandBufferEmpty = true;
}

Result CommandBufferImpl::createPreCommandBuffer()
{
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_pool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;
    auto& api = m_renderer->m_api;
    SLANG_VK_RETURN_ON_FAIL(
        api.vkAllocateCommandBuffers(api.m_device, &allocInfo, &m_preCommandBuffer));
    VkCommandBufferBeginInfo beginInfo = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        nullptr,
        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT};
    api.vkBeginCommandBuffer(m_preCommandBuffer, &beginInfo);
    return SLANG_OK;
}

VkCommandBuffer CommandBufferImpl::getPreCommandBuffer()
{
    m_isPreCommandBufferEmpty = false;
    if (m_preCommandBuffer)
        return m_preCommandBuffer;
    createPreCommandBuffer();
    return m_preCommandBuffer;
}

void CommandBufferImpl::encodeRenderCommands(
    IRenderPassLayout* renderPass, IFramebuffer* framebuffer, IRenderCommandEncoder** outEncoder)
{
    if (!m_renderCommandEncoder)
    {
        m_renderCommandEncoder = new RenderCommandEncoder();
        m_renderCommandEncoder->init(this);
    }
    m_renderCommandEncoder->beginPass(renderPass, framebuffer);
    *outEncoder = m_renderCommandEncoder.Ptr();
}

void CommandBufferImpl::encodeComputeCommands(IComputeCommandEncoder** outEncoder)
{
    if (!m_computeCommandEncoder)
    {
        m_computeCommandEncoder = new ComputeCommandEncoder();
        m_computeCommandEncoder->init(this);
    }
    *outEncoder = m_computeCommandEncoder.Ptr();
}

void CommandBufferImpl::encodeResourceCommands(IResourceCommandEncoder** outEncoder)
{
    if (!m_resourceCommandEncoder)
    {
        m_resourceCommandEncoder = new ResourceCommandEncoder();
        m_resourceCommandEncoder->init(this);
    }
    *outEncoder = m_resourceCommandEncoder.Ptr();
}

void CommandBufferImpl::encodeRayTracingCommands(IRayTracingCommandEncoder** outEncoder)
{
    if (!m_rayTracingCommandEncoder)
    {
        if (m_renderer->m_api.vkCmdBuildAccelerationStructuresKHR)
        {
            m_rayTracingCommandEncoder = new RayTracingCommandEncoder();
            m_rayTracingCommandEncoder->init(this);
        }
    }
    *outEncoder = m_rayTracingCommandEncoder.Ptr();
}

void CommandBufferImpl::close()
{
    auto& vkAPI = m_renderer->m_api;
    if (!m_isPreCommandBufferEmpty)
    {
        // `preCmdBuffer` contains buffer transfer commands for shader object
        // uniform buffers, and we need a memory barrier here to ensure the
        // transfers are visible to shaders.
        VkMemoryBarrier memBarrier = {VK_STRUCTURE_TYPE_MEMORY_BARRIER};
        memBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        memBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        vkAPI.vkCmdPipelineBarrier(
            m_preCommandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            0,
            1,
            &memBarrier,
            0,
            nullptr,
            0,
            nullptr);
        vkAPI.vkEndCommandBuffer(m_preCommandBuffer);
    }
    vkAPI.vkEndCommandBuffer(m_commandBuffer);
}

Result CommandBufferImpl::getNativeHandle(InteropHandle* outHandle)
{
    outHandle->api = InteropHandleAPI::Vulkan;
    outHandle->handleValue = (uint64_t)m_commandBuffer;
    return SLANG_OK;
}

int PipelineCommandEncoder::getBindPointIndex(VkPipelineBindPoint bindPoint)
{
    switch (bindPoint)
    {
    case VK_PIPELINE_BIND_POINT_GRAPHICS:
        return 0;
    case VK_PIPELINE_BIND_POINT_COMPUTE:
        return 1;
    case VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR:
        return 2;
    default:
        assert(!"unknown pipeline type.");
        return -1;
    }
}

void PipelineCommandEncoder::init(CommandBufferImpl* commandBuffer)
{
    m_commandBuffer = commandBuffer;
    m_device = commandBuffer->m_renderer;
    m_vkCommandBuffer = m_commandBuffer->m_commandBuffer;
    m_api = &m_commandBuffer->m_renderer->m_api;
}

void PipelineCommandEncoder::endEncodingImpl()
{
    for (auto& pipeline : m_boundPipelines)
        pipeline = VK_NULL_HANDLE;
}

void PipelineCommandEncoder::_uploadBufferData(
    VkCommandBuffer commandBuffer,
    TransientResourceHeapImpl* transientHeap,
    BufferResourceImpl* buffer,
    size_t offset,
    size_t size,
    void* data)
{
    auto& api = buffer->m_renderer->m_api;
    IBufferResource* stagingBuffer = nullptr;
    size_t stagingBufferOffset = 0;
    transientHeap->allocateStagingBuffer(
        size, stagingBuffer, stagingBufferOffset, MemoryType::Upload);

    BufferResourceImpl* stagingBufferImpl = static_cast<BufferResourceImpl*>(stagingBuffer);

    void* mappedData = nullptr;
    SLANG_VK_CHECK(api.vkMapMemory(
        api.m_device,
        stagingBufferImpl->m_buffer.m_memory,
        0,
        stagingBufferOffset + size,
        0,
        &mappedData));
    memcpy((char*)mappedData + stagingBufferOffset, data, size);
    api.vkUnmapMemory(api.m_device, stagingBufferImpl->m_buffer.m_memory);

    // Copy from staging buffer to real buffer
    VkBufferCopy copyInfo = {};
    copyInfo.size = size;
    copyInfo.dstOffset = offset;
    copyInfo.srcOffset = stagingBufferOffset;
    api.vkCmdCopyBuffer(
        commandBuffer,
        stagingBufferImpl->m_buffer.m_buffer,
        buffer->m_buffer.m_buffer,
        1,
        &copyInfo);
}

void PipelineCommandEncoder::uploadBufferDataImpl(
    IBufferResource* buffer, size_t offset, size_t size, void* data)
{
    m_vkPreCommandBuffer = m_commandBuffer->getPreCommandBuffer();
    _uploadBufferData(
        m_vkPreCommandBuffer,
        m_commandBuffer->m_transientHeap.get(),
        static_cast<BufferResourceImpl*>(buffer),
        offset,
        size,
        data);
}

Result PipelineCommandEncoder::bindRootShaderObjectImpl(VkPipelineBindPoint bindPoint)
{
    // Obtain specialized root layout.
    auto rootObjectImpl = &m_commandBuffer->m_rootObject;

    auto specializedLayout = rootObjectImpl->getSpecializedLayout();
    if (!specializedLayout)
        return SLANG_FAIL;

    // We will set up the context required when binding shader objects
    // to the pipeline. Note that this is mostly just being packaged
    // together to minimize the number of parameters that have to
    // be dealt with in the complex recursive call chains.
    //
    RootBindingContext context;
    context.pipelineLayout = specializedLayout->m_pipelineLayout;
    context.device = m_device;
    context.descriptorSetAllocator = &m_commandBuffer->m_transientHeap->m_descSetAllocator;
    context.pushConstantRanges = specializedLayout->getAllPushConstantRanges().getArrayView();

    // The context includes storage for the descriptor sets we will bind,
    // and the number of sets we need to make space for is determined
    // by the specialized program layout.
    //
    List<VkDescriptorSet> descriptorSetsStorage;
    auto descriptorSetCount = specializedLayout->getTotalDescriptorSetCount();

    descriptorSetsStorage.setCount(descriptorSetCount);
    auto descriptorSets = descriptorSetsStorage.getBuffer();

    context.descriptorSets = descriptorSets;

    // We kick off recursive binding of shader objects to the pipeline (plus
    // the state in `context`).
    //
    // Note: this logic will directly write any push-constant ranges needed,
    // and will also fill in any descriptor sets. Currently it does not
    // *bind* the descriptor sets it fills in.
    //
    // TODO: It could probably bind the descriptor sets as well.
    //
    rootObjectImpl->bindAsRoot(this, context, specializedLayout);

    // Once we've filled in all the descriptor sets, we bind them
    // to the pipeline at once.
    //
    if (descriptorSetCount > 0)
    {
        m_device->m_api.vkCmdBindDescriptorSets(
            m_commandBuffer->m_commandBuffer,
            bindPoint,
            specializedLayout->m_pipelineLayout,
            0,
            (uint32_t)descriptorSetCount,
            descriptorSets,
            0,
            nullptr);
    }

    return SLANG_OK;
}

Result PipelineCommandEncoder::setPipelineStateImpl(
    IPipelineState* state, IShaderObject** outRootObject)
{
    m_currentPipeline = static_cast<PipelineStateImpl*>(state);
    SLANG_RETURN_ON_FAIL(m_commandBuffer->m_rootObject.init(
        m_commandBuffer->m_renderer,
        m_currentPipeline->getProgram<ShaderProgramImpl>()->m_rootObjectLayout));
    *outRootObject = &m_commandBuffer->m_rootObject;
    return SLANG_OK;
}

Result PipelineCommandEncoder::setPipelineStateWithRootObjectImpl(
    IPipelineState* state, IShaderObject* inObject)
{
    IShaderObject* rootObject = nullptr;
    SLANG_RETURN_ON_FAIL(setPipelineStateImpl(state, &rootObject));
    static_cast<ShaderObjectBase*>(rootObject)
        ->copyFrom(inObject, m_commandBuffer->m_transientHeap);
    return SLANG_OK;
}

void PipelineCommandEncoder::bindRenderState(VkPipelineBindPoint pipelineBindPoint)
{
    auto& api = *m_api;

    // Get specialized pipeline state and bind it.
    //
    RefPtr<PipelineStateBase> newPipeline;
    m_device->maybeSpecializePipeline(
        m_currentPipeline, &m_commandBuffer->m_rootObject, newPipeline);
    PipelineStateImpl* newPipelineImpl = static_cast<PipelineStateImpl*>(newPipeline.Ptr());

    newPipelineImpl->ensureAPIPipelineStateCreated();
    m_currentPipeline = newPipelineImpl;

    bindRootShaderObjectImpl(pipelineBindPoint);

    auto pipelineBindPointId = getBindPointIndex(pipelineBindPoint);
    if (m_boundPipelines[pipelineBindPointId] != newPipelineImpl->m_pipeline)
    {
        api.vkCmdBindPipeline(m_vkCommandBuffer, pipelineBindPoint, newPipelineImpl->m_pipeline);
        m_boundPipelines[pipelineBindPointId] = newPipelineImpl->m_pipeline;
    }
}

void ResourceCommandEncoder::copyBuffer(
    IBufferResource* dst, size_t dstOffset, IBufferResource* src, size_t srcOffset, size_t size)
{
    auto& vkAPI = m_commandBuffer->m_renderer->m_api;

    auto dstBuffer = static_cast<BufferResourceImpl*>(dst);
    auto srcBuffer = static_cast<BufferResourceImpl*>(src);

    VkBufferCopy copyRegion;
    copyRegion.dstOffset = dstOffset;
    copyRegion.srcOffset = srcOffset;
    copyRegion.size = size;

    // Note: Vulkan puts the source buffer first in the copy
    // command, going against the dominant tradition for copy
    // operations in C/C++.
    //
    vkAPI.vkCmdCopyBuffer(
        m_commandBuffer->m_commandBuffer,
        srcBuffer->m_buffer.m_buffer,
        dstBuffer->m_buffer.m_buffer,
        /* regionCount: */ 1,
        &copyRegion);
}

void ResourceCommandEncoder::uploadBufferData(
    IBufferResource* buffer, size_t offset, size_t size, void* data)
{
    PipelineCommandEncoder::_uploadBufferData(
        m_commandBuffer->m_commandBuffer,
        m_commandBuffer->m_transientHeap.get(),
        static_cast<BufferResourceImpl*>(buffer),
        offset,
        size,
        data);
}

void ResourceCommandEncoder::textureBarrier(
    size_t count, ITextureResource* const* textures, ResourceState src, ResourceState dst)
{
    ShortList<VkImageMemoryBarrier, 16> barriers;

    for (size_t i = 0; i < count; i++)
    {
        auto image = static_cast<TextureResourceImpl*>(textures[i]);
        auto desc = image->getDesc();

        VkImageMemoryBarrier barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.image = image->m_image;
        barrier.oldLayout = translateImageLayout(src);
        barrier.newLayout = translateImageLayout(dst);
        barrier.subresourceRange.aspectMask = getAspectMaskFromFormat(VulkanUtil::getVkFormat(desc->format));
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
        barrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
        barrier.srcAccessMask = calcAccessFlags(src);
        barrier.dstAccessMask = calcAccessFlags(dst);
        barriers.add(barrier);
    }

    VkPipelineStageFlagBits srcStage = calcPipelineStageFlags(src, true);
    VkPipelineStageFlagBits dstStage = calcPipelineStageFlags(dst, false);

    auto& vkApi = m_commandBuffer->m_renderer->m_api;
    vkApi.vkCmdPipelineBarrier(
        m_commandBuffer->m_commandBuffer,
        srcStage,
        dstStage,
        0,
        0,
        nullptr,
        0,
        nullptr,
        (uint32_t)count,
        barriers.getArrayView().getBuffer());
}

void ResourceCommandEncoder::bufferBarrier(
    size_t count, IBufferResource* const* buffers, ResourceState src, ResourceState dst)
{
    List<VkBufferMemoryBarrier> barriers;
    barriers.reserve(count);

    for (size_t i = 0; i < count; i++)
    {
        auto bufferImpl = static_cast<BufferResourceImpl*>(buffers[i]);

        VkBufferMemoryBarrier barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
        barrier.srcAccessMask = calcAccessFlags(src);
        barrier.dstAccessMask = calcAccessFlags(dst);
        barrier.buffer = bufferImpl->m_buffer.m_buffer;
        barrier.offset = 0;
        barrier.size = bufferImpl->getDesc()->sizeInBytes;

        barriers.add(barrier);
    }

    VkPipelineStageFlagBits srcStage = calcPipelineStageFlags(src, true);
    VkPipelineStageFlagBits dstStage = calcPipelineStageFlags(dst, false);

    auto& vkApi = m_commandBuffer->m_renderer->m_api;
    vkApi.vkCmdPipelineBarrier(
        m_commandBuffer->m_commandBuffer,
        srcStage,
        dstStage,
        0,
        0,
        nullptr,
        (uint32_t)count,
        barriers.getBuffer(),
        0,
        nullptr);
}

void ResourceCommandEncoder::endEncoding()
{
    // Insert memory barrier to ensure transfers are visible to the GPU.
    auto& vkAPI = m_commandBuffer->m_renderer->m_api;

    VkMemoryBarrier memBarrier = {VK_STRUCTURE_TYPE_MEMORY_BARRIER};
    memBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    memBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    vkAPI.vkCmdPipelineBarrier(
        m_commandBuffer->m_commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        0,
        1,
        &memBarrier,
        0,
        nullptr,
        0,
        nullptr);
}

void ResourceCommandEncoder::writeTimestamp(IQueryPool* queryPool, SlangInt index)
{
    _writeTimestamp(
        &m_commandBuffer->m_renderer->m_api, m_commandBuffer->m_commandBuffer, queryPool, index);
}

VkImageAspectFlags ResourceCommandEncoder::getAspectMask(TextureAspect aspect)
{
    VkImageAspectFlags flags = 0;

    if ((uint32_t)aspect & (uint32_t)TextureAspect::Depth)
        flags |= VK_IMAGE_ASPECT_DEPTH_BIT;
    if ((uint32_t)aspect & (uint32_t)TextureAspect::Stencil)
        flags |= VK_IMAGE_ASPECT_STENCIL_BIT;
    if ((uint32_t)aspect & (uint32_t)TextureAspect::Color)
        flags |= VK_IMAGE_ASPECT_COLOR_BIT;
    return flags;
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
    ITextureResource::Size extent)
{
    auto srcImage = static_cast<TextureResourceImpl*>(src);
    auto srcDesc = srcImage->getDesc();
    auto srcImageLayout = VulkanUtil::getImageLayoutFromState(srcState);
    auto dstImage = static_cast<TextureResourceImpl*>(dst);
    auto dstDesc = dstImage->getDesc();
    auto dstImageLayout = VulkanUtil::getImageLayoutFromState(dstState);
    if (dstSubresource.layerCount == 0 && dstSubresource.mipLevelCount == 0)
    {
        extent = dstDesc->size;
        dstSubresource.layerCount = dstDesc->arraySize;
        if (dstSubresource.layerCount == 0)
            dstSubresource.layerCount = 1;
        dstSubresource.mipLevelCount = dstDesc->numMipLevels;
    }
    if (srcSubresource.layerCount == 0 && srcSubresource.mipLevelCount == 0)
    {
        extent = srcDesc->size;
        srcSubresource.layerCount = srcDesc->arraySize;
        if (srcSubresource.layerCount == 0)
            srcSubresource.layerCount = 1;
        srcSubresource.mipLevelCount = dstDesc->numMipLevels;
    }
    VkImageCopy region = {};
    region.srcSubresource.aspectMask = getAspectMask(srcSubresource.aspectMask);
    region.srcSubresource.baseArrayLayer = srcSubresource.baseArrayLayer;
    region.srcSubresource.mipLevel = srcSubresource.mipLevel;
    region.srcSubresource.layerCount = srcSubresource.layerCount;
    region.srcOffset = {(int32_t)srcOffset.x, (int32_t)srcOffset.y, (int32_t)srcOffset.z};
    region.dstSubresource.aspectMask = getAspectMask(dstSubresource.aspectMask);
    region.dstSubresource.baseArrayLayer = dstSubresource.baseArrayLayer;
    region.dstSubresource.mipLevel = dstSubresource.mipLevel;
    region.dstSubresource.layerCount = dstSubresource.layerCount;
    region.dstOffset = {(int32_t)dstOffset.x, (int32_t)dstOffset.y, (int32_t)dstOffset.z};
    region.extent = {(uint32_t)extent.width, (uint32_t)extent.height, (uint32_t)extent.depth};

    auto& vkApi = m_commandBuffer->m_renderer->m_api;
    vkApi.vkCmdCopyImage(
        m_commandBuffer->m_commandBuffer,
        srcImage->m_image,
        srcImageLayout,
        dstImage->m_image,
        dstImageLayout,
        1,
        &region);
}

void ResourceCommandEncoder::uploadTextureData(
    ITextureResource* dst,
    SubresourceRange subResourceRange,
    ITextureResource::Offset3D offset,
    ITextureResource::Size extend,
    ITextureResource::SubresourceData* subResourceData,
    size_t subResourceDataCount)
{
    // VALIDATION: dst must be in TransferDst state.

    auto& vkApi = m_commandBuffer->m_renderer->m_api;
    auto dstImpl = static_cast<TextureResourceImpl*>(dst);
    List<TextureResource::Size> mipSizes;

    VkCommandBuffer commandBuffer = m_commandBuffer->m_commandBuffer;
    auto& desc = *dstImpl->getDesc();
    // Calculate how large the buffer has to be
    size_t bufferSize = 0;
    // Calculate how large an array entry is
    for (uint32_t j = subResourceRange.mipLevel;
         j < subResourceRange.mipLevel + subResourceRange.mipLevelCount;
         ++j)
    {
        const TextureResource::Size mipSize = calcMipSize(desc.size, j);

        auto rowSizeInBytes = calcRowSize(desc.format, mipSize.width);
        auto numRows = calcNumRows(desc.format, mipSize.height);

        mipSizes.add(mipSize);

        bufferSize += (rowSizeInBytes * numRows) * mipSize.depth;
    }

    // Calculate the total size taking into account the array
    bufferSize *= subResourceRange.layerCount;

    IBufferResource* uploadBuffer = nullptr;
    size_t uploadBufferOffset = 0;
    m_commandBuffer->m_transientHeap->allocateStagingBuffer(
        bufferSize, uploadBuffer, uploadBufferOffset, MemoryType::Upload);

    // Copy into upload buffer
    {
        int subResourceCounter = 0;

        uint8_t* dstData;
        uploadBuffer->map(nullptr, (void**)&dstData);
        dstData += uploadBufferOffset;
        uint8_t* dstDataStart;
        dstDataStart = dstData;

        size_t dstSubresourceOffset = 0;
        for (uint32_t i = 0; i < subResourceRange.layerCount; ++i)
        {
            for (Index j = 0; j < mipSizes.getCount(); ++j)
            {
                const auto& mipSize = mipSizes[j];

                int subResourceIndex = subResourceCounter++;
                auto initSubresource = subResourceData[subResourceIndex];

                const ptrdiff_t srcRowStride = (ptrdiff_t)initSubresource.strideY;
                const ptrdiff_t srcLayerStride = (ptrdiff_t)initSubresource.strideZ;

                auto dstRowSizeInBytes = calcRowSize(desc.format, mipSize.width);
                auto numRows = calcNumRows(desc.format, mipSize.height);
                auto dstLayerSizeInBytes = dstRowSizeInBytes * numRows;

                const uint8_t* srcLayer = (const uint8_t*)initSubresource.data;
                uint8_t* dstLayer = dstData + dstSubresourceOffset;

                for (int k = 0; k < mipSize.depth; k++)
                {
                    const uint8_t* srcRow = srcLayer;
                    uint8_t* dstRow = dstLayer;

                    for (uint32_t l = 0; l < numRows; l++)
                    {
                        ::memcpy(dstRow, srcRow, dstRowSizeInBytes);

                        dstRow += dstRowSizeInBytes;
                        srcRow += srcRowStride;
                    }

                    dstLayer += dstLayerSizeInBytes;
                    srcLayer += srcLayerStride;
                }

                dstSubresourceOffset += dstLayerSizeInBytes * mipSize.depth;
            }
        }
        uploadBuffer->unmap(nullptr);
    }
    {
        size_t srcOffset = uploadBufferOffset;
        for (uint32_t i = 0; i < subResourceRange.layerCount; ++i)
        {
            for (Index j = 0; j < mipSizes.getCount(); ++j)
            {
                const auto& mipSize = mipSizes[j];

                auto rowSizeInBytes = calcRowSize(desc.format, mipSize.width);
                auto numRows = calcNumRows(desc.format, mipSize.height);

                // https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/VkBufferImageCopy.html
                // bufferRowLength and bufferImageHeight specify the data in buffer
                // memory as a subregion of a larger two- or three-dimensional image,
                // and control the addressing calculations of data in buffer memory. If
                // either of these values is zero, that aspect of the buffer memory is
                // considered to be tightly packed according to the imageExtent.

                VkBufferImageCopy region = {};

                region.bufferOffset = srcOffset;
                region.bufferRowLength = 0; // rowSizeInBytes;
                region.bufferImageHeight = 0;

                region.imageSubresource.aspectMask = getAspectMaskFromFormat(dstImpl->m_vkformat);
                region.imageSubresource.mipLevel = subResourceRange.mipLevel + uint32_t(j);
                region.imageSubresource.baseArrayLayer = subResourceRange.baseArrayLayer + i;
                region.imageSubresource.layerCount = 1;
                region.imageOffset = {0, 0, 0};
                region.imageExtent = {
                    uint32_t(mipSize.width), uint32_t(mipSize.height), uint32_t(mipSize.depth)};

                // Do the copy (do all depths in a single go)
                vkApi.vkCmdCopyBufferToImage(
                    commandBuffer,
                    static_cast<BufferResourceImpl*>(uploadBuffer)->m_buffer.m_buffer,
                    dstImpl->m_image,
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    1,
                    &region);

                // Next
                srcOffset += rowSizeInBytes * numRows * mipSize.depth;
            }
        }
    }
}

void ResourceCommandEncoder::_clearColorImage(
    TextureResourceViewImpl* viewImpl, ClearValue* clearValue)
{
    auto& api = m_commandBuffer->m_renderer->m_api;
    auto layout = viewImpl->m_layout;
    if (layout != VK_IMAGE_LAYOUT_GENERAL && layout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        m_commandBuffer->m_renderer->_transitionImageLayout(
            m_commandBuffer->m_commandBuffer,
            viewImpl->m_texture->m_image,
            viewImpl->m_texture->m_vkformat,
            *viewImpl->m_texture->getDesc(),
            viewImpl->m_layout,
            layout);
    }

    VkImageSubresourceRange subresourceRange = {};
    subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.baseArrayLayer = viewImpl->m_desc.subresourceRange.baseArrayLayer;
    subresourceRange.baseMipLevel = viewImpl->m_desc.subresourceRange.mipLevel;
    subresourceRange.layerCount = viewImpl->m_desc.subresourceRange.layerCount;
    subresourceRange.levelCount = 1;

    VkClearColorValue vkClearColor = {};
    memcpy(vkClearColor.float32, clearValue->color.floatValues, sizeof(float) * 4);

    api.vkCmdClearColorImage(
        m_commandBuffer->m_commandBuffer,
        viewImpl->m_texture->m_image,
        layout,
        &vkClearColor,
        1,
        &subresourceRange);

    if (layout != viewImpl->m_layout)
    {
        m_commandBuffer->m_renderer->_transitionImageLayout(
            m_commandBuffer->m_commandBuffer,
            viewImpl->m_texture->m_image,
            viewImpl->m_texture->m_vkformat,
            *viewImpl->m_texture->getDesc(),
            layout,
            viewImpl->m_layout);
    }
}

void ResourceCommandEncoder::_clearDepthImage(
    TextureResourceViewImpl* viewImpl, ClearValue* clearValue, ClearResourceViewFlags::Enum flags)
{
    auto& api = m_commandBuffer->m_renderer->m_api;
    auto layout = viewImpl->m_layout;
    if (layout != VK_IMAGE_LAYOUT_GENERAL && layout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        m_commandBuffer->m_renderer->_transitionImageLayout(
            m_commandBuffer->m_commandBuffer,
            viewImpl->m_texture->m_image,
            viewImpl->m_texture->m_vkformat,
            *viewImpl->m_texture->getDesc(),
            viewImpl->m_layout,
            layout);
    }

    VkImageSubresourceRange subresourceRange = {};
    if (flags & ClearResourceViewFlags::ClearDepth)
    {
        if (VulkanUtil::isDepthFormat(viewImpl->m_texture->m_vkformat))
        {
            subresourceRange.aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;
        }
    }
    if (flags & ClearResourceViewFlags::ClearStencil)
    {
        if (VulkanUtil::isStencilFormat(viewImpl->m_texture->m_vkformat))
        {
            subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    }
    subresourceRange.baseArrayLayer = viewImpl->m_desc.subresourceRange.baseArrayLayer;
    subresourceRange.baseMipLevel = viewImpl->m_desc.subresourceRange.mipLevel;
    subresourceRange.layerCount = viewImpl->m_desc.subresourceRange.layerCount;
    subresourceRange.levelCount = 1;

    VkClearDepthStencilValue vkClearValue = {};
    vkClearValue.depth = clearValue->depthStencil.depth;
    vkClearValue.stencil = clearValue->depthStencil.stencil;

    api.vkCmdClearDepthStencilImage(
        m_commandBuffer->m_commandBuffer,
        viewImpl->m_texture->m_image,
        layout,
        &vkClearValue,
        1,
        &subresourceRange);

    if (layout != viewImpl->m_layout)
    {
        m_commandBuffer->m_renderer->_transitionImageLayout(
            m_commandBuffer->m_commandBuffer,
            viewImpl->m_texture->m_image,
            viewImpl->m_texture->m_vkformat,
            *viewImpl->m_texture->getDesc(),
            layout,
            viewImpl->m_layout);
    }
}

void ResourceCommandEncoder::_clearBuffer(
    VkBuffer buffer, uint64_t bufferSize, const IResourceView::Desc& desc, uint32_t clearValue)
{
    auto& api = m_commandBuffer->m_renderer->m_api;

    FormatInfo info = {};
    gfxGetFormatInfo(desc.format, &info);
    auto texelSize = info.blockSizeInBytes;
    auto elementCount = desc.bufferRange.elementCount;
    auto clearStart = (uint64_t)desc.bufferRange.firstElement * texelSize;
    auto clearSize = bufferSize - clearStart;
    if (elementCount != 0)
    {
        clearSize = (uint64_t)elementCount * texelSize;
    }
    api.vkCmdFillBuffer(
        m_commandBuffer->m_commandBuffer, buffer, clearStart, clearSize, clearValue);
}

void ResourceCommandEncoder::clearResourceView(
    IResourceView* view, ClearValue* clearValue, ClearResourceViewFlags::Enum flags)
{
    auto& api = m_commandBuffer->m_renderer->m_api;
    switch (view->getViewDesc()->type)
    {
    case IResourceView::Type::RenderTarget:
        {
            auto viewImpl = static_cast<TextureResourceViewImpl*>(view);
            _clearColorImage(viewImpl, clearValue);
        }
        break;
    case IResourceView::Type::DepthStencil:
        {
            auto viewImpl = static_cast<TextureResourceViewImpl*>(view);
            _clearDepthImage(viewImpl, clearValue, flags);
        }
        break;
    case IResourceView::Type::UnorderedAccess:
        {
            auto viewImplBase = static_cast<ResourceViewImpl*>(view);
            switch (viewImplBase->m_type)
            {
            case ResourceViewImpl::ViewType::Texture:
                {
                    auto viewImpl = static_cast<TextureResourceViewImpl*>(viewImplBase);
                    if ((flags & ClearResourceViewFlags::ClearDepth) ||
                        (flags & ClearResourceViewFlags::ClearStencil))
                    {
                        _clearDepthImage(viewImpl, clearValue, flags);
                    }
                    else
                    {
                        _clearColorImage(viewImpl, clearValue);
                    }
                }
                break;
            case ResourceViewImpl::ViewType::PlainBuffer:
                {
                    assert(
                        clearValue->color.uintValues[1] == clearValue->color.uintValues[0] &&
                        clearValue->color.uintValues[2] == clearValue->color.uintValues[0] &&
                        clearValue->color.uintValues[3] == clearValue->color.uintValues[0]);
                    auto viewImpl = static_cast<PlainBufferResourceViewImpl*>(viewImplBase);
                    uint64_t clearStart = viewImpl->m_desc.bufferRange.firstElement;
                    uint64_t clearSize = viewImpl->m_desc.bufferRange.elementCount;
                    if (clearSize == 0)
                        clearSize = viewImpl->m_buffer->getDesc()->sizeInBytes - clearStart;
                    api.vkCmdFillBuffer(
                        m_commandBuffer->m_commandBuffer,
                        viewImpl->m_buffer->m_buffer.m_buffer,
                        clearStart,
                        clearSize,
                        clearValue->color.uintValues[0]);
                }
                break;
            case ResourceViewImpl::ViewType::TexelBuffer:
                {
                    assert(
                        clearValue->color.uintValues[1] == clearValue->color.uintValues[0] &&
                        clearValue->color.uintValues[2] == clearValue->color.uintValues[0] &&
                        clearValue->color.uintValues[3] == clearValue->color.uintValues[0]);
                    auto viewImpl = static_cast<TexelBufferResourceViewImpl*>(viewImplBase);
                    _clearBuffer(
                        viewImpl->m_buffer->m_buffer.m_buffer,
                        viewImpl->m_buffer->getDesc()->sizeInBytes,
                        viewImpl->m_desc,
                        clearValue->color.uintValues[0]);
                }
                break;
            }
        }
        break;
    }
}

void ResourceCommandEncoder::resolveResource(
    ITextureResource* source,
    ResourceState sourceState,
    SubresourceRange sourceRange,
    ITextureResource* dest,
    ResourceState destState,
    SubresourceRange destRange)
{
    auto srcTexture = static_cast<TextureResourceImpl*>(source);
    auto srcExtent = srcTexture->getDesc()->size;
    auto dstTexture = static_cast<TextureResourceImpl*>(dest);

    auto srcImage = srcTexture->m_image;
    auto dstImage = dstTexture->m_image;

    auto srcImageLayout = VulkanUtil::getImageLayoutFromState(sourceState);
    auto dstImageLayout = VulkanUtil::getImageLayoutFromState(destState);

    for (uint32_t layer = 0; layer < sourceRange.layerCount; ++layer)
    {
        for (uint32_t mip = 0; mip < sourceRange.mipLevelCount; ++mip)
        {
            VkImageResolve region = {};
            region.srcSubresource.aspectMask = getAspectMask(sourceRange.aspectMask);
            region.srcSubresource.baseArrayLayer = layer + sourceRange.baseArrayLayer;
            region.srcSubresource.layerCount = 1;
            region.srcSubresource.mipLevel = mip + sourceRange.mipLevel;
            region.srcOffset = {0, 0, 0};
            region.dstSubresource.aspectMask = getAspectMask(destRange.aspectMask);
            region.dstSubresource.baseArrayLayer = layer + destRange.baseArrayLayer;
            region.dstSubresource.layerCount = 1;
            region.dstSubresource.mipLevel = mip + destRange.mipLevel;
            region.dstOffset = {0, 0, 0};
            region.extent = {
                (uint32_t)srcExtent.width, (uint32_t)srcExtent.height, (uint32_t)srcExtent.depth};

            auto& vkApi = m_commandBuffer->m_renderer->m_api;
            vkApi.vkCmdResolveImage(
                m_commandBuffer->m_commandBuffer,
                srcImage,
                srcImageLayout,
                dstImage,
                dstImageLayout,
                1,
                &region);
        }
    }
}

void ResourceCommandEncoder::resolveQuery(
    IQueryPool* queryPool, uint32_t index, uint32_t count, IBufferResource* buffer, uint64_t offset)
{
    auto& vkApi = m_commandBuffer->m_renderer->m_api;
    auto poolImpl = static_cast<QueryPoolImpl*>(queryPool);
    auto bufferImpl = static_cast<BufferResourceImpl*>(buffer);
    vkApi.vkCmdCopyQueryPoolResults(
        m_commandBuffer->m_commandBuffer,
        poolImpl->m_pool,
        index,
        count,
        bufferImpl->m_buffer.m_buffer,
        offset,
        sizeof(uint64_t),
        VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);
}

void ResourceCommandEncoder::copyTextureToBuffer(
    IBufferResource* dst,
    size_t dstOffset,
    size_t dstSize,
    size_t dstRowStride,
    ITextureResource* src,
    ResourceState srcState,
    SubresourceRange srcSubresource,
    ITextureResource::Offset3D srcOffset,
    ITextureResource::Size extent)
{
    assert(srcSubresource.mipLevelCount <= 1);

    auto image = static_cast<TextureResourceImpl*>(src);
    auto desc = image->getDesc();
    auto buffer = static_cast<BufferResourceImpl*>(dst);
    auto srcImageLayout = VulkanUtil::getImageLayoutFromState(srcState);

    VkBufferImageCopy region = {};
    region.bufferOffset = dstOffset;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = getAspectMask(srcSubresource.aspectMask);
    region.imageSubresource.mipLevel = srcSubresource.mipLevel;
    region.imageSubresource.baseArrayLayer = srcSubresource.baseArrayLayer;
    region.imageSubresource.layerCount = srcSubresource.layerCount;
    region.imageOffset = {(int32_t)srcOffset.x, (int32_t)srcOffset.y, (int32_t)srcOffset.z};
    region.imageExtent = {uint32_t(extent.width), uint32_t(extent.height), uint32_t(extent.depth)};

    auto& vkApi = m_commandBuffer->m_renderer->m_api;
    vkApi.vkCmdCopyImageToBuffer(
        m_commandBuffer->m_commandBuffer,
        image->m_image,
        srcImageLayout,
        buffer->m_buffer.m_buffer,
        1,
        &region);
}

void ResourceCommandEncoder::textureSubresourceBarrier(
    ITextureResource* texture,
    SubresourceRange subresourceRange,
    ResourceState src,
    ResourceState dst)
{
    ShortList<VkImageMemoryBarrier> barriers;
    auto image = static_cast<TextureResourceImpl*>(texture);
    auto desc = image->getDesc();

    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = image->m_image;
    barrier.oldLayout = translateImageLayout(src);
    barrier.newLayout = translateImageLayout(dst);
    barrier.subresourceRange.aspectMask = getAspectMask(subresourceRange.aspectMask);
    barrier.subresourceRange.baseArrayLayer = subresourceRange.baseArrayLayer;
    barrier.subresourceRange.baseMipLevel = subresourceRange.mipLevel;
    barrier.subresourceRange.layerCount = subresourceRange.layerCount;
    barrier.subresourceRange.levelCount = subresourceRange.mipLevelCount;
    barrier.srcAccessMask = calcAccessFlags(src);
    barrier.dstAccessMask = calcAccessFlags(dst);
    barriers.add(barrier);

    VkPipelineStageFlagBits srcStage = calcPipelineStageFlags(src, true);
    VkPipelineStageFlagBits dstStage = calcPipelineStageFlags(dst, false);

    auto& vkApi = m_commandBuffer->m_renderer->m_api;
    vkApi.vkCmdPipelineBarrier(
        m_commandBuffer->m_commandBuffer,
        srcStage,
        dstStage,
        0,
        0,
        nullptr,
        0,
        nullptr,
        (uint32_t)barriers.getCount(),
        barriers.getArrayView().getBuffer());
}

void ResourceCommandEncoder::beginDebugEvent(const char* name, float rgbColor[3])
{
    auto& vkApi = m_commandBuffer->m_renderer->m_api;
    if (vkApi.vkCmdDebugMarkerBeginEXT)
    {
        VkDebugMarkerMarkerInfoEXT eventInfo = {};
        eventInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
        eventInfo.pMarkerName = name;
        eventInfo.color[0] = rgbColor[0];
        eventInfo.color[1] = rgbColor[1];
        eventInfo.color[2] = rgbColor[2];
        eventInfo.color[3] = 1.0f;
        vkApi.vkCmdDebugMarkerBeginEXT(m_commandBuffer->m_commandBuffer, &eventInfo);
    }
}

void ResourceCommandEncoder::endDebugEvent()
{
    auto& vkApi = m_commandBuffer->m_renderer->m_api;
    if (vkApi.vkCmdDebugMarkerEndEXT)
    {
        vkApi.vkCmdDebugMarkerEndEXT(m_commandBuffer->m_commandBuffer);
    }
}

void RenderCommandEncoder::beginPass(IRenderPassLayout* renderPass, IFramebuffer* framebuffer)
{
    FramebufferImpl* framebufferImpl = static_cast<FramebufferImpl*>(framebuffer);
    if (!framebuffer)
        framebufferImpl = this->m_device->m_emptyFramebuffer;
    RenderPassLayoutImpl* renderPassImpl = static_cast<RenderPassLayoutImpl*>(renderPass);
    VkClearValue clearValues[kMaxAttachments] = {};
    VkRenderPassBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    beginInfo.framebuffer = framebufferImpl->m_handle;
    beginInfo.renderPass = renderPassImpl->m_renderPass;
    uint32_t attachmentCount = (uint32_t)framebufferImpl->renderTargetViews.getCount();
    if (framebufferImpl->depthStencilView)
        attachmentCount++;
    beginInfo.clearValueCount = attachmentCount;
    beginInfo.renderArea.extent.width = framebufferImpl->m_width;
    beginInfo.renderArea.extent.height = framebufferImpl->m_height;
    beginInfo.pClearValues = framebufferImpl->m_clearValues;
    auto& api = *m_api;
    api.vkCmdBeginRenderPass(m_vkCommandBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void RenderCommandEncoder::endEncoding()
{
    auto& api = *m_api;
    api.vkCmdEndRenderPass(m_vkCommandBuffer);
    endEncodingImpl();
}

Result RenderCommandEncoder::bindPipeline(
    IPipelineState* pipelineState, IShaderObject** outRootObject)
{
    return setPipelineStateImpl(pipelineState, outRootObject);
}

Result RenderCommandEncoder::bindPipelineWithRootObject(
    IPipelineState* pipelineState, IShaderObject* rootObject)
{
    return setPipelineStateWithRootObjectImpl(pipelineState, rootObject);
}

void RenderCommandEncoder::setViewports(uint32_t count, const Viewport* viewports)
{
    static const int kMaxViewports = 8; // TODO: base on device caps
    assert(count <= kMaxViewports);

    m_viewports.setCount(count);
    for (UInt ii = 0; ii < count; ++ii)
    {
        auto& inViewport = viewports[ii];
        auto& vkViewport = m_viewports[ii];

        vkViewport.x = inViewport.originX;
        vkViewport.y = inViewport.originY + inViewport.extentY;
        vkViewport.width = inViewport.extentX;
        vkViewport.height = -inViewport.extentY;
        vkViewport.minDepth = inViewport.minZ;
        vkViewport.maxDepth = inViewport.maxZ;
    }

    auto& api = *m_api;
    api.vkCmdSetViewport(m_vkCommandBuffer, 0, uint32_t(count), m_viewports.getBuffer());
}

void RenderCommandEncoder::setScissorRects(uint32_t count, const ScissorRect* rects)
{
    static const int kMaxScissorRects = 8; // TODO: base on device caps
    assert(count <= kMaxScissorRects);

    m_scissorRects.setCount(count);
    for (UInt ii = 0; ii < count; ++ii)
    {
        auto& inRect = rects[ii];
        auto& vkRect = m_scissorRects[ii];

        vkRect.offset.x = int32_t(inRect.minX);
        vkRect.offset.y = int32_t(inRect.minY);
        vkRect.extent.width = uint32_t(inRect.maxX - inRect.minX);
        vkRect.extent.height = uint32_t(inRect.maxY - inRect.minY);
    }

    auto& api = *m_api;
    api.vkCmdSetScissor(m_vkCommandBuffer, 0, uint32_t(count), m_scissorRects.getBuffer());
}

void RenderCommandEncoder::setPrimitiveTopology(PrimitiveTopology topology)
{
    auto& api = *m_api;
    if (api.vkCmdSetPrimitiveTopologyEXT)
    {
        api.vkCmdSetPrimitiveTopologyEXT(
            m_vkCommandBuffer, VulkanUtil::getVkPrimitiveTopology(topology));
    }
    else
    {
        switch (topology)
        {
        case PrimitiveTopology::TriangleList:
            break;
        default:
            // We are using a non-list topology, but we don't have dynmaic state
            // extension, error out.
            assert(!"Non-list topology requires VK_EXT_extended_dynamic_states, which "
                    "is not present.");
            break;
        }
    }
}

void RenderCommandEncoder::setVertexBuffers(
    uint32_t startSlot,
    uint32_t slotCount,
    IBufferResource* const* buffers,
    const uint32_t* offsets)
{
    for (Index i = 0; i < Index(slotCount); i++)
    {
        Index slotIndex = startSlot + i;
        BufferResourceImpl* buffer = static_cast<BufferResourceImpl*>(buffers[i]);
        if (buffer)
        {
            VkBuffer vertexBuffers[] = {buffer->m_buffer.m_buffer};
            VkDeviceSize offset = VkDeviceSize(offsets[i]);

            m_api->vkCmdBindVertexBuffers(
                m_vkCommandBuffer, (uint32_t)slotIndex, 1, vertexBuffers, &offset);
        }
    }
}

void RenderCommandEncoder::setIndexBuffer(
    IBufferResource* buffer, Format indexFormat, uint32_t offset)
{
    VkIndexType indexType = VK_INDEX_TYPE_UINT16;
    switch (indexFormat)
    {
    case Format::R16_UINT:
        indexType = VK_INDEX_TYPE_UINT16;
        break;
    case Format::R32_UINT:
        indexType = VK_INDEX_TYPE_UINT32;
        break;
    default:
        assert(!"unsupported index format");
    }

    BufferResourceImpl* bufferImpl = static_cast<BufferResourceImpl*>(buffer);

    m_api->vkCmdBindIndexBuffer(
        m_vkCommandBuffer, bufferImpl->m_buffer.m_buffer, (VkDeviceSize)offset, indexType);
}

void RenderCommandEncoder::prepareDraw()
{
    auto pipeline = static_cast<PipelineStateImpl*>(m_currentPipeline.Ptr());
    if (!pipeline)
    {
        assert(!"Invalid render pipeline");
        return;
    }
    bindRenderState(VK_PIPELINE_BIND_POINT_GRAPHICS);
}

void RenderCommandEncoder::draw(uint32_t vertexCount, uint32_t startVertex)
{
    prepareDraw();
    auto& api = *m_api;
    api.vkCmdDraw(m_vkCommandBuffer, vertexCount, 1, 0, 0);
}

void RenderCommandEncoder::drawIndexed(
    uint32_t indexCount, uint32_t startIndex, uint32_t baseVertex)
{
    prepareDraw();
    auto& api = *m_api;
    api.vkCmdDrawIndexed(m_vkCommandBuffer, indexCount, 1, startIndex, baseVertex, 0);
}

void RenderCommandEncoder::setStencilReference(uint32_t referenceValue)
{
    auto& api = *m_api;
    api.vkCmdSetStencilReference(m_vkCommandBuffer, VK_STENCIL_FRONT_AND_BACK, referenceValue);
}

void RenderCommandEncoder::drawIndirect(
    uint32_t maxDrawCount,
    IBufferResource* argBuffer,
    uint64_t argOffset,
    IBufferResource* countBuffer,
    uint64_t countOffset)
{
    // Vulkan does not support sourcing the count from a buffer.
    assert(!countBuffer);

    prepareDraw();
    auto& api = *m_api;
    auto argBufferImpl = static_cast<BufferResourceImpl*>(argBuffer);
    api.vkCmdDrawIndirect(
        m_vkCommandBuffer,
        argBufferImpl->m_buffer.m_buffer,
        argOffset,
        maxDrawCount,
        sizeof(VkDrawIndirectCommand));
}

void RenderCommandEncoder::drawIndexedIndirect(
    uint32_t maxDrawCount,
    IBufferResource* argBuffer,
    uint64_t argOffset,
    IBufferResource* countBuffer,
    uint64_t countOffset)
{
    // Vulkan does not support sourcing the count from a buffer.
    assert(!countBuffer);

    prepareDraw();
    auto& api = *m_api;
    auto argBufferImpl = static_cast<BufferResourceImpl*>(argBuffer);
    api.vkCmdDrawIndexedIndirect(
        m_vkCommandBuffer,
        argBufferImpl->m_buffer.m_buffer,
        argOffset,
        maxDrawCount,
        sizeof(VkDrawIndexedIndirectCommand));
}

Result RenderCommandEncoder::setSamplePositions(
    uint32_t samplesPerPixel, uint32_t pixelCount, const SamplePosition* samplePositions)
{
    if (m_api->vkCmdSetSampleLocationsEXT)
    {
        VkSampleLocationsInfoEXT sampleLocInfo = {};
        sampleLocInfo.sType = VK_STRUCTURE_TYPE_SAMPLE_LOCATIONS_INFO_EXT;
        sampleLocInfo.sampleLocationsCount = samplesPerPixel * pixelCount;
        sampleLocInfo.sampleLocationsPerPixel = (VkSampleCountFlagBits)samplesPerPixel;
        m_api->vkCmdSetSampleLocationsEXT(m_vkCommandBuffer, &sampleLocInfo);
        return SLANG_OK;
    }
    return SLANG_E_NOT_AVAILABLE;
}

void RenderCommandEncoder::drawInstanced(
    uint32_t vertexCount,
    uint32_t instanceCount,
    uint32_t startVertex,
    uint32_t startInstanceLocation)
{
    prepareDraw();
    auto& api = *m_api;
    api.vkCmdDraw(
        m_vkCommandBuffer, vertexCount, instanceCount, startVertex, startInstanceLocation);
}

void RenderCommandEncoder::drawIndexedInstanced(
    uint32_t indexCount,
    uint32_t instanceCount,
    uint32_t startIndexLocation,
    int32_t baseVertexLocation,
    uint32_t startInstanceLocation)
{
    prepareDraw();
    auto& api = *m_api;
    api.vkCmdDrawIndexed(
        m_vkCommandBuffer,
        indexCount,
        instanceCount,
        startIndexLocation,
        baseVertexLocation,
        startInstanceLocation);
}

void ComputeCommandEncoder::endEncoding() { endEncodingImpl(); }

Result ComputeCommandEncoder::bindPipeline(
    IPipelineState* pipelineState, IShaderObject** outRootObject)
{
    return setPipelineStateImpl(pipelineState, outRootObject);
}

Result ComputeCommandEncoder::bindPipelineWithRootObject(
    IPipelineState* pipelineState, IShaderObject* rootObject)
{
    return setPipelineStateWithRootObjectImpl(pipelineState, rootObject);
}

void ComputeCommandEncoder::dispatchCompute(int x, int y, int z)
{
    auto pipeline = static_cast<PipelineStateImpl*>(m_currentPipeline.Ptr());
    if (!pipeline)
    {
        assert(!"Invalid compute pipeline");
        return;
    }

    // Also create descriptor sets based on the given pipeline layout
    bindRenderState(VK_PIPELINE_BIND_POINT_COMPUTE);
    m_api->vkCmdDispatch(m_vkCommandBuffer, x, y, z);
}

void ComputeCommandEncoder::dispatchComputeIndirect(IBufferResource* argBuffer, uint64_t offset)
{
    SLANG_UNIMPLEMENTED_X("dispatchComputeIndirect");
}

void RayTracingCommandEncoder::_memoryBarrier(
    int count,
    IAccelerationStructure* const* structures,
    AccessFlag srcAccess,
    AccessFlag destAccess)
{
    ShortList<VkBufferMemoryBarrier> memBarriers;
    memBarriers.setCount(count);
    for (int i = 0; i < count; i++)
    {
        memBarriers[i].sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
        memBarriers[i].pNext = nullptr;
        memBarriers[i].dstAccessMask = translateAccelerationStructureAccessFlag(destAccess);
        memBarriers[i].srcAccessMask = translateAccelerationStructureAccessFlag(srcAccess);
        memBarriers[i].srcQueueFamilyIndex = m_commandBuffer->m_renderer->m_queueFamilyIndex;
        memBarriers[i].dstQueueFamilyIndex = m_commandBuffer->m_renderer->m_queueFamilyIndex;

        auto asImpl = static_cast<AccelerationStructureImpl*>(structures[i]);
        memBarriers[i].buffer = asImpl->m_buffer->m_buffer.m_buffer;
        memBarriers[i].offset = asImpl->m_offset;
        memBarriers[i].size = asImpl->m_size;
    }
    m_commandBuffer->m_renderer->m_api.vkCmdPipelineBarrier(
        m_commandBuffer->m_commandBuffer,
        VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR |
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR | VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT |
            VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT |
            VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR,
        0,
        0,
        nullptr,
        (uint32_t)memBarriers.getCount(),
        memBarriers.getArrayView().getBuffer(),
        0,
        nullptr);
}

void RayTracingCommandEncoder::_queryAccelerationStructureProperties(
    int accelerationStructureCount,
    IAccelerationStructure* const* accelerationStructures,
    int queryCount,
    AccelerationStructureQueryDesc* queryDescs)
{
    ShortList<VkAccelerationStructureKHR> vkHandles;
    vkHandles.setCount(accelerationStructureCount);
    for (int i = 0; i < accelerationStructureCount; i++)
    {
        vkHandles[i] =
            static_cast<AccelerationStructureImpl*>(accelerationStructures[i])->m_vkHandle;
    }
    auto vkHandlesView = vkHandles.getArrayView();
    for (int i = 0; i < queryCount; i++)
    {
        VkQueryType queryType;
        switch (queryDescs[i].queryType)
        {
        case QueryType::AccelerationStructureCompactedSize:
            queryType = VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR;
            break;
        case QueryType::AccelerationStructureSerializedSize:
            queryType = VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE_KHR;
            break;
        case QueryType::AccelerationStructureCurrentSize:
            continue;
        default:
            getDebugCallback()->handleMessage(
                DebugMessageType::Error,
                DebugMessageSource::Layer,
                "Invalid query type for use in queryAccelerationStructureProperties.");
            return;
        }
        auto queryPool = static_cast<QueryPoolImpl*>(queryDescs[i].queryPool)->m_pool;
        m_commandBuffer->m_renderer->m_api.vkCmdResetQueryPool(
            m_commandBuffer->m_commandBuffer,
            queryPool,
            (uint32_t)queryDescs[i].firstQueryIndex,
            1);
        m_commandBuffer->m_renderer->m_api.vkCmdWriteAccelerationStructuresPropertiesKHR(
            m_commandBuffer->m_commandBuffer,
            accelerationStructureCount,
            vkHandlesView.getBuffer(),
            queryType,
            queryPool,
            queryDescs[i].firstQueryIndex);
    }
}

void RayTracingCommandEncoder::buildAccelerationStructure(
    const IAccelerationStructure::BuildDesc& desc,
    int propertyQueryCount,
    AccelerationStructureQueryDesc* queryDescs)
{
    AccelerationStructureBuildGeometryInfoBuilder geomInfoBuilder;
    if (geomInfoBuilder.build(desc.inputs, getDebugCallback()) != SLANG_OK)
        return;

    if (desc.dest)
    {
        geomInfoBuilder.buildInfo.dstAccelerationStructure =
            static_cast<AccelerationStructureImpl*>(desc.dest)->m_vkHandle;
    }
    if (desc.source)
    {
        geomInfoBuilder.buildInfo.srcAccelerationStructure =
            static_cast<AccelerationStructureImpl*>(desc.source)->m_vkHandle;
    }
    geomInfoBuilder.buildInfo.scratchData.deviceAddress = desc.scratchData;

    List<VkAccelerationStructureBuildRangeInfoKHR> rangeInfos;
    rangeInfos.setCount(geomInfoBuilder.primitiveCounts.getCount());
    for (Index i = 0; i < geomInfoBuilder.primitiveCounts.getCount(); i++)
    {
        auto& rangeInfo = rangeInfos[i];
        rangeInfo.primitiveCount = geomInfoBuilder.primitiveCounts[i];
        rangeInfo.firstVertex = 0;
        rangeInfo.primitiveOffset = 0;
        rangeInfo.transformOffset = 0;
    }

    auto rangeInfoPtr = rangeInfos.getBuffer();
    m_commandBuffer->m_renderer->m_api.vkCmdBuildAccelerationStructuresKHR(
        m_commandBuffer->m_commandBuffer, 1, &geomInfoBuilder.buildInfo, &rangeInfoPtr);

    if (propertyQueryCount)
    {
        _memoryBarrier(1, &desc.dest, AccessFlag::Write, AccessFlag::Read);
        _queryAccelerationStructureProperties(1, &desc.dest, propertyQueryCount, queryDescs);
    }
}

void RayTracingCommandEncoder::copyAccelerationStructure(
    IAccelerationStructure* dest, IAccelerationStructure* src, AccelerationStructureCopyMode mode)
{
    VkCopyAccelerationStructureInfoKHR copyInfo = {
        VK_STRUCTURE_TYPE_COPY_ACCELERATION_STRUCTURE_INFO_KHR};
    copyInfo.src = static_cast<AccelerationStructureImpl*>(src)->m_vkHandle;
    copyInfo.dst = static_cast<AccelerationStructureImpl*>(dest)->m_vkHandle;
    switch (mode)
    {
    case AccelerationStructureCopyMode::Clone:
        copyInfo.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_KHR;
        break;
    case AccelerationStructureCopyMode::Compact:
        copyInfo.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_COMPACT_KHR;
        break;
    default:
        getDebugCallback()->handleMessage(
            DebugMessageType::Error,
            DebugMessageSource::Layer,
            "Unsupported AccelerationStructureCopyMode.");
        return;
    }
    m_commandBuffer->m_renderer->m_api.vkCmdCopyAccelerationStructureKHR(
        m_commandBuffer->m_commandBuffer, &copyInfo);
}

void RayTracingCommandEncoder::queryAccelerationStructureProperties(
    int accelerationStructureCount,
    IAccelerationStructure* const* accelerationStructures,
    int queryCount,
    AccelerationStructureQueryDesc* queryDescs)
{
    _queryAccelerationStructureProperties(
        accelerationStructureCount, accelerationStructures, queryCount, queryDescs);
}

void RayTracingCommandEncoder::serializeAccelerationStructure(
    DeviceAddress dest, IAccelerationStructure* source)
{
    VkCopyAccelerationStructureToMemoryInfoKHR copyInfo = {
        VK_STRUCTURE_TYPE_COPY_ACCELERATION_STRUCTURE_TO_MEMORY_INFO_KHR};
    copyInfo.src = static_cast<AccelerationStructureImpl*>(source)->m_vkHandle;
    copyInfo.dst.deviceAddress = dest;
    copyInfo.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_SERIALIZE_KHR;
    m_commandBuffer->m_renderer->m_api.vkCmdCopyAccelerationStructureToMemoryKHR(
        m_commandBuffer->m_commandBuffer, &copyInfo);
}

void RayTracingCommandEncoder::deserializeAccelerationStructure(
    IAccelerationStructure* dest, DeviceAddress source)
{
    VkCopyMemoryToAccelerationStructureInfoKHR copyInfo = {
        VK_STRUCTURE_TYPE_COPY_MEMORY_TO_ACCELERATION_STRUCTURE_INFO_KHR};
    copyInfo.src.deviceAddress = source;
    copyInfo.dst = static_cast<AccelerationStructureImpl*>(dest)->m_vkHandle;
    copyInfo.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_DESERIALIZE_KHR;
    m_commandBuffer->m_renderer->m_api.vkCmdCopyMemoryToAccelerationStructureKHR(
        m_commandBuffer->m_commandBuffer, &copyInfo);
}

void RayTracingCommandEncoder::bindPipeline(IPipelineState* pipeline, IShaderObject** outRootObject)
{
    setPipelineStateImpl(pipeline, outRootObject);
}

Result RayTracingCommandEncoder::bindPipelineWithRootObject(
    IPipelineState* pipelineState, IShaderObject* rootObject)
{
    return setPipelineStateWithRootObjectImpl(pipelineState, rootObject);
}

void RayTracingCommandEncoder::dispatchRays(
    uint32_t raygenShaderIndex,
    IShaderTable* shaderTable,
    int32_t width,
    int32_t height,
    int32_t depth)
{
    auto vkApi = m_commandBuffer->m_renderer->m_api;
    auto vkCommandBuffer = m_commandBuffer->m_commandBuffer;

    bindRenderState(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR);

    auto rtProps = vkApi.m_rtProperties;
    auto shaderTableImpl = (ShaderTableImpl*)shaderTable;
    auto alignedHandleSize =
        VulkanUtil::calcAligned(rtProps.shaderGroupHandleSize, rtProps.shaderGroupHandleAlignment);

    auto shaderTableBuffer = shaderTableImpl->getOrCreateBuffer(
        m_currentPipeline,
        m_commandBuffer->m_transientHeap,
        static_cast<ResourceCommandEncoder*>(this));

    VkStridedDeviceAddressRegionKHR raygenSBT;
    raygenSBT.deviceAddress = shaderTableBuffer->getDeviceAddress();
    raygenSBT.stride = VulkanUtil::calcAligned(alignedHandleSize, rtProps.shaderGroupBaseAlignment);
    raygenSBT.size = raygenSBT.stride;

    VkStridedDeviceAddressRegionKHR missSBT;
    missSBT.deviceAddress = raygenSBT.deviceAddress + raygenSBT.size;
    missSBT.stride = alignedHandleSize;
    missSBT.size = shaderTableImpl->m_missTableSize;

    VkStridedDeviceAddressRegionKHR hitSBT;
    hitSBT.deviceAddress = missSBT.deviceAddress + missSBT.size;
    hitSBT.stride = alignedHandleSize;
    hitSBT.size = shaderTableImpl->m_hitTableSize;

    // TODO: Are callable shaders needed?
    VkStridedDeviceAddressRegionKHR callableSBT;
    callableSBT.deviceAddress = 0;
    callableSBT.stride = 0;
    callableSBT.size = 0;

    vkApi.vkCmdTraceRaysKHR(
        vkCommandBuffer,
        &raygenSBT,
        &missSBT,
        &hitSBT,
        &callableSBT,
        (uint32_t)width,
        (uint32_t)height,
        (uint32_t)depth);
}

void RayTracingCommandEncoder::endEncoding() { endEncodingImpl(); }

ICommandQueue* CommandQueueImpl::getInterface(const Guid& guid)
{
    if (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_ICommandQueue)
        return static_cast<ICommandQueue*>(this);
    return nullptr;
}

CommandQueueImpl::~CommandQueueImpl()
{
    m_renderer->m_api.vkQueueWaitIdle(m_queue);

    m_renderer->m_queueAllocCount--;
    m_renderer->m_api.vkDestroySemaphore(m_renderer->m_api.m_device, m_semaphore, nullptr);
}

void CommandQueueImpl::init(DeviceImpl* renderer, VkQueue queue, uint32_t queueFamilyIndex)
{
    m_renderer = renderer;
    m_queue = queue;
    m_queueFamilyIndex = queueFamilyIndex;
    VkSemaphoreCreateInfo semaphoreCreateInfo = {};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreCreateInfo.flags = 0;
    m_renderer->m_api.vkCreateSemaphore(
        m_renderer->m_api.m_device, &semaphoreCreateInfo, nullptr, &m_semaphore);
}

void CommandQueueImpl::waitOnHost()
{
    auto& vkAPI = m_renderer->m_api;
    vkAPI.vkQueueWaitIdle(m_queue);
}

Result CommandQueueImpl::getNativeHandle(InteropHandle* outHandle)
{
    outHandle->api = InteropHandleAPI::D3D12;
    outHandle->handleValue = (uint64_t)m_queue;
    return SLANG_OK;
}

const CommandQueueImpl::Desc& CommandQueueImpl::getDesc() { return m_desc; }

Result CommandQueueImpl::waitForFenceValuesOnDevice(
    uint32_t fenceCount, IFence** fences, uint64_t* waitValues)
{
    for (uint32_t i = 0; i < fenceCount; ++i)
    {
        FenceWaitInfo waitInfo;
        waitInfo.fence = static_cast<FenceImpl*>(fences[i]);
        waitInfo.waitValue = waitValues[i];
        m_pendingWaitFences.add(waitInfo);
    }
    return SLANG_OK;
}

void CommandQueueImpl::queueSubmitImpl(
    uint32_t count, ICommandBuffer* const* commandBuffers, IFence* fence, uint64_t valueToSignal)
{
    auto& vkAPI = m_renderer->m_api;
    m_submitCommandBuffers.clear();
    for (uint32_t i = 0; i < count; i++)
    {
        auto cmdBufImpl = static_cast<CommandBufferImpl*>(commandBuffers[i]);
        if (!cmdBufImpl->m_isPreCommandBufferEmpty)
            m_submitCommandBuffers.add(cmdBufImpl->m_preCommandBuffer);
        auto vkCmdBuf = cmdBufImpl->m_commandBuffer;
        m_submitCommandBuffers.add(vkCmdBuf);
    }
    Array<VkSemaphore, 2> signalSemaphores;
    Array<uint64_t, 2> signalValues;
    signalSemaphores.add(m_semaphore);
    signalValues.add(0);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    VkPipelineStageFlags stageFlag[] = {
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT};
    submitInfo.pWaitDstStageMask = stageFlag;
    submitInfo.commandBufferCount = (uint32_t)m_submitCommandBuffers.getCount();
    submitInfo.pCommandBuffers = m_submitCommandBuffers.getBuffer();
    Array<VkSemaphore, 3> waitSemaphores;
    Array<uint64_t, 3> waitValues;
    for (auto s : m_pendingWaitSemaphores)
    {
        if (s != VK_NULL_HANDLE)
        {
            waitSemaphores.add(s);
            waitValues.add(0);
        }
    }
    for (auto& fenceWait : m_pendingWaitFences)
    {
        waitSemaphores.add(fenceWait.fence->m_semaphore);
        waitValues.add(fenceWait.waitValue);
    }
    m_pendingWaitFences.clear();
    VkTimelineSemaphoreSubmitInfo timelineSubmitInfo = {
        VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO};
    if (fence)
    {
        auto fenceImpl = static_cast<FenceImpl*>(fence);
        signalSemaphores.add(fenceImpl->m_semaphore);
        signalValues.add(valueToSignal);
        submitInfo.pNext = &timelineSubmitInfo;
        timelineSubmitInfo.signalSemaphoreValueCount = (uint32_t)signalValues.getCount();
        timelineSubmitInfo.pSignalSemaphoreValues = signalValues.getBuffer();
        timelineSubmitInfo.waitSemaphoreValueCount = (uint32_t)waitValues.getCount();
        timelineSubmitInfo.pWaitSemaphoreValues = waitValues.getBuffer();
    }
    submitInfo.waitSemaphoreCount = (uint32_t)waitSemaphores.getCount();
    if (submitInfo.waitSemaphoreCount)
    {
        submitInfo.pWaitSemaphores = waitSemaphores.getBuffer();
    }
    submitInfo.signalSemaphoreCount = (uint32_t)signalSemaphores.getCount();
    submitInfo.pSignalSemaphores = signalSemaphores.getBuffer();

    VkFence vkFence = VK_NULL_HANDLE;
    if (count)
    {
        auto commandBufferImpl = static_cast<CommandBufferImpl*>(commandBuffers[0]);
        vkFence = commandBufferImpl->m_transientHeap->getCurrentFence();
        vkAPI.vkResetFences(vkAPI.m_device, 1, &vkFence);
        commandBufferImpl->m_transientHeap->advanceFence();
    }
    vkAPI.vkQueueSubmit(m_queue, 1, &submitInfo, vkFence);
    m_pendingWaitSemaphores[0] = m_semaphore;
    m_pendingWaitSemaphores[1] = VK_NULL_HANDLE;
}

void CommandQueueImpl::executeCommandBuffers(
    uint32_t count, ICommandBuffer* const* commandBuffers, IFence* fence, uint64_t valueToSignal)
{
    if (count == 0 && fence == nullptr)
        return;
    queueSubmitImpl(count, commandBuffers, fence, valueToSignal);
}

Result QueryPoolImpl::init(const IQueryPool::Desc& desc, DeviceImpl* device)
{
    m_device = device;
    m_pool = VK_NULL_HANDLE;
    VkQueryPoolCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    createInfo.queryCount = (uint32_t)desc.count;
    switch (desc.type)
    {
    case QueryType::Timestamp:
        createInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;
        break;
    case QueryType::AccelerationStructureCompactedSize:
        createInfo.queryType = VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR;
        break;
    case QueryType::AccelerationStructureSerializedSize:
        createInfo.queryType = VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE_KHR;
        break;
    case QueryType::AccelerationStructureCurrentSize:
        // Vulkan does not support CurrentSize query, will not create actual pools here.
        return SLANG_OK;
    default:
        return SLANG_E_INVALID_ARG;
    }
    SLANG_VK_RETURN_ON_FAIL(
        m_device->m_api.vkCreateQueryPool(m_device->m_api.m_device, &createInfo, nullptr, &m_pool));
    return SLANG_OK;
}

QueryPoolImpl::~QueryPoolImpl()
{
    m_device->m_api.vkDestroyQueryPool(m_device->m_api.m_device, m_pool, nullptr);
}

Result QueryPoolImpl::getResult(SlangInt index, SlangInt count, uint64_t* data)
{
    if (!m_pool)
    {
        // Vulkan does not support CurrentSize query, return 0 here.
        for (SlangInt i = 0; i < count; i++)
            data[i] = 0;
        return SLANG_OK;
    }

    SLANG_VK_RETURN_ON_FAIL(m_device->m_api.vkGetQueryPoolResults(
        m_device->m_api.m_device,
        m_pool,
        (uint32_t)index,
        (uint32_t)count,
        sizeof(uint64_t) * count,
        data,
        sizeof(uint64_t),
        VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT));
    return SLANG_OK;
}

ISwapchain* SwapchainImpl::getInterface(const Guid& guid)
{
    if (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_ISwapchain)
        return static_cast<ISwapchain*>(this);
    return nullptr;
}

void SwapchainImpl::destroySwapchainAndImages()
{
    m_api->vkQueueWaitIdle(m_queue->m_queue);
    if (m_swapChain != VK_NULL_HANDLE)
    {
        m_api->vkDestroySwapchainKHR(m_api->m_device, m_swapChain, nullptr);
        m_swapChain = VK_NULL_HANDLE;
    }

    // Mark that it is no longer used
    m_images.clear();
}

void SwapchainImpl::getWindowSize(int* widthOut, int* heightOut) const
{
#if SLANG_WINDOWS_FAMILY
    RECT rc;
    ::GetClientRect((HWND)m_windowHandle.handleValues[0], &rc);
    *widthOut = rc.right - rc.left;
    *heightOut = rc.bottom - rc.top;
#elif defined(SLANG_ENABLE_XLIB)
    XWindowAttributes winAttr = {};
    XGetWindowAttributes(
        (Display*)m_windowHandle.handleValues[0], (Window)m_windowHandle.handleValues[1], &winAttr);

    *widthOut = winAttr.width;
    *heightOut = winAttr.height;
#else
    *widthOut = 0;
    *heightOut = 0;
#endif
}

Result SwapchainImpl::createSwapchainAndImages()
{
    int width, height;
    getWindowSize(&width, &height);

    VkExtent2D imageExtent = {};
    imageExtent.width = width;
    imageExtent.height = height;

    m_desc.width = width;
    m_desc.height = height;

    // catch this before throwing error
    if (width == 0 || height == 0)
    {
        return SLANG_FAIL;
    }

    // It is necessary to query the caps -> otherwise the LunarG verification layer will
    // issue an error
    {
        VkSurfaceCapabilitiesKHR surfaceCaps;

        SLANG_VK_RETURN_ON_FAIL(m_api->vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
            m_api->m_physicalDevice, m_surface, &surfaceCaps));
    }

    VkPresentModeKHR presentMode;
    List<VkPresentModeKHR> presentModes;
    uint32_t numPresentModes = 0;
    m_api->vkGetPhysicalDeviceSurfacePresentModesKHR(
        m_api->m_physicalDevice, m_surface, &numPresentModes, nullptr);
    presentModes.setCount(numPresentModes);
    m_api->vkGetPhysicalDeviceSurfacePresentModesKHR(
        m_api->m_physicalDevice, m_surface, &numPresentModes, presentModes.getBuffer());

    {
        int numCheckPresentOptions = 3;
        VkPresentModeKHR presentOptions[] = {
            VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_FIFO_KHR};
        if (m_desc.enableVSync)
        {
            presentOptions[0] = VK_PRESENT_MODE_FIFO_KHR;
            presentOptions[1] = VK_PRESENT_MODE_IMMEDIATE_KHR;
            presentOptions[2] = VK_PRESENT_MODE_MAILBOX_KHR;
        }

        presentMode = VK_PRESENT_MODE_MAX_ENUM_KHR; // Invalid

        // Find the first option that's available on the device
        for (int j = 0; j < numCheckPresentOptions; j++)
        {
            if (presentModes.indexOf(presentOptions[j]) != Index(-1))
            {
                presentMode = presentOptions[j];
                break;
            }
        }

        if (presentMode == VK_PRESENT_MODE_MAX_ENUM_KHR)
        {
            return SLANG_FAIL;
        }
    }

    VkSwapchainKHR oldSwapchain = VK_NULL_HANDLE;

    VkSwapchainCreateInfoKHR swapchainDesc = {};
    swapchainDesc.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainDesc.surface = m_surface;
    swapchainDesc.minImageCount = m_desc.imageCount;
    swapchainDesc.imageFormat = m_vkformat;
    swapchainDesc.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    swapchainDesc.imageExtent = imageExtent;
    swapchainDesc.imageArrayLayers = 1;
    swapchainDesc.imageUsage =
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    swapchainDesc.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchainDesc.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchainDesc.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainDesc.presentMode = presentMode;
    swapchainDesc.clipped = VK_TRUE;
    swapchainDesc.oldSwapchain = oldSwapchain;

    SLANG_VK_RETURN_ON_FAIL(
        m_api->vkCreateSwapchainKHR(m_api->m_device, &swapchainDesc, nullptr, &m_swapChain));

    uint32_t numSwapChainImages = 0;
    m_api->vkGetSwapchainImagesKHR(m_api->m_device, m_swapChain, &numSwapChainImages, nullptr);
    List<VkImage> vkImages;
    {
        vkImages.setCount(numSwapChainImages);
        m_api->vkGetSwapchainImagesKHR(
            m_api->m_device, m_swapChain, &numSwapChainImages, vkImages.getBuffer());
    }

    for (uint32_t i = 0; i < m_desc.imageCount; i++)
    {
        ITextureResource::Desc imageDesc = {};
        imageDesc.allowedStates = ResourceStateSet(
            ResourceState::Present, ResourceState::RenderTarget, ResourceState::CopyDestination);
        imageDesc.type = IResource::Type::Texture2D;
        imageDesc.arraySize = 0;
        imageDesc.format = m_desc.format;
        imageDesc.size.width = m_desc.width;
        imageDesc.size.height = m_desc.height;
        imageDesc.size.depth = 1;
        imageDesc.numMipLevels = 1;
        imageDesc.defaultState = ResourceState::Present;
        RefPtr<TextureResourceImpl> image = new TextureResourceImpl(imageDesc, m_renderer);
        image->m_image = vkImages[i];
        image->m_imageMemory = 0;
        image->m_vkformat = m_vkformat;
        image->m_isWeakImageReference = true;
        m_images.add(image);
    }
    return SLANG_OK;
}

SwapchainImpl::~SwapchainImpl()
{
    destroySwapchainAndImages();
    if (m_surface)
    {
        m_api->vkDestroySurfaceKHR(m_api->m_instance, m_surface, nullptr);
        m_surface = VK_NULL_HANDLE;
    }
    m_renderer->m_api.vkDestroySemaphore(m_renderer->m_api.m_device, m_nextImageSemaphore, nullptr);
}

Index SwapchainImpl::_indexOfFormat(List<VkSurfaceFormatKHR>& formatsIn, VkFormat format)
{
    const Index numFormats = formatsIn.getCount();
    const VkSurfaceFormatKHR* formats = formatsIn.getBuffer();

    for (Index i = 0; i < numFormats; ++i)
    {
        if (formats[i].format == format)
        {
            return i;
        }
    }
    return -1;
}

Result SwapchainImpl::init(DeviceImpl* renderer, const ISwapchain::Desc& desc, WindowHandle window)
{
    m_desc = desc;
    m_renderer = renderer;
    m_api = &renderer->m_api;
    m_queue = static_cast<CommandQueueImpl*>(desc.queue);
    m_windowHandle = window;

    VkSemaphoreCreateInfo semaphoreCreateInfo = {};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    SLANG_VK_RETURN_ON_FAIL(renderer->m_api.vkCreateSemaphore(
        renderer->m_api.m_device, &semaphoreCreateInfo, nullptr, &m_nextImageSemaphore));

    m_queue = static_cast<CommandQueueImpl*>(desc.queue);

    // Make sure it's not set initially
    m_vkformat = VK_FORMAT_UNDEFINED;

#if SLANG_WINDOWS_FAMILY
    VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.hinstance = ::GetModuleHandle(nullptr);
    surfaceCreateInfo.hwnd = (HWND)window.handleValues[0];
    SLANG_VK_RETURN_ON_FAIL(
        m_api->vkCreateWin32SurfaceKHR(m_api->m_instance, &surfaceCreateInfo, nullptr, &m_surface));
#else
    VkXlibSurfaceCreateInfoKHR surfaceCreateInfo = {};
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.dpy = (Display*)window.handleValues[0];
    surfaceCreateInfo.window = (Window)window.handleValues[1];
    SLANG_VK_RETURN_ON_FAIL(
        m_api->vkCreateXlibSurfaceKHR(m_api->m_instance, &surfaceCreateInfo, nullptr, &m_surface));
#endif

    VkBool32 supported = false;
    m_api->vkGetPhysicalDeviceSurfaceSupportKHR(
        m_api->m_physicalDevice, renderer->m_queueFamilyIndex, m_surface, &supported);

    uint32_t numSurfaceFormats = 0;
    List<VkSurfaceFormatKHR> surfaceFormats;
    m_api->vkGetPhysicalDeviceSurfaceFormatsKHR(
        m_api->m_physicalDevice, m_surface, &numSurfaceFormats, nullptr);
    surfaceFormats.setCount(int(numSurfaceFormats));
    m_api->vkGetPhysicalDeviceSurfaceFormatsKHR(
        m_api->m_physicalDevice, m_surface, &numSurfaceFormats, surfaceFormats.getBuffer());

    // Look for a suitable format
    List<VkFormat> formats;
    formats.add(VulkanUtil::getVkFormat(desc.format));
    // HACK! To check for a different format if couldn't be found
    if (desc.format == Format::R8G8B8A8_UNORM)
    {
        formats.add(VK_FORMAT_B8G8R8A8_UNORM);
    }

    for (Index i = 0; i < formats.getCount(); ++i)
    {
        VkFormat format = formats[i];
        if (_indexOfFormat(surfaceFormats, format) >= 0)
        {
            m_vkformat = format;
        }
    }

    if (m_vkformat == VK_FORMAT_UNDEFINED)
    {
        return SLANG_FAIL;
    }

    // Save the desc
    m_desc = desc;
    if (m_desc.format == Format::R8G8B8A8_UNORM && m_vkformat == VK_FORMAT_B8G8R8A8_UNORM)
    {
        m_desc.format = Format::B8G8R8A8_UNORM;
    }

    createSwapchainAndImages();
    return SLANG_OK;
}

Result SwapchainImpl::getImage(uint32_t index, ITextureResource** outResource)
{
    if (m_images.getCount() <= (Index)index)
        return SLANG_FAIL;
    returnComPtr(outResource, m_images[index]);
    return SLANG_OK;
}

Result SwapchainImpl::resize(uint32_t width, uint32_t height)
{
    SLANG_UNUSED(width);
    SLANG_UNUSED(height);
    destroySwapchainAndImages();
    return createSwapchainAndImages();
}

Result SwapchainImpl::present()
{
    // If there are pending fence wait operations, flush them as an
    // empty vkQueueSubmit.
    if (m_queue->m_pendingWaitFences.getCount() != 0)
    {
        m_queue->queueSubmitImpl(0, nullptr, nullptr, 0);
    }

    uint32_t swapChainIndices[] = {uint32_t(m_currentImageIndex)};

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &m_swapChain;
    presentInfo.pImageIndices = swapChainIndices;
    Array<VkSemaphore, 2> waitSemaphores;
    for (auto s : m_queue->m_pendingWaitSemaphores)
    {
        if (s != VK_NULL_HANDLE)
        {
            waitSemaphores.add(s);
        }
    }
    m_queue->m_pendingWaitSemaphores[0] = VK_NULL_HANDLE;
    m_queue->m_pendingWaitSemaphores[1] = VK_NULL_HANDLE;
    presentInfo.waitSemaphoreCount = (uint32_t)waitSemaphores.getCount();
    if (presentInfo.waitSemaphoreCount)
    {
        presentInfo.pWaitSemaphores = waitSemaphores.getBuffer();
    }
    if (m_currentImageIndex != -1)
        m_api->vkQueuePresentKHR(m_queue->m_queue, &presentInfo);
    return SLANG_OK;
}

int SwapchainImpl::acquireNextImage()
{
    if (!m_images.getCount())
    {
        m_queue->m_pendingWaitSemaphores[1] = VK_NULL_HANDLE;
        return -1;
    }

    m_currentImageIndex = -1;
    VkResult result = m_api->vkAcquireNextImageKHR(
        m_api->m_device,
        m_swapChain,
        UINT64_MAX,
        m_nextImageSemaphore,
        VK_NULL_HANDLE,
        (uint32_t*)&m_currentImageIndex);

    if (result != VK_SUCCESS)
    {
        m_currentImageIndex = -1;
        destroySwapchainAndImages();
        return m_currentImageIndex;
    }
    // Make the queue's next submit wait on `m_nextImageSemaphore`.
    m_queue->m_pendingWaitSemaphores[1] = m_nextImageSemaphore;
    return m_currentImageIndex;
}

Result SwapchainImpl::setFullScreenMode(bool mode) { return SLANG_FAIL; }

} // namespace vk

Result SLANG_MCALL createVKDevice(const IDevice::Desc* desc, IDevice** outRenderer)
{
    RefPtr<vk::DeviceImpl> result = new vk::DeviceImpl();
    SLANG_RETURN_ON_FAIL(result->initialize(*desc));
    returnComPtr(outRenderer, result);
    return SLANG_OK;
}

} // namespace gfx
