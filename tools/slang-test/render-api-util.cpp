
#include "render-api-util.h"

#include "../../slang.h"

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

/* static */ Slang::Result RenderApiUtil::findApiFlagsByName(const Slang::UnownedStringSlice& name, RenderApiFlags* flagsOut)
{
    // Special case 'all'
    if (name == "all")
    {
        *flagsOut = RenderApiFlags(RenderApiFlag::AllOf);
        return SLANG_OK;
    }
    if (name == "none")
    {
        *flagsOut = RenderApiFlags(0);
        return SLANG_OK;
    }
    RenderApiType type = findApiTypeByName(name);
    if (type == RenderApiType::Unknown)
    {
        return SLANG_FAIL;
    }
    *flagsOut = RenderApiFlags(1) << int(type);
    return SLANG_OK;
}

static bool isNameStartChar(char c)
{
    return (c >= 'a' && c <='z') || (c >= 'A' && c <= 'Z') || (c == '_');
}

static bool isNameNextChar(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_') || (c >= '0' && c <= '9');
}

namespace { // anonymous
enum class Token 
{
    eError,
    eOp,
    eId,
    eEnd,
};
}

static Token nextToken(Slang::UnownedStringSlice& textInOut, Slang::UnownedStringSlice& lexemeOut)
{
    using namespace Slang;
    if (textInOut.size() <= 0)
    {
        return Token::eEnd;
    }
    const char* start = textInOut.begin();
    const char* end = textInOut.end();

    const char firstChar = start[0];
    if (firstChar == '-' || firstChar == '+')
    {
        lexemeOut = UnownedStringSlice(start, start + 1);
        textInOut = UnownedStringSlice(start + 1, end);
        return Token::eOp;
    }

    if (!isNameStartChar(firstChar))
    {
        lexemeOut = UnownedStringSlice(start, start + 1);
        return Token::eError;
    }
    const char* cur = start + 1;
    while (cur < end && isNameNextChar(*cur))
    {
        cur++;
    }

    lexemeOut = UnownedStringSlice(start, cur);
    textInOut = UnownedStringSlice(cur, end);
    return Token::eId;
}

/* static */Slang::Result RenderApiUtil::parseApiFlags(const Slang::UnownedStringSlice& textIn, RenderApiFlags initialFlags, RenderApiFlags* apiFlagsOut)
{
    using namespace Slang;

    UnownedStringSlice text(textIn);
    UnownedStringSlice lexeme;

    RenderApiFlags apiFlags = 0; 
    
    switch (nextToken(text, lexeme))
    {
        case Token::eOp:
        {
            // If we start with an op - we use the passed in values as the default
            // Rewind back to the start
            text = textIn;
            apiFlags = initialFlags;
            break;
        }
        case Token::eId:
        {
            // If we start with an Id - we use that as the starting state
            SLANG_RETURN_ON_FAIL(findApiFlagsByName(lexeme, &apiFlags));
            break;
        }
        default: return SLANG_FAIL;
    }
    
    while (true)
    {
        // Must have an op followed by an id unless we are at the end
        switch (nextToken(text, lexeme))
        {
            case Token::eEnd:
            {
                *apiFlagsOut = apiFlags;
                return SLANG_OK;
            }
            case Token::eOp:    break;
            default:            return SLANG_FAIL;
        }

        const char op = lexeme[0];
        if (nextToken(text, lexeme) != Token::eId)
        {
            return SLANG_FAIL;
        }

        RenderApiFlags flags;
        SLANG_RETURN_ON_FAIL(findApiFlagsByName(lexeme, &flags));

        if (op == '+')
        {
            apiFlags |= flags;
        }
        else
        {
            apiFlags &= ~flags;
        }
    }
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
