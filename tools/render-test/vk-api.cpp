// vk-api.cpp
#include "vk-api.h"

namespace renderer_test {
using namespace Slang;

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! VulkanApi !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

#define VK_API_CHECK_FUNCTION(x) && (x != nullptr) 
#define  VK_API_CHECK_FUNCTIONS(FUNCTION_LIST) true FUNCTION_LIST(VK_API_CHECK_FUNCTION) 

bool VulkanApi::areDefined(ProcType type) const
{
    switch (type)
    {
        case ProcType::Global:          return VK_API_CHECK_FUNCTIONS(VK_API_ALL_GLOBAL_PROCS);
        case ProcType::Instance:        return VK_API_CHECK_FUNCTIONS(VK_API_ALL_INSTANCE_PROCS);
        case ProcType::Device:          return VK_API_CHECK_FUNCTIONS(VK_API_ALL_DEVICE_PROCS);
        default:
        {
            assert(!"Unhandled type");
            return false;
        }
    }
}

Slang::Result VulkanApi::initGlobalProcs(const VulkanModule& module)
{
#define VK_API_GET_GLOBAL_PROC(x) x = (PFN_##x)module.getFunction(#x);

    // Initialize all the global functions
    VK_API_ALL_GLOBAL_PROCS(VK_API_GET_GLOBAL_PROC)

    if (!areDefined(ProcType::Global))
    {
        return SLANG_FAIL;
    }
    m_module = &module;
    return SLANG_OK;
}

Slang::Result VulkanApi::initInstanceProcs(VkInstance instance)
{
    assert(instance && vkGetInstanceProcAddr != nullptr);

#define VK_API_GET_INSTANCE_PROC(x) x = (PFN_##x)vkGetInstanceProcAddr(instance, #x);

    VK_API_ALL_INSTANCE_PROCS(VK_API_GET_INSTANCE_PROC)

    if (!areDefined(ProcType::Instance))
    {
        return SLANG_FAIL;
    }

    m_instance = instance;
    return SLANG_OK;
}

Slang::Result VulkanApi::initDeviceProcs(VkPhysicalDevice physicalDevice, VkDevice device)
{
    assert(m_instance && device && vkGetDeviceProcAddr != nullptr);

#define VK_API_GET_DEVICE_PROC(x) x = (PFN_##x)vkGetDeviceProcAddr(device, #x);

    VK_API_ALL_DEVICE_PROCS(VK_API_GET_DEVICE_PROC)

    if (!areDefined(ProcType::Device))
    {
        return SLANG_FAIL;
    }

    m_device = device;
    m_physicalDevice = physicalDevice;
    return SLANG_OK;
}




} // renderer_test
