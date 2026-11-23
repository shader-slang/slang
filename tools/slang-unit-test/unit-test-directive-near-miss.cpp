// unit-test-directive-near-miss.cpp

#include "slang-test/test-directive-util.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// TestDirectiveUtil decides what slang-test reports for a mistyped directive comment: which
// unrecognized commands are near misses of a directive the harness recognizes, and which
// spelling to suggest once one is found. It guards a boundary that is easy to get wrong and
// impossible to notice when it slips: the near-miss table must follow the dispatch in
// `_gatherTestsForFile` (an entry the scanner does not recognize would flag legitimate lines;
// a missing one would silently skip its typos), and the suggestion must never name a spelling
// the harness silently ignores.
//
// The paths it guards are not otherwise testable here. A .slang regression test cannot
// exercise slang-test's own directive parsing — a file with a near-miss directive would fail
// the suite permanently — so the rule is pinned directly, the same way
// unit-test-subtest-matching.cpp pins TestToolUtil's selection rules.

SLANG_UNIT_TEST(directiveNearMissLookup, NoRenderApi)
{
    // Typos in the `DISABLE_` prefix itself do not strip, so they land within one insertion
    // of the prefixed entries.
    SLANG_CHECK(
        TestDirectiveUtil::findNearMissDirectiveCommand(toSlice("DISABLED_TEST")) ==
        toSlice("DISABLE_TEST"));
    SLANG_CHECK(
        TestDirectiveUtil::findNearMissDirectiveCommand(toSlice("DISABLED_DIAGNOSTIC_TEST")) ==
        toSlice("DISABLE_DIAGNOSTIC_TEST"));
    SLANG_CHECK(
        TestDirectiveUtil::findNearMissDirectiveCommand(toSlice("DISABLED_TEST_CATEGORY")) ==
        toSlice("DISABLE_TEST_CATEGORY"));

    // Typos after the scanner's `DISABLE_` strip match the plain entries.
    SLANG_CHECK(
        TestDirectiveUtil::findNearMissDirectiveCommand(toSlice("TESTD")) == toSlice("TEST"));
    SLANG_CHECK(
        TestDirectiveUtil::findNearMissDirectiveCommand(toSlice("dTEST")) == toSlice("TEST"));
    SLANG_CHECK(
        TestDirectiveUtil::findNearMissDirectiveCommand(toSlice("TST")) == toSlice("TEST"));
    SLANG_CHECK(
        TestDirectiveUtil::findNearMissDirectiveCommand(toSlice("DIAGNOSTIC_TSET")) ==
        toSlice("DIAGNOSTIC_TEST"));
    SLANG_CHECK(
        TestDirectiveUtil::findNearMissDirectiveCommand(toSlice("TEST_IGNORE_FIL")) ==
        toSlice("TEST_IGNORE_FILE"));

    // Words of the file's shared comment namespace that stay silent: filecheck labels,
    // render-test's input layout, and a diagnostic test's free-form annotation label.
    SLANG_CHECK(
        TestDirectiveUtil::findNearMissDirectiveCommand(toSlice("CHECK")).getLength() == 0);
    SLANG_CHECK(
        TestDirectiveUtil::findNearMissDirectiveCommand(toSlice("TEST_INPUT")).getLength() == 0);
    SLANG_CHECK(
        TestDirectiveUtil::findNearMissDirectiveCommand(toSlice("NEST")).getLength() == 0);
    SLANG_CHECK(
        TestDirectiveUtil::findNearMissDirectiveCommand(toSlice("TEST_DISABLED")).getLength() ==
        0);
    SLANG_CHECK(
        TestDirectiveUtil::findNearMissDirectiveCommand(toSlice("Test")).getLength() == 0);

    // The suggestion re-attaches the stripped `DISABLE_` prefix, except for TEST_IGNORE_FILE,
    // whose prefixed form the scanner compares before the strip and so never dispatches on.
    SLANG_CHECK(
        TestDirectiveUtil::getNearMissDirectiveSuggestion(toSlice("TEST"), true) ==
        "DISABLE_TEST");
    SLANG_CHECK(
        TestDirectiveUtil::getNearMissDirectiveSuggestion(toSlice("DIAGNOSTIC_TEST"), true) ==
        "DISABLE_DIAGNOSTIC_TEST");
    SLANG_CHECK(
        TestDirectiveUtil::getNearMissDirectiveSuggestion(toSlice("TEST_CATEGORY"), true) ==
        "DISABLE_TEST_CATEGORY");
    SLANG_CHECK(
        TestDirectiveUtil::getNearMissDirectiveSuggestion(toSlice("TEST_IGNORE_FILE"), true) ==
        "TEST_IGNORE_FILE");
    SLANG_CHECK(
        TestDirectiveUtil::getNearMissDirectiveSuggestion(toSlice("TEST"), false) == "TEST");
    SLANG_CHECK(
        TestDirectiveUtil::getNearMissDirectiveSuggestion(toSlice("DISABLE_TEST"), false) ==
        "DISABLE_TEST");
}
