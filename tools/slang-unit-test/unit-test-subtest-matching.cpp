// unit-test-subtest-matching.cpp

#include "core/slang-test-tool-util.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// TestToolUtil::entryMatchesSubtest decides whether a command-line entry
// (-exclude-prefix, -skip-list, a positional test prefix) selects a particular
// expanded subtest. It exists for a boundary that is easy to get wrong and
// impossible to notice when it is: `foo.slang.6` must not also select
// `foo.slang.60`, and subtest 0 is spelled with no suffix at all.
//
// The paths it guards are not otherwise testable here. A .slang regression
// test cannot exercise slang-test's own selection logic, and the case that
// motivated it — a synthesized LLVM variant crashing under macOS coverage
// instrumentation — needs that platform to reproduce. So the rule is pinned
// directly.

SLANG_UNIT_TEST(subtestIndexParsing)
{
    const String file = "tests/compute/foo.slang";

    SLANG_CHECK(TestToolUtil::getSubtestIndex("tests/compute/foo.slang.6", file) == 6);
    SLANG_CHECK(TestToolUtil::getSubtestIndex("tests/compute/foo.slang.60", file) == 60);
    SLANG_CHECK(TestToolUtil::getSubtestIndex("tests/compute/foo.slang.0", file) == 0);

    // Not subtest selectors: the file itself, a non-numeric suffix, a suffix
    // with no dot, a trailing dot with no digits, and an unrelated path.
    SLANG_CHECK(TestToolUtil::getSubtestIndex("tests/compute/foo.slang", file) == -1);
    SLANG_CHECK(TestToolUtil::getSubtestIndex("tests/compute/foo.slang.x", file) == -1);
    SLANG_CHECK(TestToolUtil::getSubtestIndex("tests/compute/foo.slang6", file) == -1);
    // A bare trailing dot leaves an empty suffix, exercising the `< 2` length guard.
    SLANG_CHECK(TestToolUtil::getSubtestIndex("tests/compute/foo.slang.", file) == -1);
    SLANG_CHECK(TestToolUtil::getSubtestIndex("tests/other", file) == -1);
}

SLANG_UNIT_TEST(subtestEntryMatching)
{
    const String file = "tests/compute/foo.slang";
    const String stem0 = "tests/compute/foo.slang"; // subtest 0 has no suffix
    const String stem6 = "tests/compute/foo.slang.6";
    const String stem60 = "tests/compute/foo.slang.60";

    // THE boundary this predicate exists for.
    SLANG_CHECK(TestToolUtil::entryMatchesSubtest("tests/compute/foo.slang.6", file, stem6, 6));
    SLANG_CHECK(!TestToolUtil::entryMatchesSubtest("tests/compute/foo.slang.6", file, stem60, 60));
    SLANG_CHECK(TestToolUtil::entryMatchesSubtest("tests/compute/foo.slang.60", file, stem60, 60));

    // Subtest 0 is spelled without a suffix in the stem, so it cannot be
    // matched by string equality against the entry.
    SLANG_CHECK(TestToolUtil::entryMatchesSubtest("tests/compute/foo.slang.0", file, stem0, 0));
    SLANG_CHECK(!TestToolUtil::entryMatchesSubtest("tests/compute/foo.slang.0", file, stem6, 6));

    // A plain path prefix still selects every subtest of a matching file,
    // which is the pre-existing behaviour of these flags.
    SLANG_CHECK(TestToolUtil::entryMatchesSubtest("tests/compute", file, stem6, 6));
    SLANG_CHECK(TestToolUtil::entryMatchesSubtest("tests/compute/foo.slang", file, stem6, 6));
    SLANG_CHECK(!TestToolUtil::entryMatchesSubtest("tests/other", file, stem6, 6));

    // A non-numeric suffix is a prefix, not a subtest selector — and as a
    // prefix of this file's path it does not match either.
    SLANG_CHECK(!TestToolUtil::entryMatchesSubtest("tests/compute/foo.slang.x", file, stem6, 6));
}

// TestToolUtil::isSubtestExcluded is the decision the pre-dispatch skip in
// slang-test actually makes: it consults BOTH -exclude-prefix and -skip-list,
// because once a subtest has been expanded the two flags mean the same thing.
// The call site sits mid-way through slang-test's dispatch loop where nothing
// else can reach it, so the disjunction is pinned here instead — otherwise a
// refactor could drop one of the two lists and every committed test would still
// pass, leaving "did the macOS coverage job crash again" as the only signal.

SLANG_UNIT_TEST(subtestExclusionConsultsBothLists)
{
    const String file = "tests/compute/foo.slang";
    const String stem6 = "tests/compute/foo.slang.6";
    const String stem7 = "tests/compute/foo.slang.7";
    const List<String> none;

    List<String> subtest6;
    subtest6.add("tests/compute/foo.slang.6");

    // Named on -exclude-prefix only, on -skip-list only, and on neither.
    SLANG_CHECK(TestToolUtil::isSubtestExcluded(subtest6, none, file, stem6, 6));
    SLANG_CHECK(TestToolUtil::isSubtestExcluded(none, subtest6, file, stem6, 6));
    SLANG_CHECK(!TestToolUtil::isSubtestExcluded(none, none, file, stem6, 6));

    // Excluding one subtest leaves its siblings running — the property the
    // whole change exists to provide, versus dropping the file wholesale.
    SLANG_CHECK(!TestToolUtil::isSubtestExcluded(subtest6, none, file, stem7, 7));
    SLANG_CHECK(!TestToolUtil::isSubtestExcluded(none, subtest6, file, stem7, 7));

    // A plain path entry still removes every subtest of that file.
    List<String> wholeFile;
    wholeFile.add("tests/compute/foo.slang");
    SLANG_CHECK(TestToolUtil::isSubtestExcluded(wholeFile, none, file, stem6, 6));
    SLANG_CHECK(TestToolUtil::isSubtestExcluded(wholeFile, none, file, stem7, 7));
}

SLANG_UNIT_TEST(subtestIndexRejectsOutOfRangeSuffix)
{
    const String file = "tests/compute/foo.slang";

    // The accumulation in getSubtestIndex is signed, so a suffix that would
    // exceed int has to be rejected rather than wrapped. INT_MAX itself still
    // parses; one past it, and an absurdly long run of digits, do not.
    SLANG_CHECK(
        TestToolUtil::getSubtestIndex("tests/compute/foo.slang.2147483647", file) == 2147483647);
    SLANG_CHECK(TestToolUtil::getSubtestIndex("tests/compute/foo.slang.2147483648", file) == -1);
    SLANG_CHECK(
        TestToolUtil::getSubtestIndex("tests/compute/foo.slang.99999999999999999999", file) == -1);

    // Rejected means "not a subtest selector", so such an entry falls back to
    // plain-prefix matching rather than silently selecting a wrapped index.
    SLANG_CHECK(!TestToolUtil::entryMatchesSubtest(
        "tests/compute/foo.slang.2147483648",
        file,
        "tests/compute/foo.slang.6",
        6));
}
