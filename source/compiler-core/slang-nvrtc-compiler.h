#ifndef SLANG_NVRTC_COMPILER_UTIL_H
#define SLANG_NVRTC_COMPILER_UTIL_H

#include "core/slang-platform.h"
#include "slang-downstream-compiler-util.h"

namespace Slang
{


struct NVRTCDownstreamCompilerUtil
{
    static SlangResult locateCompilers(
        const String& path,
        ISlangSharedLibraryLoader* loader,
        DownstreamCompilerSet* set);

    /// Pick the architecture to ask NVRTC for, given what it reports it accepts.
    ///
    /// `supportedAscending` is NVRTC's own list, as `major * 10 + minor` in
    /// ascending order. A requirement need not name a real architecture --
    /// `__cuda_sm_version` takes an arbitrary version -- so the answer is the
    /// smallest supported architecture that *satisfies* `requested`:
    /// `8.1` against `{80, 86, 89, 90}` resolves to `8.6`, not `8.0`, because
    /// `8.0` would not provide what the code asked for.
    ///
    /// A requirement above everything supported is clamped to the highest,
    /// which cannot satisfy it. That is deliberate: passing the request through
    /// fails the compile with an error naming the architecture, which says
    /// nothing about the user's shader, whereas compiling against the highest
    /// available makes NVRTC report the specific construct it cannot compile.
    ///
    /// Returns `requested` unchanged if `supportedAscending` is empty.
    static SemanticVersion resolveArchAgainstSupported(
        SemanticVersion requested,
        const List<int>& supportedAscending);
};

} // namespace Slang

#endif
