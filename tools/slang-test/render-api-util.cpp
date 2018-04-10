
#include "render-api-util.h"
#include "../../source/core/slang-defines.h"

/* static */const RenderApiUtil::Info RenderApiUtil::s_infos[] =
{
    { RenderApiType::kOpenGl, "gl,ogl,opengl"},
    { RenderApiType::kVulkan, "vk,vulkan"},
    { RenderApiType::kD3D12,  "dx12,d3d12"},
    { RenderApiType::kD3D11,  "dx11,d3d11"},
};

#if SLANG_WINDOWS_FAMILY

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#undef WIN32_LEAN_AND_MEAN
#undef NOMINMAX

namespace { // anonymous

class WinModule
{
    public:
        
        /// Initialize. Returns the module on success
    HMODULE init(const char* name)
    {
        if (m_module)
        {
            ::FreeModule(m_module);
            m_module = nullptr;
        }
        return m_module = ::LoadLibraryA(name);
    }
    
        /// convert to HMODULE
    SLANG_FORCE_INLINE operator HMODULE() const { return m_module; }
    
        /// True if loaded
    bool isLoaded() const { return m_module != nullptr; }

    explicit WinModule(const char* name):
        m_module(nullptr)
    {
        init(name);
    }

        /// Ctor
    WinModule():m_module(nullptr) {}
        /// Dtor
    ~WinModule() 
    { 
        if (m_module)
        {
            ::FreeLibrary(m_module); 
        }
    }

    protected:
    HMODULE m_module;
};

} // anonymous

#else
#endif

/* static */bool RenderApiUtil::calcHasApi(RenderApiType type)
{
#if SLANG_WINDOWS_FAMILY
    switch (type)
    {
        case RenderApiType::kOpenGl:    return WinModule("opengl32.dll").isLoaded();
        case RenderApiType::kVulkan:    return WinModule("vulkan-1.dll").isLoaded();
        case RenderApiType::kD3D11:     return WinModule("d3d11.dll").isLoaded(); 
        case RenderApiType::kD3D12:     return WinModule("d3d12.dll").isLoaded();

        default: return false;
    }

#else

    switch (type)
    {
        case RenderApiType::kOpenGl:
        case RenderApiType::kVulkan:
        {
            return true;
        }
        default: return false;
    }
#endif

}

static int _calcAvailableApis()
{
    int flags = 0;
    for (int i = 0; i < int(RenderApiType::kCountOf); i++)
    {
        if (RenderApiUtil::calcHasApi(RenderApiType(i)))
        {
            flags |= (1 << i);
        }
    }

    return flags;
}

/* static */int RenderApiUtil::getAvailableApis()
{
    static int s_availableApis = _calcAvailableApis();
    return s_availableApis;
}