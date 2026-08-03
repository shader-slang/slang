// unit-test-slang-test-backend-requirements.cpp

#include "slang-test/slang-test-backend-requirements.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

SLANG_UNIT_TEST(slangTestForcedDownstreamBackend)
{
    // `-emit-cpu-via-llvm` forces the slang-llvm backend regardless of the target, including for
    // `host-callable`, whose target alone would imply only a generic C/C++ compiler. This is the
    // case that made `coverage-llvm-skip.slang` hard-fail (instead of being ignored) on a runner
    // without slang-llvm.
    {
        List<String> args;
        args.add("-target");
        args.add("host-callable");
        args.add("-emit-cpu-via-llvm");
        args.add("-entry");
        args.add("main");

        SLANG_CHECK(SlangTest::getForcedDownstreamBackend(args) == SLANG_PASS_THROUGH_LLVM);
    }

    // The same holds for an llvm-ir target that always routes through slang-llvm.
    {
        List<String> args;
        args.add("-target");
        args.add("llvm-host-ir");
        args.add("-emit-cpu-via-llvm");

        SLANG_CHECK(SlangTest::getForcedDownstreamBackend(args) == SLANG_PASS_THROUGH_LLVM);
    }

    // Without the flag, no downstream backend is forced even for a host-callable target: the
    // target-based requirement (a generic C/C++ compiler) is captured separately.
    {
        List<String> args;
        args.add("-target");
        args.add("host-callable");
        args.add("-entry");
        args.add("main");

        SLANG_CHECK(SlangTest::getForcedDownstreamBackend(args) == SLANG_PASS_THROUGH_NONE);
    }

    // A typical front-end diagnostic command line forces nothing, so such tests keep running on
    // every runner regardless of which target backends are present.
    {
        List<String> args;
        args.add("-target");
        args.add("spirv");

        SLANG_CHECK(SlangTest::getForcedDownstreamBackend(args) == SLANG_PASS_THROUGH_NONE);
    }

    // An empty command line forces nothing.
    {
        List<String> args;

        SLANG_CHECK(SlangTest::getForcedDownstreamBackend(args) == SLANG_PASS_THROUGH_NONE);
    }
}
