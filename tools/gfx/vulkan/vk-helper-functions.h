// vk-helper-functions.h
#pragma once

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

Size calcRowSize(Format format, int width);
GfxCount calcNumRows(Format format, int height);

VkAttachmentLoadOp translateLoadOp(IRenderPassLayout::TargetLoadOp loadOp);
VkAttachmentStoreOp translateStoreOp(IRenderPassLayout::TargetStoreOp storeOp);
VkPipelineCreateFlags translateRayTracingPipelineFlags(RayTracingPipelineFlags::Enum flags);

uint32_t getMipLevelSize(uint32_t mipLevel, uint32_t size);
VkImageLayout translateImageLayout(ResourceState state);

VkAccessFlagBits calcAccessFlags(ResourceState state);
VkPipelineStageFlagBits calcPipelineStageFlags(ResourceState state, bool src);
VkAccessFlags translateAccelerationStructureAccessFlag(AccessFlag access);

VkBufferUsageFlagBits _calcBufferUsageFlags(ResourceState state);
VkBufferUsageFlagBits _calcBufferUsageFlags(ResourceStateSet states);
VkImageUsageFlagBits _calcImageUsageFlags(ResourceState state);
VkImageViewType _calcImageViewType(ITextureResource::Type type, const ITextureResource::Desc& desc);
VkImageUsageFlagBits _calcImageUsageFlags(ResourceStateSet states);
VkImageUsageFlags _calcImageUsageFlags(
    ResourceStateSet states, MemoryType memoryType, const void* initData);

VkAccessFlags calcAccessFlagsFromImageLayout(VkImageLayout layout);
VkPipelineStageFlags calcPipelineStageFlagsFromImageLayout(VkImageLayout layout);

void _writeTimestamp(
    VulkanApi* api, VkCommandBuffer vkCmdBuffer, IQueryPool* queryPool, SlangInt index);

} // namespace vk
} // namespace gfx
