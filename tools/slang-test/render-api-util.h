#ifndef SLANG_RENDER_API_UTIL_H
#define SLANG_RENDER_API_UTIL_H

enum class RenderApiType
{
    kUnknown = -1,
    kOpenGl = 0,
    kVulkan,
    kD3D12,
    kD3D11,
    kCountOf,
};

// Use a struct wrapped Enum instead of enum class, cos we want to be able to manipulate as integrals
struct RenderApiFlag
{
    enum Enum
    {
        kOpenGl = 1 << int(RenderApiType::kOpenGl),
        kVulkan = 1 << int(RenderApiType::kVulkan),
        kD3D12 = 1 << int(RenderApiType::kD3D12),
        kD3D11 = 1 << int(RenderApiType::kD3D11),
        kAllOf = (1 << int(RenderApiType::kCountOf)) - 1                   ///< All bits set
    };
};

struct RenderApiUtil
{
    struct Info
    {
        RenderApiType type;               ///< The type
        const char* names;          ///< Comma separated list of names associated with the type
    };

        /// Returns true if the API is available. 
    static bool calcHasApi(RenderApiType type);

        /// Returns a combination of RenderApiFlag bits to say
    static int getAvailableApis();

    static const Info s_infos[int(RenderApiType::kCountOf)];
};

#endif // SLANG_RENDER_API_UTIL_H