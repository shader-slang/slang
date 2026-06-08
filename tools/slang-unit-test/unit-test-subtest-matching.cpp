// unit-test-subtest-matching.cpp

#include "../../source/core/slang-test-tool-util.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// Guards the subtest-name matching that backs slang-test's -exclude-prefix / -skip-list
// subtest-granular skipping (shader-slang/slang#11384). These helpers are file-local in
// slang-test-main.cpp's run loop normally; they live in TestToolUtil so they can be tested
// here without a GPU or a running test session.

SLANG_UNIT_TEST(subtestGetSubtestIndex)
{
    const String file = "tests/compute/parameter-block.slang";

    // A ".<digits>" suffix parses to that index.
    SLANG_CHECK(TestToolUtil::getSubtestIndex("tests/compute/parameter-block.slang.6", file) == 6);
    SLANG_CHECK(TestToolUtil::getSubtestIndex("tests/compute/parameter-block.slang.0", file) == 0);

    // Exact: ".6" must not be confused with ".60".
    SLANG_CHECK(
        TestToolUtil::getSubtestIndex("tests/compute/parameter-block.slang.60", file) == 60);

    // The bare file path is not a subtest stem.
    SLANG_CHECK(TestToolUtil::getSubtestIndex(file, file) == -1);

    // The full expanded display name is NOT a subtest stem: the space after ".6" ends
    // digit parsing. This is the crux of #11384 - the full variant must be matched by
    // exact name, not via getSubtestIndex.
    SLANG_CHECK(
        TestToolUtil::getSubtestIndex("tests/compute/parameter-block.slang.6 syn (llvm)", file) ==
        -1);

    // A different file path does not match.
    SLANG_CHECK(TestToolUtil::getSubtestIndex("tests/other.slang.1", file) == -1);

    // An implausibly long digit run is rejected (guards 32-bit signed overflow),
    // rather than wrapping to a bogus index.
    SLANG_CHECK(
        TestToolUtil::getSubtestIndex("tests/compute/parameter-block.slang.99999999999", file) ==
        -1);
}

SLANG_UNIT_TEST(subtestInsertSubtestIndex)
{
    // ".<index>" is inserted after the path token, before any " syn"/" (api)" suffix.
    SLANG_CHECK(TestToolUtil::insertSubtestIndex("foo.slang (vk)", 0) == "foo.slang.0 (vk)");
    SLANG_CHECK(
        TestToolUtil::insertSubtestIndex("foo.slang syn (llvm)", 0) == "foo.slang.0 syn (llvm)");
    // No suffix -> appended at the end.
    SLANG_CHECK(TestToolUtil::insertSubtestIndex("foo.slang", 3) == "foo.slang.3");
}

SLANG_UNIT_TEST(subtestMatchExcludeEntry)
{
    const String file = "tests/compute/parameter-block.slang";

    // --- A synthesized subtest at index 6 (the #11384 target) ---
    // testName = "<file>.6 syn (llvm)", outputStem = "<file>.6", 7 subtests total.
    const String stem6 = "tests/compute/parameter-block.slang.6";
    const String name6 = "tests/compute/parameter-block.slang.6 syn (llvm)";

    auto match6 = [&](const String& entry)
    { return TestToolUtil::doesSubtestMatchExcludeEntry(entry, file, stem6, name6, 6, 7); };

    // Full expanded name -> matches just this variant.
    SLANG_CHECK(match6("tests/compute/parameter-block.slang.6 syn (llvm)"));
    // Subtest stem -> matches all variants of index 6.
    SLANG_CHECK(match6("tests/compute/parameter-block.slang.6"));
    // Precision: ".60" must not match index 6.
    SLANG_CHECK(!match6("tests/compute/parameter-block.slang.60"));
    // A different subtest's full name does not match.
    SLANG_CHECK(!match6("tests/compute/parameter-block.slang.5 syn (llvm)"));
    // A ".0" stem must not match subtest 6 (exercises the entrySubtest==0 branch's
    // outputStem==filePath guard: subtest 6's outputStem is "<file>.6", not "<file>").
    SLANG_CHECK(!match6("tests/compute/parameter-block.slang.0"));
    // The bare file path is handled pre-expansion in shouldRunTest, not here.
    SLANG_CHECK(!match6(file));

    // --- A non-synthesized render variant at index 3 (README correctness: the (api)
    //     suffix is not synthesized-only) ---
    const String stem3 = "tests/compute/parameter-block.slang.3";
    const String name3 = "tests/compute/parameter-block.slang.3 (vk)";
    SLANG_CHECK(TestToolUtil::doesSubtestMatchExcludeEntry(name3, file, stem3, name3, 3, 7));
    SLANG_CHECK(TestToolUtil::doesSubtestMatchExcludeEntry(stem3, file, stem3, name3, 3, 7));

    // --- The first subtest (index 0) ---
    // testName = "<file> (cpu)" (no ".0"); outputStem == file; 7 subtests total.
    const String name0 = "tests/compute/parameter-block.slang (cpu)";
    auto match0 = [&](const String& entry)
    { return TestToolUtil::doesSubtestMatchExcludeEntry(entry, file, file, name0, 0, 7); };
    // Internal name (no ".0") matches.
    SLANG_CHECK(match0("tests/compute/parameter-block.slang (cpu)"));
    // The ".0" form -dry-run prints for the first subtest also matches.
    SLANG_CHECK(match0("tests/compute/parameter-block.slang.0 (cpu)"));
    // The bare ".0" stem matches all variants of subtest 0.
    SLANG_CHECK(match0("tests/compute/parameter-block.slang.0"));
    // A different API's ".0" form does not match this (cpu) variant.
    SLANG_CHECK(!match0("tests/compute/parameter-block.slang.0 (vk)"));

    // --- A single-subtest file: no ".0" form is printed (count == 1), so only the
    //     bare name / ".0" stem apply ---
    const String soloName = "tests/foo.slang (cpu)";
    SLANG_CHECK(TestToolUtil::doesSubtestMatchExcludeEntry(
        soloName,
        "tests/foo.slang",
        "tests/foo.slang",
        soloName,
        0,
        1));
    SLANG_CHECK(!TestToolUtil::doesSubtestMatchExcludeEntry(
        "tests/foo.slang.0 (cpu)",
        "tests/foo.slang",
        "tests/foo.slang",
        soloName,
        0,
        1));
}
