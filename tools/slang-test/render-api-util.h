#ifndef SLANG_RENDER_API_UTIL_H
#define SLANG_RENDER_API_UTIL_H

#include "../../source/core/slang-string.h"
#include "../../source/core/slang-result.h"


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

        /// Returns a combination of RenderApiFlag bits which if set indicates that the API is available.
    static int getAvailableApis();

        /// Returns -1 if unknown
    static RenderApiType findApiTypeByName(const Slang::UnownedStringSlice& name);
        /// Returns 0 if none found.
    static int findApiFlagsByName(const Slang::UnownedStringSlice& name);

        /// Parse api flags string (comma delimited list of api names, or 'all' for all)
        /// For example "all,-dx12" would be all apis, except dx12
    static Slang::Result parseApiFlags(const Slang::UnownedStringSlice& text, int* apiBitsOut);

        /// Get information about a render API
    static const Info& getInfo(RenderApiType type) { return s_infos[int(type)]; }

        /// Static information about each render api
    static const Info s_infos[int(RenderApiType::kCountOf)];
};

#endif // SLANG_RENDER_API_UTIL_H