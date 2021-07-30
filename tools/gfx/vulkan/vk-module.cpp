// module.cpp
#include "vk-module.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#if SLANG_WINDOWS_FAMILY
#   include <windows.h>
#else
#   include <dlfcn.h>
#endif

#include "../renderer-shared.h"

namespace gfx {
using namespace Slang;

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! VulkanModule !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

Slang::Result VulkanModule::init(bool useSoftwareImpl)
{
    if (isInitialized())
    {
        destroy();
    }

    const char* dynamicLibraryName = "Unknown";
    m_isSoftware = useSoftwareImpl;

#if SLANG_WINDOWS_FAMILY
    dynamicLibraryName = useSoftwareImpl ? "vk_swiftshader.dll" : "vulkan-1.dll";
    HMODULE module = ::LoadLibraryA(dynamicLibraryName);
    m_module = (void*)module;
#else
    dynamicLibraryName = useSoftwareImpl ? "libvk_swiftshader.so" : "libvulkan.so.1";
    if (useSoftwareImpl)
    {
        dlopen("libpthread.so.0", RTLD_NOW | RTLD_GLOBAL);
    }
    m_module = dlopen(dynamicLibraryName, RTLD_NOW);
#endif

    if (!m_module)
    {
        return SLANG_FAIL;
    }

    return SLANG_OK;
}

PFN_vkVoidFunction VulkanModule::getFunction(const char* name) const
{
    assert(m_module);
    if (!m_module)
    {
        return nullptr;
    }
#if SLANG_WINDOWS_FAMILY
    return (PFN_vkVoidFunction)::GetProcAddress((HMODULE)m_module, name);
#else
    return (PFN_vkVoidFunction)dlsym(m_module, name);
#endif
}

void VulkanModule::destroy()
{
    if (!isInitialized())
    {
        return;
    }

#if SLANG_WINDOWS_FAMILY
    ::FreeLibrary((HMODULE)m_module);
#else
    dlclose(m_module);
#endif
    m_module = nullptr;
}

} // renderer_test
