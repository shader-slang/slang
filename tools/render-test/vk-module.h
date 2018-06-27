﻿// vk-module.h
#pragma once

#include "../../slang.h"

#include "../../slang-com-helper.h"

#if SLANG_WINDOWS_FAMILY
#   define VK_USE_PLATFORM_WIN32_KHR 1
#else
#   define VK_USE_PLATFORM_XLIB_KHR 1
#endif

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

namespace renderer_test {

struct VulkanModule
{
        /// true if has been initialized
    SLANG_FORCE_INLINE bool isInitialized() const { return m_module != nullptr; }

        /// Get a function by name
    PFN_vkVoidFunction getFunction(const char* name) const;

        /// Initialize
    Slang::Result init();
        /// Destroy
    void destroy();    

        /// Dtor
    ~VulkanModule() { destroy(); }

    protected:
    void* m_module = nullptr;
};

} // renderer_test
