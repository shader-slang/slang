// vk-swap-chain.h
#pragma once

#include "vk-api.h"

namespace renderer_test {

struct VulkanDeviceQueue
{
    enum 
    {
        kMaxCommandBuffers = 8,
    };

    SlangResult init(const VulkanApi& api, VkQueue graphicsQueue, int graphicsQueueIndex);

    void flushStep();

    void flushStepA();
    void flushStepB();

    void fenceUpdate(int fenceIndex, bool blocking);

    void waitForIdle() { m_api->vkQueueWaitIdle(m_graphicsQueue); }

    //NvFlowDeviceQueue deviceQueue;
    //NvFlowDeviceVulkan* device;

    VkQueue m_graphicsQueue = nullptr;

    struct Fence
    {
        VkFence fence;
        bool  active;
        uint64_t value;
    };

    VkCommandPool m_commandPool = VK_NULL_HANDLE;
    int m_numCommandBuffers = 0;
    int m_commandBufferIndex = 0;
    VkCommandBuffer m_commandBuffers[kMaxCommandBuffers] = { nullptr };
    Fence m_fences[kMaxCommandBuffers] = { {VK_NULL_HANDLE, 0, 0u} };
    VkCommandBuffer m_commandBuffer = nullptr;

    VkSemaphore m_beginFrameSemaphore = VK_NULL_HANDLE;
    VkSemaphore m_endFrameSemaphore = VK_NULL_HANDLE;

    VkSemaphore m_currentBeginFrameSemaphore = VK_NULL_HANDLE;
    VkSemaphore m_currentEndFrameSemaphore = VK_NULL_HANDLE;

    uint64_t m_lastFenceCompleted = 1;
    uint64_t m_nextFenceValue = 2;

    int m_graphicsQueueIndex = 0;

    //NvFlowContext* internalContext = nullptr;

    const VulkanApi* m_api = nullptr;
};

} // renderer_test
