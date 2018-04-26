// vk-swap-chain.cpp
#include "vk-swap-chain.h"

#include "../../source/core/list.h"

#include <stdlib.h>
#include <stdio.h>

namespace renderer_test {
using namespace Slang;

SlangResult VulkanSwapChain::init(VulkanDeviceQueue* deviceQueue, const Desc& desc, const PlatformDesc* platformDescIn)
{
    assert(platformDescIn);

    m_deviceQueue = deviceQueue;
    m_api = deviceQueue->m_api;
    
#if SLANG_WINDOWS_FAMILY
    const WinPlatformDesc* platformDesc = static_cast<const WinPlatformDesc*>(platformDescIn);
    _setPlatformDesc(*platformDesc);

    VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.hinstance = platformDesc->m_hinstance;
    surfaceCreateInfo.hwnd = platformDesc->m_hwnd;

    m_api->vkCreateWin32SurfaceKHR(m_api->m_instance, &surfaceCreateInfo, nullptr, &m_surface);
#else
    const XPlatformDesc* platformDesc = static_cast<const XPlatformDesc*>(platformDescIn);
    _setPlatformDesc(*platformDesc);

    VkXlibSurfaceCreateInfoKHR surfaceCreateInfo = {};
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.dpy = platformDesc->m_display; 
    surfaceCreateInfo.window = platformDesc->m_window;
    
    m_api->vkCreateXlibSurfaceKHR(m_api->m_instance, &surfaceCreateInfo, nullptr, &m_surface);
#endif

    VkBool32 supported = false;
    m_api->vkGetPhysicalDeviceSurfaceSupportKHR(m_api->m_physicalDevice, deviceQueue->m_graphicsQueueIndex, m_surface, &supported);

    uint32_t numSurfaceFormats = 0;
    List<VkSurfaceFormatKHR> surfaceFormats;
    m_api->vkGetPhysicalDeviceSurfaceFormatsKHR(m_api->m_physicalDevice, m_surface, &numSurfaceFormats, nullptr);
    surfaceFormats.SetSize(int(numSurfaceFormats));
    m_api->vkGetPhysicalDeviceSurfaceFormatsKHR(m_api->m_physicalDevice, m_surface, &numSurfaceFormats, surfaceFormats.Buffer());

    //m_swapchainFormat = NvFlowVulkan_convertToVulkan(ptr->swapchain.desc.format);

    /*
    // try to find BGR, fall back to RGB
    bool formatFound = false;
    for (uint32_t i = 0; i < surfaceCount; i++)
    {
    if (surfaceFormats[i].format == VK_FORMAT_B8G8R8A8_UNORM)
    {
    ptr->swapchainFormat = surfaceFormats[i].format;
    formatFound = true;
    break;
    }
    }
    if (!formatFound)
    {
    for (uint32_t i = 0; i < surfaceCount; i++)
    {
    if (surfaceFormats[i].format == VK_FORMAT_R8G8B8A8_UNORM)
    {
    ptr->swapchainFormat = surfaceFormats[i].format;
    formatFound = true;
    break;
    }
    }
    }
    */

    return initSwapchain();
}

void VulkanSwapChain::getWindowSize(int* widthOut, int* heightOut) const
{
#if SLANG_WINDOWS_FAMILY
    auto platformDesc = _getPlatformDesc<WinPlatformDesc>();

    RECT rc;
    ::GetClientRect(platformDesc->m_hwnd, &rc);
    *widthOut = rc.right - rc.left;
    *heightOut = rc.bottom - rc.top;
#else
    auto platformDesc = _getPlatformDesc<XPlatformDesc>();

    XWindowAttributes winAttr = {}; 
    XGetWindowAttributes(platformDesc->m_display, platformDesc->m_window, &winAttr);

    *widthOut = winAttr.width;
    *heightOut = winAttr.height;
#endif
}

SlangResult VulkanSwapChain::initSwapchain()
{
    int width, height; 
    getWindowSize(&width, &height);

    VkExtent2D imageExtent = {};
    imageExtent.width = width;
    imageExtent.height = height;

    m_width = width;
    m_height = height;

    // catch this before throwing error
    if (m_width == 0 || m_height == 0)
    {
        return SLANG_FAIL;
    }

    List<VkPresentModeKHR> presentModes;
    uint32_t numPresentModes = 0;
    m_api->vkGetPhysicalDeviceSurfacePresentModesKHR(m_api->m_physicalDevice, m_surface, &numPresentModes, nullptr);
    presentModes.SetSize(numPresentModes);
    m_api->vkGetPhysicalDeviceSurfacePresentModesKHR(m_api->m_physicalDevice, m_surface, &numPresentModes, presentModes.Buffer());

    VkPresentModeKHR presentOptions[] = { VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_FIFO_KHR };
    if (m_vsync)
    {
        presentOptions[0] = VK_PRESENT_MODE_FIFO_KHR;
        presentOptions[1] = VK_PRESENT_MODE_IMMEDIATE_KHR;
        presentOptions[2] = VK_PRESENT_MODE_MAILBOX_KHR;
    }

    for (int j = 0; j < 3; j++)
    {
        for (uint32_t i = 0; i < numPresentModes; i++)
        {
            if (presentModes[i] == presentOptions[j])
            {
                m_presentMode = presentOptions[j];
                j = 3;
                break;
            }
        }
    }

    VkSwapchainKHR oldSwapchain = nullptr;

    VkSwapchainCreateInfoKHR swapchainDesc = {};
    swapchainDesc.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainDesc.surface = m_surface;
    swapchainDesc.minImageCount = 3;
    swapchainDesc.imageFormat = m_swapchainFormat;
    swapchainDesc.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    swapchainDesc.imageExtent = imageExtent;
    swapchainDesc.imageArrayLayers = 1;
    swapchainDesc.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    swapchainDesc.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchainDesc.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchainDesc.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainDesc.presentMode = m_presentMode;
    swapchainDesc.clipped = VK_TRUE;
    swapchainDesc.oldSwapchain = oldSwapchain;

    VkResult result = m_api->vkCreateSwapchainKHR(m_api->m_device, &swapchainDesc, nullptr, &m_swapChain);

    if (result != VK_SUCCESS)
    {
        return SLANG_FAIL;
    }

    uint32_t numSwapChainImages = 0;
    m_api->vkGetSwapchainImagesKHR(m_api->m_device, m_swapChain, &numSwapChainImages, nullptr);
    if (numSwapChainImages > kMaxImages)
    {
        numSwapChainImages = kMaxImages;
    }

    m_api->vkGetSwapchainImagesKHR(m_api->m_device, m_swapChain, &numSwapChainImages, m_images);
    m_numImages = int(numSwapChainImages);

#if 0
    for (NvFlowUint idx = 0; idx < ptr->numSwapchainImages; idx++)
    {
        NvFlowRenderTargetDescVulkan renderTargetDesc = {};
        renderTargetDesc.texture = ptr->swapchainImages[idx];
        renderTargetDesc.format = ptr->swapchainFormat;
        renderTargetDesc.width = imageExtent.width;
        renderTargetDesc.height = imageExtent.height;

        ptr->renderTargets[idx] = NvFlowCreateRenderTargetExternalVulkan(ptr->deviceQueue->internalContext, &renderTargetDesc);
    }

    if (ptr->swapchain.desc.enableDepth)
    {
        NvFlowDepthBufferDesc depthBufferDesc = {};
        depthBufferDesc.format_typeless = ptr->swapchain.desc.depthFormat_typeless;
        depthBufferDesc.format_depth = ptr->swapchain.desc.depthFormat_depth;
        depthBufferDesc.format_texture = ptr->swapchain.desc.depthFormat_texture;
        depthBufferDesc.width = ptr->width;
        depthBufferDesc.height = ptr->height;

        ptr->depthBuffer = NvFlowCreateDepthBuffer(ptr->deviceQueue->internalContext, &depthBufferDesc);
    }
#endif

    return SLANG_OK;
}

void VulkanSwapChain::destroySwapchain()
{
    m_deviceQueue->waitForIdle();

#if 0
    if (m_depthBuffer)
    {
        NvFlowDestroyDepthBuffer(ptr->deviceQueue->internalContext, ptr->depthBuffer);
        ptr->depthBuffer = nullptr;
    }

    for (NvFlowUint idx = 0; idx < ptr->numSwapchainImages; idx++)
    {
        NvFlowDestroyRenderTarget(ptr->deviceQueue->internalContext, ptr->renderTargets[idx]);
        ptr->renderTargets[idx] = VK_NULL_HANDLE;
    }
#endif

    if (m_swapChain != VK_NULL_HANDLE)
    {
        m_api->vkDestroySwapchainKHR(m_api->m_device, m_swapChain, nullptr);
        m_swapChain = VK_NULL_HANDLE;
    }
}

VulkanSwapChain::~VulkanSwapChain()
{
    destroySwapchain();
   
    if (m_surface)
    {
        m_api->vkDestroySurfaceKHR(m_api->m_instance, m_surface, nullptr);
        m_surface = VK_NULL_HANDLE;
    }
}

TextureResource* VulkanSwapChain::getFrontRenderTargetVulkan()
{
    m_deviceQueue->m_currentBeginFrameSemaphore = m_deviceQueue->m_beginFrameSemaphore;

    uint32_t swapChainIndex = 0;
    VkResult result = m_api->vkAcquireNextImageKHR(m_api->m_device, m_swapChain, UINT64_MAX, m_deviceQueue->m_currentBeginFrameSemaphore, VK_NULL_HANDLE, &swapChainIndex);

    if (result != VK_SUCCESS)
    {
        destroySwapchain();

        //ptr->valid = NV_FLOW_FALSE;
        return nullptr;
    }
    m_currentSwapChainIndex = int(swapChainIndex);

    //return ptr->renderTargets[ptr->currentSwapchainIdx];
    return nullptr;
}

void VulkanSwapChain::present(bool vsync)
{
#if 0
    if (ptr->valid == NV_FLOW_FALSE)
    {
        NvFlowDeviceQueueFlush(&ptr->deviceQueue->deviceQueue);
        return;
    }
#endif

    m_deviceQueue->m_currentEndFrameSemaphore = m_deviceQueue->m_endFrameSemaphore;

    m_deviceQueue->flushStepA();
    
    uint32_t swapChainIndices[] = { uint32_t(m_currentSwapChainIndex) };

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &m_swapChain; 
    presentInfo.pImageIndices = swapChainIndices;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &m_deviceQueue->m_currentEndFrameSemaphore;

    VkResult result = m_api->vkQueuePresentKHR(m_deviceQueue->m_graphicsQueue, &presentInfo);

    m_deviceQueue->m_currentEndFrameSemaphore = VK_NULL_HANDLE;

    m_deviceQueue->flushStepB();
    
#if 0
    if (result != VK_SUCCESS || m_vsync != vsync)
    {
        m_vsync = vsync;
        destroySwapchain();
        ptr->valid = NV_FLOW_FALSE;
    }
#endif
}



} // renderer_test
