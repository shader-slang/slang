// unit-test-nvrtc-arch-resolve.cpp

#include "compiler-core/slang-nvrtc-compiler.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// Resolving a requested CUDA architecture against the set NVRTC reports it
// accepts. This is pure, so it can be checked without an NVRTC to load --
// which matters because the interesting cases (a requirement that names no real
// architecture, and one above everything supported) cannot be reached through
// `-capability` on the toolkits available to CI.
SLANG_UNIT_TEST(nvrtcArchResolve)
{
    // A representative report from NVRTC 12.6.
    const List<int> supported = {50, 52, 53, 60, 61, 62, 70, 72, 75, 80, 86, 87, 89, 90};

    const auto resolve = [&](SemanticVersion requested)
    { return NVRTCDownstreamCompilerUtil::resolveArchAgainstSupported(requested, supported); };

    // An architecture that is in the set is returned unchanged.
    SLANG_CHECK(resolve(SemanticVersion(8, 0)) == SemanticVersion(8, 0));
    SLANG_CHECK(resolve(SemanticVersion(5, 0)) == SemanticVersion(5, 0));
    SLANG_CHECK(resolve(SemanticVersion(9, 0)) == SemanticVersion(9, 0));

    // A requirement between two supported architectures rounds *up*, to the
    // smallest one that actually satisfies it. Rounding down would hand back an
    // architecture lacking what the code asked for.
    SLANG_CHECK(resolve(SemanticVersion(8, 1)) == SemanticVersion(8, 6));
    SLANG_CHECK(resolve(SemanticVersion(6, 3)) == SemanticVersion(7, 0));
    SLANG_CHECK(resolve(SemanticVersion(8, 8)) == SemanticVersion(8, 9));

    // Below everything supported resolves to the lowest, never to the highest.
    SLANG_CHECK(resolve(SemanticVersion(3, 0)) == SemanticVersion(5, 0));
    SLANG_CHECK(resolve(SemanticVersion(4, 9)) == SemanticVersion(5, 0));

    // Above everything supported clamps to the highest. It cannot satisfy the
    // request; compiling against the best available makes NVRTC report the
    // construct it cannot compile, rather than failing with an error that only
    // names the architecture.
    SLANG_CHECK(resolve(SemanticVersion(12, 0)) == SemanticVersion(9, 0));
    SLANG_CHECK(resolve(SemanticVersion(9, 1)) == SemanticVersion(9, 0));

    // An empty report means the loaded NVRTC could not be asked, so the request
    // is left alone for the caller's own floor logic to have decided.
    const List<int> none;
    SLANG_CHECK(
        NVRTCDownstreamCompilerUtil::resolveArchAgainstSupported(SemanticVersion(8, 1), none) ==
        SemanticVersion(8, 1));

    // A report with a single entry is still resolved on both sides of it.
    const List<int> onlyOne = {80};
    const auto resolveOne = [&](SemanticVersion v)
    { return NVRTCDownstreamCompilerUtil::resolveArchAgainstSupported(v, onlyOne); };
    SLANG_CHECK(resolveOne(SemanticVersion(7, 0)) == SemanticVersion(8, 0));
    SLANG_CHECK(resolveOne(SemanticVersion(8, 0)) == SemanticVersion(8, 0));
    SLANG_CHECK(resolveOne(SemanticVersion(9, 0)) == SemanticVersion(8, 0));
}
