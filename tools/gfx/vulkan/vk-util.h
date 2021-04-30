// vk-util.h
#pragma once

#include "vk-api.h"
#include "slang-gfx.h"

// Macros to make testing vulkan return codes simpler

/// SLANG_VK_RETURN_ON_FAIL can be used in a similar way to SLANG_RETURN_ON_FAIL macro, except it will turn a vulkan failure into Slang::Result in the process
/// Calls handleFail which on debug builds asserts
#define SLANG_VK_RETURN_ON_FAIL(x) { VkResult _res = x; if (_res != VK_SUCCESS) { return VulkanUtil::handleFail(_res); }  }

#define SLANG_VK_RETURN_NULL_ON_FAIL(x) { VkResult _res = x; if (_res != VK_SUCCESS) { VulkanUtil::handleFail(_res); return nullptr; }  }

/// Is similar to SLANG_VK_RETURN_ON_FAIL, but does not return. Will call checkFail on failure - which asserts on debug builds.
#define SLANG_VK_CHECK(x) {  VkResult _res = x; if (_res != VK_SUCCESS) { VulkanUtil::checkFail(_res); }  }

namespace gfx {

// Utility functions for Vulkan
struct VulkanUtil
{
        /// Get the equivalent VkFormat from the format
        /// Returns VK_FORMAT_UNDEFINED if a match is not found
    static VkFormat getVkFormat(Format format);

        /// Called by SLANG_VK_RETURN_FAIL if a res is a failure.
        /// On debug builds this will cause an assertion on failure.
    static Slang::Result handleFail(VkResult res);
        /// Called when a failure has occurred with SLANG_VK_CHECK - will typically assert.
    static void checkFail(VkResult res);

        /// Get the VkPrimitiveTopology for the given topology.
        /// Returns VK_PRIMITIVE_TOPOLOGY_MAX_ENUM on failure
    static VkPrimitiveTopology getVkPrimitiveTopology(PrimitiveTopology topology);

    static VkImageLayout mapResourceStateToLayout(ResourceState state);

        /// Returns Slang::Result equivalent of a VkResult
    static Slang::Result toSlangResult(VkResult res);

    static VkShaderStageFlags getShaderStage(SlangStage stage);

    static VkPipelineBindPoint getPipelineBindPoint(PipelineType pipelineType);

    static VkImageLayout getImageLayoutFromState(ResourceState state);
};

} // renderer_test
