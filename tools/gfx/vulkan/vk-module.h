// vk-module.h
#pragma once

#include "slang.h"

#include "slang-com-helper.h"

#if SLANG_WINDOWS_FAMILY
#   define VK_USE_PLATFORM_WIN32_KHR 1
#else
#   define VK_USE_PLATFORM_XLIB_KHR 1
#endif

#define VK_NO_PROTOTYPES

#include <vulkan/vulkan.h>

// Undef xlib macros
#ifdef Always
#    undef Always
#endif
#ifdef None
#    undef None
#endif

namespace gfx {

struct VulkanModule
{
        /// true if has been initialized
    SLANG_FORCE_INLINE bool isInitialized() const { return m_module != nullptr; }

        /// Get a function by name
    PFN_vkVoidFunction getFunction(const char* name) const;

        /// true if using a software Vulkan implementation.
    bool isSoftware() const { return m_isSoftware; }

        /// Initialize
    Slang::Result init(bool useSoftwareImpl);
        /// Destroy
    void destroy();

        /// Dtor
    ~VulkanModule() { destroy(); }

 protected:
     void* m_module = nullptr;
     bool m_isSoftware = false;
};

} // renderer_test
