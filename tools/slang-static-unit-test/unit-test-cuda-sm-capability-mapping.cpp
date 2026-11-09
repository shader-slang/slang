// unit-test-cuda-sm-capability-mapping.cpp
//
// Every `_cuda_sm_X_Y` capability atom must map, via getCUDASMVersionForAtom, to a CUDA
// SM version whose digits match the atom name; this test enforces that invariant so the
// atom list and the version mapping cannot drift apart.
//
// It lives in slang-static-unit-test because it calls getCUDASMVersionForAtom and
// capabilityNameToString directly, and neither carries an export annotation, so they are
// reachable only when the compiler is linked statically.

#include "core/slang-string-util.h"
#include "slang/slang-capability.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

SLANG_UNIT_TEST(cudaSMCapabilityVersionMappingIsComplete)
{
    // SLANG_CHECK_MSG takes a string literal, so the specific offending atom is logged
    // separately (as the other tests in this directory do) to keep a failure diagnosable.
    const UnownedStringSlice prefix = UnownedStringSlice::fromLiteral("_cuda_sm_");
    int cudaAtomCount = 0;
    for (Index i = 1; i < Index(CapabilityAtom::Count); ++i)
    {
        // `CapabilityAtom(i)` and `CapabilityName(i)` denote the same atom over this range: the
        // generator emits `CapabilityName::<atom> = (int)CapabilityAtom::<atom>` for Normal atoms,
        // so the two enums share integer values across [1, CapabilityAtom::Count).
        const CapabilityAtom atom = CapabilityAtom(i);
        const UnownedStringSlice name = capabilityNameToString(CapabilityName(i));
        if (!name.startsWith(prefix))
        {
            // The call site gates on `.isSet()`, so a non-CUDA atom must map to an unset version;
            // assert it here so a mis-added case cannot give a non-CUDA target a spurious `-arch`.
            SLANG_CHECK_MSG(
                !getCUDASMVersionForAtom(atom).isSet(),
                "non-_cuda_sm_* atom unexpectedly maps to a CUDA SM version");
            continue;
        }
        ++cudaAtomCount;

        const SemanticVersion version = getCUDASMVersionForAtom(atom);
        if (!version.isSet())
            getTestReporter()->message(TestMessageType::Info, String(name).getBuffer());
        SLANG_CHECK_MSG(version.isSet(), "_cuda_sm_* atom has no CUDA SM version mapping");

        // The mapped version must equal the digits in the atom name (e.g. `_cuda_sm_8_9`
        // -> 8.9), so a wrong entry is caught, not only a missing one.
        const UnownedStringSlice digits = name.tail(prefix.getLength());
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

    SLANG_CHECK_MSG(cudaAtomCount > 0, "expected at least one _cuda_sm_* capability atom");
}
