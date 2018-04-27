// vk-util.h
#pragma once

#include "vk-api.h"
#include "render.h"

#define SLANG_VK_RETURN_ON_FAIL(x) { VkResult _res = x; if (_res != VK_SUCCESS) { return VulkanUtil::handleFail(_res); }  }

#define SLANG_VK_CHECK(x) {  VkResult _res = x; if (_res != VK_SUCCESS) { VulkanUtil::checkFail(_res); }  } 
    
namespace renderer_test {

// Utility functions for Vulkan
struct VulkanUtil
{
        /// Calculate the VkFormat from the renderer format
    static VkFormat calcVkFormat(Format format);
        /// Handles a failure
    static Slang::Result handleFail(VkResult res);

    static VkPrimitiveTopology calcVkPrimitiveTopology(PrimitiveTopology topology);

        /// Called when a failure has occured with SLANG_VK_CHECK - will typically assert.
    static void checkFail(VkResult res);

        /// Returns a vulkan result into a Slang::Result
    static Slang::Result toSlangResult(VkResult res);
};

} // renderer_test
