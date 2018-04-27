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

        /// Initialize - must be called before anything else can be done
    SlangResult init(const VulkanApi& api, VkQueue queue, int queueIndex);

        /// Flushes the current command list, and steps to next (ie combination of step A then B)
    void flushStep();

        /// Blocks until all work submitted to GPU has completed
    void waitForIdle() { m_api->vkQueueWaitIdle(m_queue); }

        /// Set the graphics queue index (as set on init)
    int getQueueIndex() const { return m_queueIndex; }

    VkSemaphore makeCurrent(EventType eventType);

    void makeCompleted(EventType eventType);

        /// Get the command buffer
    VkCommandBuffer getCommandBuffer() const { return m_commandBuffer; }

        /// Get the queue
    VkQueue getQueue() const { return m_queue; }

        /// Get the API
    const VulkanApi* getApi() const { return m_api; }

        /// Flushes the current command list 
    void flushStepA();
        /// Steps to next command buffer and opens. May block if command buffer is still in use
    void flushStepB();

    void flushAndWait();

        /// Dtor
    ~VulkanDeviceQueue();

    protected:
        
    struct Fence
    {
        VkFence fence;
        bool  active;
        uint64_t value;
    };

    void fenceUpdate(int fenceIndex, bool blocking);

    VkQueue m_queue = VK_NULL_HANDLE;

    VkCommandPool m_commandPool = VK_NULL_HANDLE;
    int m_numCommandBuffers = 0;
    int m_commandBufferIndex = 0;
    // There are the same amount of command buffers as fences
    VkCommandBuffer m_commandBuffers[kMaxCommandBuffers] = { VK_NULL_HANDLE };

    Fence m_fences[kMaxCommandBuffers] = { {VK_NULL_HANDLE, 0, 0u} };
    
    VkCommandBuffer m_commandBuffer = VK_NULL_HANDLE;

    VkSemaphore m_semaphores[int(EventType::CountOf)];
    VkSemaphore m_currentSemaphores[int(EventType::CountOf)];

    uint64_t m_lastFenceCompleted = 1;
    uint64_t m_nextFenceValue = 2;

    int m_queueIndex = 0;

    const VulkanApi* m_api = nullptr;
};

} // renderer_test
