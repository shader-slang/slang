// unit-test-cuda-sm-capability-mapping.cpp
//
// Every CUDA SM capability and its underlying atom must map to the same numbered
// architecture. Capabilities such as atomic_reduce may imply an SM but do not name one.

#include "core/slang-string-util.h"
#include "slang/slang-capability.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

SLANG_UNIT_TEST(cudaSMCapabilityVersionMappingIsComplete, NoRenderApi)
{
    const auto prefix = UnownedStringSlice::fromLiteral("cuda_sm_");
    int cudaNameCount = 0;
    for (Index i = 1; i < Index(CapabilityName::Count); ++i)
    {
        auto capability = CapabilityName(i);
        auto name = capabilityNameToString(capability);
        auto publicName = name.startsWith("_") ? name.tail(1) : name;
        const SemanticVersion version = getCUDASMVersion(capability);
        if (!publicName.startsWith(prefix))
        {
            SLANG_CHECK_MSG(!version.isSet(), "feature capability unexpectedly selects a CUDA SM");
            continue;
        }
        ++cudaNameCount;
        if (!version.isSet())
            getTestReporter()->message(TestMessageType::Info, String(name).getBuffer());
        SLANG_CHECK_MSG(version.isSet(), "CUDA SM capability has no version mapping");

        // The mapped version must equal the digits in the atom name (e.g. `_cuda_sm_8_9`
        // -> 8.9), so a wrong entry is caught, not only a missing one.
        const UnownedStringSlice digits = publicName.tail(prefix.getLength());
        const Index sep = digits.indexOf('_');
        SLANG_CHECK_MSG(sep >= 0, "malformed _cuda_sm_* atom name");
        if (sep < 0)
            continue;

        Int major = 0;
        Int minor = 0;
        SLANG_CHECK(SLANG_SUCCEEDED(StringUtil::parseInt(digits.head(sep), major)));
        SLANG_CHECK(SLANG_SUCCEEDED(StringUtil::parseInt(digits.tail(sep + 1), minor)));
        const bool matches = Int(version.m_major) == major && Int(version.m_minor) == minor;
        if (!matches)
            getTestReporter()->message(TestMessageType::Info, String(name).getBuffer());
        SLANG_CHECK_MSG(matches, "_cuda_sm_* atom maps to a version that disagrees with its name");
    }

    SLANG_CHECK_MSG(cudaNameCount > 0, "expected at least one _cuda_sm_* capability atom");
}
