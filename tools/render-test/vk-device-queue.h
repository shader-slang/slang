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

    enum class EventType
    {
        BeginFrame,
        EndFrame,
        CountOf,
    };

    SlangResult init(const VulkanApi& api, VkQueue graphicsQueue, int graphicsQueueIndex);

    void flushStep();

    void fenceUpdate(int fenceIndex, bool blocking);

    void waitForIdle() { m_api->vkQueueWaitIdle(m_graphicsQueue); }

        /// Set the graphics queue index (as set on init)
    int getGraphicsQueueIndex() const { return m_graphicsQueueIndex; }

    VkSemaphore makeCurrent(EventType eventType);

    void makeCompleted(EventType eventType);

        /// Get the graphics queu
    VkQueue getGraphicsQueue() const { return m_graphicsQueue; }

        /// Get the API
    const VulkanApi* getApi() const { return m_api; }

    void flushStepA();
    void flushStepB();

        /// Dtor
    ~VulkanDeviceQueue();

    protected:

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
    // There are the same amount of command buffers as fences
    VkCommandBuffer m_commandBuffers[kMaxCommandBuffers] = { nullptr };

    Fence m_fences[kMaxCommandBuffers] = { {VK_NULL_HANDLE, 0, 0u} };
    
    VkCommandBuffer m_commandBuffer = nullptr;

    VkSemaphore m_semaphores[int(EventType::CountOf)];
    VkSemaphore m_currentSemaphores[int(EventType::CountOf)];

    uint64_t m_lastFenceCompleted = 1;
    uint64_t m_nextFenceValue = 2;

    int m_graphicsQueueIndex = 0;

    const VulkanApi* m_api = nullptr;
};

} // renderer_test
