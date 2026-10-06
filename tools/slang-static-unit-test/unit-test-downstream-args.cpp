// unit-test-downstream-args.cpp
//
// Tests for CompilerOptionSet::getDownstreamArgs, the accessor that gathers the arguments forwarded
// to a downstream compiler (nvrtc, dxc, ...).

#include "slang/slang-compiler-options.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// Multiple `DownstreamArgs` entries for the same tool are concatenated in insertion order.
SLANG_UNIT_TEST(downstreamArgsForSameToolAreConcatenated)
{
    CompilerOptionSet options;

    CompilerOptionValue first;
    first.kind = CompilerOptionValueKind::String;
    first.stringValue = "nvrtc";
    first.stringValue2 = "--first";
    options.add(CompilerOptionName::DownstreamArgs, first);

    CompilerOptionValue second;
    second.kind = CompilerOptionValueKind::String;
    second.stringValue = "nvrtc";
    second.stringValue2 = "--second";
    options.add(CompilerOptionName::DownstreamArgs, second);

    List<String> args = options.getDownstreamArgs(String("nvrtc"));

    SLANG_CHECK_ABORT(args.getCount() == 2);
    SLANG_CHECK(args[0] == "--first");
    SLANG_CHECK(args[1] == "--second");
}
