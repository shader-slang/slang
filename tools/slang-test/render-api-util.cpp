
#include "render-api-util.h"

#include "../../source/core/slang-defines.h"

#include "../../source/core/list.h"
#include "../../source/core/slang-string-util.h"

/* static */const RenderApiUtil::Info RenderApiUtil::s_infos[] =
{
    { RenderApiType::OpenGl, "gl,ogl,opengl"},
    { RenderApiType::Vulkan, "vk,vulkan"},
    { RenderApiType::D3D12,  "dx12,d3d12"},
    { RenderApiType::D3D11,  "dx11,d3d11"},
};

static int _calcAvailableApis()
{
    int flags = 0;
    for (int i = 0; i < int(RenderApiType::CountOf); i++)
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

/* static */RenderApiType RenderApiUtil::findApiTypeByName(const Slang::UnownedStringSlice& name)
{
    using namespace Slang;
    List<UnownedStringSlice> namesList;
    for (int j = 0; j < SLANG_COUNT_OF(RenderApiUtil::s_infos); j++)
    {
        const auto& apiInfo = RenderApiUtil::s_infos[j];
        const UnownedStringSlice names(apiInfo.names);

        if (names.indexOf(',') >= 0)
        {
            StringUtil::split(names, ',', namesList);
            if (namesList.IndexOf(name) != UInt(-1))
            {
                return apiInfo.type;
            }
        }
        else if (names == name)
        {
            return apiInfo.type;
        }
    }
    return RenderApiType::Unknown;
}

/* static */int RenderApiUtil::findApiFlagsByName(const Slang::UnownedStringSlice& name)
{
    // Special case 'all'
    if (name == "all")
    {
        return int(RenderApiFlag::AllOf);
    }
    RenderApiType type = findApiTypeByName(name);
    return (type == RenderApiType::Unknown) ? 0 : (1 << int(type));
}

/* static */Slang::Result RenderApiUtil::parseApiFlags(const Slang::UnownedStringSlice& text, int* apiBitsOut)
{
    using namespace Slang;

    int apiBits = 0;

    List<UnownedStringSlice> slices;
    StringUtil::split(text, ',', slices);

    for (int i = 0; i < int(slices.Count()); ++i)
    {
        UnownedStringSlice slice = slices[i];
        bool add = true;
        if (slice.size() <= 0)
        {
            return SLANG_FAIL;
        }
        if (slice[0] == '+')
        {
            // Drop the +
            slice = UnownedStringSlice(slice.begin() + 1, slice.end());
        }
        else if (slice[0] == '-')
        {
            add = false;
            // Drop the +
            slice = UnownedStringSlice(slice.begin() + 1, slice.end());
        }

        // We need to find the bits... 
        int bits = findApiFlagsByName(slice);
        // 0 means an error
        if (bits == 0)
        {
            return SLANG_FAIL;
        }

        if (add)
        {
            apiBits |= bits;
        }
        else
        {
            apiBits &= ~bits;
        }
    }

    *apiBitsOut = apiBits;
    return SLANG_OK;
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!! Platform specific stuff !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */


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

    explicit WinModule(const char* name) :
        m_module(nullptr)
    {
        init(name);
    }

    /// Ctor
    WinModule() :m_module(nullptr) {}
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
        case RenderApiType::OpenGl:    return WinModule("opengl32.dll").isLoaded();
        case RenderApiType::Vulkan:    return WinModule("vulkan-1.dll").isLoaded();
        case RenderApiType::D3D11:     return WinModule("d3d11.dll").isLoaded();
        case RenderApiType::D3D12:     return WinModule("d3d12.dll").isLoaded();

        default: return false;
    }

#else

    switch (type)
    {
        case RenderApiType::OpenGl:
        case RenderApiType::Vulkan:
        {
            return true;
        }
        default: return false;
    }
#endif

}
