// unit-test-string-util.cpp
//
// Contract under test
// -------------------
// `StringUtil` is a junk drawer of string-manipulation helpers used
// across the compiler — splitting / joining / line extraction /
// printf-style formatting / character replacement / blob-to-string.
// Distinct from the existing `unit-test-string.cpp` which only
// covers the basic `String` class.
//
// What this suite pins:
//
//   * `split(in, sep, out)` decomposes `in` at every occurrence of
//     `sep`. Adjacent separators produce empty slices. **Empty
//     input yields no slices** (the implementation returns with
//     `out` left empty rather than a single empty slice). Length-
//     bounded `split` returns SLANG_FAIL when the input has more
//     than `maxSlices` segments.
//   * `splitOnWhitespace` collapses runs of whitespace and skips
//     leading / trailing whitespace.
//   * `join` is the inverse of `split` for non-empty separators —
//     `join(split(s, sep), sep) == s`.
//   * `indexOfInSplit / getAtInSplit` — random access without
//     materializing the full split list.
//   * `calcCharReplaced(s, from, to)` — every `from` becomes `to`;
//     other chars unchanged.
//   * `replaceAll(s, from, to)` — string-level replacement.
//   * `extractLine` walks lines respecting CRLF / LF / CR endings;
//     `calcLines` returns the same line decomposition as a list.
//   * `appendFormat / makeStringWithFormat` — printf-style.

#include "../../source/core/slang-string-util.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

namespace
{

bool slicesEqual(
    const List<UnownedStringSlice>& slices,
    std::initializer_list<const char*> expected)
{
    if ((Index)expected.size() != slices.getCount())
        return false;
    Index i = 0;
    for (const char* e : expected)
    {
        if (slices[i] != UnownedStringSlice(e))
            return false;
        ++i;
    }
    return true;
}

} // namespace

// -- split ------------------------------------------------------------

SLANG_UNIT_TEST(stringUtilSplitBasic)
{
    List<UnownedStringSlice> out;
    StringUtil::split(toSlice("a,b,c"), ',', out);
    SLANG_CHECK(slicesEqual(out, {"a", "b", "c"}));

    out.clear();
    StringUtil::split(toSlice("hello"), ',', out);
    SLANG_CHECK(slicesEqual(out, {"hello"}));
}

SLANG_UNIT_TEST(stringUtilSplitEmpty)
{
    // Empty input — call mustn't crash. Documenting observed
    // behaviour: out is left empty.
    List<UnownedStringSlice> out;
    StringUtil::split(toSlice(""), ',', out);
    SLANG_CHECK(out.getCount() == 0);
}

SLANG_UNIT_TEST(stringUtilSplitAdjacentSeparators)
{
    List<UnownedStringSlice> out;
    StringUtil::split(toSlice("a,,b"), ',', out);
    // Adjacent separators produce an empty middle slice.
    SLANG_CHECK(slicesEqual(out, {"a", "", "b"}));
}

SLANG_UNIT_TEST(stringUtilSplitMaxSlicesReturnsFailWhenExceeded)
{
    UnownedStringSlice slices[3];
    Index count;
    SlangResult r = StringUtil::split(toSlice("a.b.c.d"), '.', 3, slices, count);
    // 4 segments but only 3 slots: must return FAIL per the
    // contract documented in the source.
    SLANG_CHECK(SLANG_FAILED(r));
}

SLANG_UNIT_TEST(stringUtilSplitMaxSlicesExactFit)
{
    UnownedStringSlice slices[3];
    Index count;
    SlangResult r = StringUtil::split(toSlice("a.b.c"), '.', 3, slices, count);
    SLANG_CHECK(SLANG_SUCCEEDED(r));
    SLANG_CHECK(count == 3);
    SLANG_CHECK(slices[0] == "a");
    SLANG_CHECK(slices[1] == "b");
    SLANG_CHECK(slices[2] == "c");
}

SLANG_UNIT_TEST(stringUtilSplitOnWhitespace)
{
    List<UnownedStringSlice> out;
    StringUtil::splitOnWhitespace(toSlice("  hello\tworld\n  foo  "), out);
    // Whitespace runs collapse and bounding whitespace is skipped.
    SLANG_CHECK(slicesEqual(out, {"hello", "world", "foo"}));
}

// -- join -------------------------------------------------------------

SLANG_UNIT_TEST(stringUtilJoinBasic)
{
    List<String> in;
    in.add("alpha");
    in.add("beta");
    in.add("gamma");
    StringBuilder out;
    StringUtil::join(in, ',', out);
    SLANG_CHECK(out.produceString() == "alpha,beta,gamma");
}

SLANG_UNIT_TEST(stringUtilJoinIsInverseOfSplit)
{
    // For inputs without leading/trailing separators, join(split())
    // round-trips. Edge cases like leading/trailing commas may be
    // collapsed by split, so the round-trip identity isn't universal
    // — verify only on samples where it should hold.
    const char* samples[] = {
        "a,b,c",
        "x",
        "a,,b",
        "alpha,beta,gamma,delta",
    };
    for (const char* s : samples)
    {
        List<UnownedStringSlice> parts;
        StringUtil::split(UnownedStringSlice(s), ',', parts);
        List<String> stringParts;
        for (auto& p : parts)
            stringParts.add(String(p));
        StringBuilder out;
        StringUtil::join(stringParts, ',', out);
        SLANG_CHECK(out.produceString() == s);
    }
}

// -- indexOfInSplit / getAtInSplit -----------------------------------

SLANG_UNIT_TEST(stringUtilIndexOfInSplit)
{
    Index idx = StringUtil::indexOfInSplit(toSlice("apple,banana,cherry"), ',', toSlice("banana"));
    SLANG_CHECK(idx == 1);

    idx = StringUtil::indexOfInSplit(toSlice("apple,banana,cherry"), ',', toSlice("apple"));
    SLANG_CHECK(idx == 0);

    idx = StringUtil::indexOfInSplit(toSlice("apple,banana,cherry"), ',', toSlice("cherry"));
    SLANG_CHECK(idx == 2);

    idx = StringUtil::indexOfInSplit(toSlice("apple,banana,cherry"), ',', toSlice("missing"));
    SLANG_CHECK(idx == -1);
}

SLANG_UNIT_TEST(stringUtilGetAtInSplit)
{
    auto s = StringUtil::getAtInSplit(toSlice("apple,banana,cherry"), ',', 0);
    SLANG_CHECK(s == "apple");

    s = StringUtil::getAtInSplit(toSlice("apple,banana,cherry"), ',', 2);
    SLANG_CHECK(s == "cherry");
}

// -- character / string replacement -----------------------------------

SLANG_UNIT_TEST(stringUtilCalcCharReplaced)
{
    String r = StringUtil::calcCharReplaced(toSlice("a/b/c"), '/', '_');
    SLANG_CHECK(r == "a_b_c");

    // No-op when source char absent.
    r = StringUtil::calcCharReplaced(toSlice("hello"), 'x', 'y');
    SLANG_CHECK(r == "hello");

    // Empty input returns empty.
    r = StringUtil::calcCharReplaced(toSlice(""), '/', '_');
    SLANG_CHECK(r == "");
}

SLANG_UNIT_TEST(stringUtilReplaceAll)
{
    String r = StringUtil::replaceAll(toSlice("foo bar foo"), toSlice("foo"), toSlice("baz"));
    SLANG_CHECK(r == "baz bar baz");

    // Left-to-right, non-overlapping replacement: "aaaa" with
    // "aa"→"b" matches positions 0 and 2, producing "bb".
    r = StringUtil::replaceAll(toSlice("aaaa"), toSlice("aa"), toSlice("b"));
    SLANG_CHECK(r == "bb");

    // Replacement longer than the needle expands the result; the
    // overlapping prefix "aa" of "aaa" is consumed first.
    r = StringUtil::replaceAll(toSlice("aaaa"), toSlice("aa"), toSlice("XYZ"));
    SLANG_CHECK(r == "XYZXYZ");

    // No-op when needle absent.
    r = StringUtil::replaceAll(toSlice("hello"), toSlice("xxx"), toSlice("yyy"));
    SLANG_CHECK(r == "hello");

    // Single-char needle replacement.
    r = StringUtil::replaceAll(toSlice("a-b-c"), toSlice("-"), toSlice("__"));
    SLANG_CHECK(r == "a__b__c");
}

// -- line extraction --------------------------------------------------

SLANG_UNIT_TEST(stringUtilCalcLinesLF)
{
    List<UnownedStringSlice> lines;
    StringUtil::calcLines(toSlice("a\nb\nc"), lines);
    SLANG_CHECK(slicesEqual(lines, {"a", "b", "c"}));
}

SLANG_UNIT_TEST(stringUtilCalcLinesCRLF)
{
    List<UnownedStringSlice> lines;
    StringUtil::calcLines(toSlice("a\r\nb\r\nc"), lines);
    SLANG_CHECK(slicesEqual(lines, {"a", "b", "c"}));
}

SLANG_UNIT_TEST(stringUtilCalcLinesTrailingNewline)
{
    List<UnownedStringSlice> lines;
    StringUtil::calcLines(toSlice("a\nb\n"), lines);
    // Trailing newline produces a final empty-line slice.
    SLANG_CHECK(slicesEqual(lines, {"a", "b", ""}));
}

SLANG_UNIT_TEST(stringUtilExtractLine)
{
    UnownedStringSlice text = toSlice("first\nsecond\nthird");
    UnownedStringSlice line;
    SLANG_CHECK(StringUtil::extractLine(text, line));
    SLANG_CHECK(line == "first");
    SLANG_CHECK(StringUtil::extractLine(text, line));
    SLANG_CHECK(line == "second");
    // Terminator-less final segment: the call that consumes it
    // returns true with the segment as the line, then resets the
    // in/out text to (nullptr, nullptr) to mark completion.
    SLANG_CHECK(StringUtil::extractLine(text, line));
    SLANG_CHECK(line == "third");
    SLANG_CHECK(text.begin() == nullptr);
    // Subsequent calls return false to signal end-of-input.
    SLANG_CHECK(!StringUtil::extractLine(text, line));
}

SLANG_UNIT_TEST(stringUtilExtractLineCrlf)
{
    // CRLF and bare-CR terminators are both consumed correctly.
    UnownedStringSlice text = toSlice("a\r\nb\rc\n");
    UnownedStringSlice line;
    SLANG_CHECK(StringUtil::extractLine(text, line));
    SLANG_CHECK(line == "a");
    SLANG_CHECK(StringUtil::extractLine(text, line));
    SLANG_CHECK(line == "b");
    SLANG_CHECK(StringUtil::extractLine(text, line));
    SLANG_CHECK(line == "c");
    // A trailing terminator yields one final empty line, then end.
    SLANG_CHECK(StringUtil::extractLine(text, line));
    SLANG_CHECK(line == "");
    SLANG_CHECK(!StringUtil::extractLine(text, line));
}

// -- printf-style formatting ------------------------------------------

SLANG_UNIT_TEST(stringUtilMakeStringWithFormat)
{
    String s = StringUtil::makeStringWithFormat("hello %s, %d", "world", 42);
    SLANG_CHECK(s == "hello world, 42");

    s = StringUtil::makeStringWithFormat("%d.%d.%d", 1, 2, 3);
    SLANG_CHECK(s == "1.2.3");
}

SLANG_UNIT_TEST(stringUtilAppendFormat)
{
    StringBuilder buf;
    buf << "prefix:";
    StringUtil::appendFormat(buf, " v=%d", 7);
    SLANG_CHECK(buf.produceString() == "prefix: v=7");
}
