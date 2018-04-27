// vk-device-queue.cpp
#include "vk-device-queue.h"

#include <stdlib.h>
#include <stdio.h>

namespace renderer_test {
using namespace Slang;

VulkanDeviceQueue::~VulkanDeviceQueue()
{
    for (int i = 0; i < int(EventType::CountOf); ++i)
    {
        m_api->vkDestroySemaphore(m_api->m_device, m_semaphores[i], nullptr);
    }

    for (int i = 0; i < m_numCommandBuffers; i++)
    {
        m_api->vkFreeCommandBuffers(m_api->m_device, m_commandPool, 1, &m_commandBuffers[i]);
        m_api->vkDestroyFence(m_api->m_device, m_fences[i].fence, nullptr);
    }
    m_api->vkDestroyCommandPool(m_api->m_device, m_commandPool, nullptr);
}

SlangResult VulkanDeviceQueue::init(const VulkanApi& api, VkQueue graphicsQueue, int graphicsQueueIndex)
{
    assert(m_api == nullptr);

    for (int i = 0; i < int(EventType::CountOf); ++i)
    {
        m_semaphores[i] = VK_NULL_HANDLE;
        m_currentSemaphores[i] = VK_NULL_HANDLE;
    }
    
    m_numCommandBuffers = kMaxCommandBuffers;
    m_graphicsQueueIndex = graphicsQueueIndex;

    m_graphicsQueue = graphicsQueue;
    
    VkCommandPoolCreateInfo poolCreateInfo = {};
    poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    poolCreateInfo.queueFamilyIndex = graphicsQueueIndex;

    api.vkCreateCommandPool(api.m_device, &poolCreateInfo, nullptr, &m_commandPool);

    VkCommandBufferAllocateInfo commandInfo = {};
    commandInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandInfo.commandPool = m_commandPool;
    commandInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandInfo.commandBufferCount = 1;

    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = 0; // VK_FENCE_CREATE_SIGNALED_BIT;

    for (int i = 0; i < m_numCommandBuffers; i++)
    {
        Fence& fence = m_fences[i];

        api.vkAllocateCommandBuffers(api.m_device, &commandInfo, &m_commandBuffers[i]);

        api.vkCreateFence(api.m_device, &fenceCreateInfo, nullptr, &fence.fence);
        fence.active = false;
        fence.value = 0;
    }

    VkSemaphoreCreateInfo semaphoreCreateInfo = {};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    for (int i = 0; i < int(EventType::CountOf); ++i)
    {
        api.vkCreateSemaphore(api.m_device, &semaphoreCreateInfo, nullptr, &m_semaphores[i]);
    }
    
    // Second step of flush to prime command buffer
    flushStepB();

#if 0
    NvFlowContextDescVulkan contextDesc = {};
    contextDesc.vkGetInstanceProcAddr = ptr->device->loader.vkGetInstanceProcAddr;
    contextDesc.vkGetDeviceProcAddr = ptr->device->loader.vkGetDeviceProcAddr;
    contextDesc.instance = ptr->device->vulkanInstance;
    contextDesc.physicalDevice = ptr->device->physicalDevice;
    contextDesc.device = ptr->vulkanDevice;
    contextDesc.queue = ptr->graphicsQueue;
    contextDesc.commandBuffer = ptr->commandBuffer;
    contextDesc.lastFenceCompleted = ptr->lastFenceCompleted;
    contextDesc.nextFenceValue = ptr->nextFenceValue;

    ptr->internalContext = NvFlowCreateContextVulkan(NV_FLOW_VERSION, &contextDesc);
#endif

    m_api = &api;
    return SLANG_OK;
}



void VulkanDeviceQueue::flushStepA()
{
    //NvFlowContextEndRenderPass(ptr->internalContext);

    m_api->vkEndCommandBuffer(m_commandBuffer);

    VkPipelineStageFlags stageFlags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    // Wait semaphores
    if (m_currentSemaphores[int(EventType::BeginFrame)] != VK_NULL_HANDLE)
    {
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &m_currentSemaphores[int(EventType::BeginFrame)];
    }

    submitInfo.pWaitDstStageMask = &stageFlags;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_commandBuffer;
    
    // Signal semaphores
    if (m_currentSemaphores[int(EventType::EndFrame)] != VK_NULL_HANDLE)
    {
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &m_currentSemaphores[int(EventType::EndFrame)];
    }

    Fence& fence = m_fences[m_commandBufferIndex];

    m_api->vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, fence.fence);

    // mark signaled fence value
    fence.value = m_nextFenceValue;
    fence.active = true;
    
    // increment fence value
    m_nextFenceValue++;

    // No longer waiting on this semaphore
    makeCompleted(EventType::BeginFrame);
}

void VulkanDeviceQueue::fenceUpdate( int fenceIndex, bool blocking)
{
    Fence& fence = m_fences[fenceIndex];

    if (fence.active)
    {
        uint64_t timeout = blocking ? ~uint64_t(0) : 0;

        if (VK_SUCCESS == m_api->vkWaitForFences(m_api->m_device, 1, &fence.fence, VK_TRUE, timeout))
        {
            m_api->vkResetFences(m_api->m_device, 1, &fence.fence);

            fence.active = false;

            if (fence.value > m_lastFenceCompleted)
            {
                m_lastFenceCompleted = fence.value;
            }
        }
    }
}

void VulkanDeviceQueue::flushStepB()
{
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    m_commandBufferIndex = (m_commandBufferIndex + 1) % m_numCommandBuffers;
    m_commandBuffer = m_commandBuffers[m_commandBufferIndex];
    
    // non-blocking update of fence values
    for (int i = 0; i < m_numCommandBuffers; ++i)
    {
        fenceUpdate(i, false);
    }

    // blocking update of fence values
    fenceUpdate(m_commandBufferIndex, true);
    
    m_api->vkResetCommandBuffer(m_commandBuffer, 0);

    //m_api.vkResetCommandPool(m_api->m_device, m_commandPool, 0);

    m_api->vkBeginCommandBuffer(m_commandBuffer, &beginInfo);
}

void VulkanDeviceQueue::flushStep()
{
    flushStepA();
    flushStepB();
}

VkSemaphore VulkanDeviceQueue::makeCurrent(EventType eventType)
{
    assert(m_currentSemaphores[int(eventType)] == VK_NULL_HANDLE);
    VkSemaphore semaphore = m_semaphores[int(eventType)];
    m_currentSemaphores[int(eventType)] = semaphore;
    return semaphore;
}

void VulkanDeviceQueue::makeCompleted(EventType eventType)
{
    assert(m_currentSemaphores[int(eventType)] != VK_NULL_HANDLE);
    m_currentSemaphores[int(eventType)] = VK_NULL_HANDLE;
}

} // renderer_test
