// slang-spirv-target-info.cpp
#include "slang-spirv-target-info.h"

#include "../../slang-com-helper.h"

#include "../core/slang-string-util.h"

namespace Slang {

/*
The problem here is that when specifying a SPIR-V version it actually implies to things
a target environment and a SPIR-V language version. The target strings here are designed to match
glslang as that is currently our main SPIR-V mechanism.

Additionally idea here is to do the majority of processing on the SLANG side before handing off a result to
glslang.

The reasons for this are multiple
1) Doing anything with glslang requires building new binaries - and making available, which is a pain
So we want to push heuristics/processing that might change to Slang 
2) We want the glslang API to be small and stable

The intention is to try and fallback on reasonable defaults. This applies to the glslang side too
that if a specific target is not found, it will use the SPIR-V version and the universal target.
*/

// NOTE! Only handles non 'universal' SPIR-V targets. Targets that are major.minor, or major.minor.patch,
// are just parsed.

namespace { // anonymous

struct SPIRVNameAndVersion
{
    const char* targetName;
    SPIRVVersion version;
};

} // anonymous

static const SPIRVNameAndVersion kSPIRVNameAndVersions[] =
{
    {"vk1.0",           SPIRVVersion()},
    {"cl2.1",           SPIRVVersion()},
    {"cl2.2",           SPIRVVersion()},
    {"gl4.0",           SPIRVVersion()},
    {"gl4.1",           SPIRVVersion()},
    {"gl4.2",           SPIRVVersion()},
    {"gl4.3",           SPIRVVersion()},
    {"gl4.5",           SPIRVVersion()},
    {"cl1.2",           SPIRVVersion()},
    {"cl_emb1.2",       SPIRVVersion()},
    {"cl2.0",           SPIRVVersion()},
    {"cl_emb2.0",       SPIRVVersion()},
    {"cl_emb2.1",       SPIRVVersion()},
    {"cl_emb2.2",       SPIRVVersion()},
    {"vk1.1",           SPIRVVersion()},
    {"web_gpu1.0",      SPIRVVersion()},
    {"vk1.1_spirv1.4",  SPIRVVersion(1, 4) },
};

/* static */SlangResult SPIRVTargetInfo::find(const UnownedStringSlice& inName, SPIRVTargetInfo& outInfo)
{
    {
        UnownedStringSlice name(inName);

        // Strip spirv or spv prefixes - which are just 'universal' spirv type
        if (name.startsWith(UnownedStringSlice::fromLiteral("spirv")))
        {
            name = UnownedStringSlice(name.begin() + 5, name.end());
        }
        else if (name.startsWith(UnownedStringSlice::fromLiteral("spv")))
        {
            name = UnownedStringSlice(name.begin() + 3, name.end());
        }

        {
            UnownedStringSlice slices[3];
            const Index splitCount = StringUtil::split(name, '.', 3, slices);
            if (splitCount > 0 && splitCount < 3)
            {
                Int ints[3] = { 0, 0, 0};
                for (Index i = 0; i < splitCount; i++)
                {
                    SLANG_RETURN_ON_FAIL(StringUtil::parseInt(slices[i], ints[i]));

                    if (ints[i] < 0 || ints[i] > 0xff)
                    {
                        return SLANG_FAIL;
                    }
                }

                outInfo.targetName = name;
                outInfo.version.m_major = uint8_t(ints[0]);
                outInfo.version.m_minor = uint8_t(ints[1]);
                outInfo.version.m_patch = uint8_t(ints[2]);
                outInfo.flags = SPIRVTargetFlag::Universal;
                return SLANG_OK;
            }
        }
    }

    // Let's just look it up
    const Index count = SLANG_COUNT_OF(kSPIRVNameAndVersions);
    for (int i = 0; i < count; ++i)
    {
        const auto& info = kSPIRVNameAndVersions[i];
        if (inName == info.targetName)
        {
            outInfo.flags = 0;
            outInfo.version = info.version;
            outInfo.targetName = inName; 
            return SLANG_OK;
        }
    }

    return SLANG_FAIL;
}

} // namespace Slang
