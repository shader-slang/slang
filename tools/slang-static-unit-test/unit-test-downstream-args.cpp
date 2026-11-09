// unit-test-downstream-args.cpp
//
// Tests for CompilerOptionSet::getDownstreamArgs, the accessor that gathers the arguments forwarded
// to a downstream compiler (nvrtc, dxc, ...), and for how the option-set merges treat
// `DownstreamArgs`, which each level (session, target, component type) holds only for itself.

#include "slang/slang-compiler-options.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

static void addDownstreamArgs(CompilerOptionSet& options, const char* tool, const char* args)
{
    CompilerOptionValue value;
    value.kind = CompilerOptionValueKind::String;
    value.stringValue = tool;
    value.stringValue2 = args;
    options.add(CompilerOptionName::DownstreamArgs, value);
}

static bool argsEqual(const List<String>& actual, std::initializer_list<const char*> expected)
{
    if (actual.getCount() != Index(expected.size()))
        return false;
    Index i = 0;
    for (auto arg : expected)
    {
        if (actual[i++] != arg)
            return false;
    }
    return true;
}

// Multiple `DownstreamArgs` entries for the same tool are concatenated in insertion order.
SLANG_UNIT_TEST(downstreamArgsForSameToolAreConcatenated)
{
    CompilerOptionSet options;
    addDownstreamArgs(options, "nvrtc", "--first");
    addDownstreamArgs(options, "nvrtc", "--second");

    List<String> args = options.getDownstreamArgs(String("nvrtc"));

    SLANG_CHECK_ABORT(args.getCount() == 2);
    SLANG_CHECK(args[0] == "--first");
    SLANG_CHECK(args[1] == "--second");
}

// `overrideWith` adds the other set's entries after the current ones, without matching or
// dropping any of them, so arguments given one token per entry survive exactly as given.
SLANG_UNIT_TEST(downstreamArgsOverrideWithAppends)
{
    CompilerOptionSet target;
    addDownstreamArgs(target, "nvrtc", "-D");
    addDownstreamArgs(target, "nvrtc", "FOO=1");

    CompilerOptionSet link;
    addDownstreamArgs(link, "nvrtc", "-D");
    addDownstreamArgs(link, "nvrtc", "BAR=2");

    target.overrideWith(link);

    SLANG_CHECK(
        argsEqual(target.getDownstreamArgs(String("nvrtc")), {"-D", "FOO=1", "-D", "BAR=2"}));
}

// Merging a set into itself appends a copy of its own entries, including when the list has to grow.
SLANG_UNIT_TEST(downstreamArgsSelfOverrideWithDoublesTheList)
{
    const Index kEntryCount = 20;
    CompilerOptionSet options;
    for (Index i = 0; i < kEntryCount; i++)
        addDownstreamArgs(options, "nvrtc", String(i).getBuffer());

    options.overrideWith(options);

    List<String> args = options.getDownstreamArgs(String("nvrtc"));
    SLANG_CHECK_ABORT(args.getCount() == 2 * kEntryCount);
    for (Index i = 0; i < kEntryCount; i++)
    {
        SLANG_CHECK(args[i] == String(i));
        SLANG_CHECK(args[kEntryCount + i] == String(i));
    }
}

// `inheritFrom` and `copyWithoutLevelLocalOptions` never carry `DownstreamArgs` from one level to
// another, while other options are still inherited and copied.
SLANG_UNIT_TEST(downstreamArgsAreNotInheritedOrCopied)
{
    CompilerOptionSet session;
    addDownstreamArgs(session, "nvrtc", "--gpu-architecture=compute_86");
    session.add(CompilerOptionName::Include, String("session-include"));

    CompilerOptionSet target;
    addDownstreamArgs(target, "nvrtc", "--fmad=false");
    target.inheritFrom(session);
    SLANG_CHECK(argsEqual(target.getDownstreamArgs(String("nvrtc")), {"--fmad=false"}));
    SLANG_CHECK(target.getArray(CompilerOptionName::Include).getCount() == 1);

    CompilerOptionSet empty;
    empty.inheritFrom(session);
    SLANG_CHECK(!empty.hasOption(CompilerOptionName::DownstreamArgs));

    CompilerOptionSet copy = session.copyWithoutLevelLocalOptions();
    SLANG_CHECK(!copy.hasOption(CompilerOptionName::DownstreamArgs));
    SLANG_CHECK(copy.getArray(CompilerOptionName::Include).getCount() == 1);
}
