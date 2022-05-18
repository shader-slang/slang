// vk-fence.cpp
#include "vk-fence.h"

#include "vk-device.h"
#include "vk-util.h"

namespace gfx
{

using namespace Slang;

namespace vk
{

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

} // namespace vk
} // namespace gfx
