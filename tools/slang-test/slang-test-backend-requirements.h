// slang-test-backend-requirements.h
#ifndef SLANG_TEST_BACKEND_REQUIREMENTS_H
#define SLANG_TEST_BACKEND_REQUIREMENTS_H

#include "core/slang-list.h"
#include "core/slang-string.h"
#include "slang.h"

namespace Slang
{
namespace SlangTest
{

/// Returns the downstream backend that a slangc command line forces regardless of its `-target`,
/// or `SLANG_PASS_THROUGH_NONE` when it forces none. At most one flag is expected to force a
/// backend, so the first one found is returned.
///
/// Some flags select a downstream compiler independently of the target, so a test using one depends
/// on that backend even when the `-target` alone would not imply it. `-emit-cpu-via-llvm` routes
/// CPU/host-callable code generation through the slang-llvm plugin, so a test using it needs the
/// LLVM backend even when its target maps only to a generic C/C++ compiler (for example
/// `-target host-callable`, which otherwise implies `Generic_C_CPP`). Without the plugin slangc
/// fails with "unable to generate code for target '...'" before any IR pass runs, so the harness
/// must treat such a test as requiring the LLVM backend — and ignore it where slang-llvm is
/// unavailable — rather than run it and fail on a later diagnostic that is never reached.
inline SlangPassThrough getForcedDownstreamBackend(const List<String>& args)
{
    for (const auto& arg : args)
    {
        if (arg == "-emit-cpu-via-llvm")
            return SLANG_PASS_THROUGH_LLVM;
    }
    return SLANG_PASS_THROUGH_NONE;
}

} // namespace SlangTest
} // namespace Slang

#endif
