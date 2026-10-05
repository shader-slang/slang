// unit-test-downstream-args.cpp
//
// Tests for CompilerOptionSet::getDownstreamArgs, the accessor that gathers the arguments forwarded
// to a downstream compiler (nvrtc, dxc, ...), and for how `DownstreamArgs` entries compose when
// option sets from different levels (session, target, link) are merged.

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

// A higher-priority level merged with `overrideWith` adds its arguments after the lower level's
// arguments for the same tool instead of replacing them.
SLANG_UNIT_TEST(downstreamArgsOverrideWithAppendsHigherLevel)
{
    CompilerOptionSet target;
    addDownstreamArgs(target, "nvrtc", "--gpu-architecture=compute_86");

    CompilerOptionSet link;
    addDownstreamArgs(link, "nvrtc", "--fmad=false");

    target.overrideWith(link);

    SLANG_CHECK(argsEqual(
        target.getDownstreamArgs(String("nvrtc")),
        {"--gpu-architecture=compute_86", "--fmad=false"}));
}

// A lower-priority level merged with `inheritFrom` adds its arguments before the higher level's
// arguments for the same tool instead of being ignored.
SLANG_UNIT_TEST(downstreamArgsInheritFromPrependsLowerLevel)
{
    CompilerOptionSet link;
    addDownstreamArgs(link, "nvrtc", "--fmad=false");

    CompilerOptionSet target;
    addDownstreamArgs(target, "nvrtc", "--gpu-architecture=compute_86");

    link.inheritFrom(target);

    SLANG_CHECK(argsEqual(
        link.getDownstreamArgs(String("nvrtc")),
        {"--gpu-architecture=compute_86", "--fmad=false"}));
}

// The composed order is lower level first regardless of call order: `TargetProgram` applies the
// link-level set with `overrideWith` and then the target-level set with `inheritFrom`, while
// `Linkage::addTarget` starts from the session set and applies the target set with `overrideWith`.
SLANG_UNIT_TEST(downstreamArgsComposeLowerLevelFirstInAnyCallOrder)
{
    CompilerOptionSet session;
    addDownstreamArgs(session, "nvrtc", "--session");

    CompilerOptionSet targetDesc;
    addDownstreamArgs(targetDesc, "nvrtc", "--target");

    CompilerOptionSet link;
    addDownstreamArgs(link, "nvrtc", "--link");

    // `Linkage::addTarget`: the target request starts as a copy of the session set, inherits the
    // session set again, and is then overridden by the target description.
    CompilerOptionSet target = session;
    target.inheritFrom(session);
    target.overrideWith(targetDesc);
    SLANG_CHECK(argsEqual(target.getDownstreamArgs(String("nvrtc")), {"--session", "--target"}));

    // `TargetProgram`: the program's (link-level) set first, then the target request's set.
    CompilerOptionSet program;
    program.overrideWith(link);
    program.inheritFrom(target);
    SLANG_CHECK(argsEqual(
        program.getDownstreamArgs(String("nvrtc")),
        {"--session", "--target", "--link"}));

    // Starting from the higher level and inheriting a lower level that already contains one of its
    // entries gives the same order as appending the higher level to the lower one.
    CompilerOptionSet higherFirst;
    addDownstreamArgs(higherFirst, "nvrtc", "--a");
    CompilerOptionSet lower;
    addDownstreamArgs(lower, "nvrtc", "--a");
    addDownstreamArgs(lower, "nvrtc", "--b");
    higherFirst.inheritFrom(lower);

    CompilerOptionSet lowerFirst = lower;
    CompilerOptionSet higher;
    addDownstreamArgs(higher, "nvrtc", "--a");
    lowerFirst.overrideWith(higher);

    SLANG_CHECK(argsEqual(higherFirst.getDownstreamArgs(String("nvrtc")), {"--a", "--b"}));
    SLANG_CHECK(argsEqual(lowerFirst.getDownstreamArgs(String("nvrtc")), {"--a", "--b"}));
}

// The session set reaches a target several times (the `TargetRequest` copy and later
// `inheritFrom` calls), so merging an entry that is already present keeps a single copy. Equality
// is per entry: an entry is dropped only when both its tool and its whole serialized argument list
// match an entry already kept, so an entry holding `a` and `b` does not absorb a later entry `a`.
SLANG_UNIT_TEST(downstreamArgsExactDuplicateEntriesAreElided)
{
    CompilerOptionSet session;
    addDownstreamArgs(session, "nvrtc", "--fmad=false");

    CompilerOptionSet target = session;
    target.inheritFrom(session);
    target.inheritFrom(session);
    target.overrideWith(session);
    SLANG_CHECK(argsEqual(target.getDownstreamArgs(String("nvrtc")), {"--fmad=false"}));

    CompilerOptionSet sameLevelPair;
    addDownstreamArgs(sameLevelPair, "nvrtc", "--fmad=false");
    addDownstreamArgs(sameLevelPair, "nvrtc", "--fmad=false");
    CompilerOptionSet merged;
    merged.overrideWith(sameLevelPair);
    SLANG_CHECK(argsEqual(merged.getDownstreamArgs(String("nvrtc")), {"--fmad=false"}));

    CompilerOptionSet multiArgSession;
    addDownstreamArgs(multiArgSession, "nvrtc", "-a\n-b");
    CompilerOptionSet singleArgLink;
    addDownstreamArgs(singleArgLink, "nvrtc", "-a");
    multiArgSession.overrideWith(singleArgLink);
    SLANG_CHECK(argsEqual(multiArgSession.getDownstreamArgs(String("nvrtc")), {"-a", "-b", "-a"}));

    // The same arguments for a different tool are a different entry.
    CompilerOptionSet nvrtc;
    addDownstreamArgs(nvrtc, "nvrtc", "-DX=1");
    CompilerOptionSet dxc;
    addDownstreamArgs(dxc, "dxc", "-DX=1");
    nvrtc.overrideWith(dxc);
    SLANG_CHECK(argsEqual(nvrtc.getDownstreamArgs(String("nvrtc")), {"-DX=1"}));
    SLANG_CHECK(argsEqual(nvrtc.getDownstreamArgs(String("dxc")), {"-DX=1"}));
}
