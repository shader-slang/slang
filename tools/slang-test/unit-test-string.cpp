// unit-test-path.cpp

#include "../../source/core/slang-string-util.h"

#include "test-context.h"

using namespace Slang;

static bool _areEqual(const List<UnownedStringSlice>& lines, const UnownedStringSlice* checkLines, Int checkLinesCount)
{
    if (checkLinesCount != lines.getCount())
    {
        return false;
    }

    for (Int i = 0; i < checkLinesCount; ++i)
    {
        if (lines[i] != checkLines[i])
        {
            return false;
        }
    }
    return true;
}

static bool _checkLines(const UnownedStringSlice& input, const UnownedStringSlice* checkLines, Int checkLinesCount)
{
    List<UnownedStringSlice> lines;
    StringUtil::calcLines(input, lines);
    return _areEqual(lines, checkLines, checkLinesCount);
}

static bool _checkLineParser(const UnownedStringSlice& input)
{
    UnownedStringSlice remaining(input), line;
    for (const auto parserLine : LineParser(input))
    {
        if (!StringUtil::extractLine(remaining, line) || line != parserLine)
        {
            return false;
        }
    }
    return StringUtil::extractLine(remaining, line) == false;
}

static void stringUnitTest()
{
    {
        UnownedStringSlice checkLines[] = { UnownedStringSlice::fromLiteral("") };
        SLANG_CHECK(_checkLines(UnownedStringSlice::fromLiteral(""), checkLines, SLANG_COUNT_OF(checkLines)));
    }
    {
        // Will emit no lines
        SLANG_CHECK(_checkLines(UnownedStringSlice(nullptr, nullptr), nullptr, 0));
    }
    {
        // Two lines - both empty
        UnownedStringSlice checkLines[] = { UnownedStringSlice(), UnownedStringSlice()};
        SLANG_CHECK(_checkLines(UnownedStringSlice::fromLiteral("\n"), checkLines, SLANG_COUNT_OF(checkLines)));
    }
    {
        UnownedStringSlice checkLines[] = { UnownedStringSlice::fromLiteral("Hello"), UnownedStringSlice::fromLiteral("World!") };
        SLANG_CHECK(_checkLines(UnownedStringSlice::fromLiteral("Hello\nWorld!"), checkLines, SLANG_COUNT_OF(checkLines)));
    }
    {
        UnownedStringSlice checkLines[] = { UnownedStringSlice::fromLiteral("Hello"), UnownedStringSlice::fromLiteral("World!"), UnownedStringSlice() };
        SLANG_CHECK(_checkLines(UnownedStringSlice::fromLiteral("Hello\n\rWorld!\n"), checkLines, SLANG_COUNT_OF(checkLines)));
    }

    {
        SLANG_CHECK(_checkLineParser(UnownedStringSlice::fromLiteral("Hello\n\rWorld!\n")));
        SLANG_CHECK(_checkLineParser(UnownedStringSlice::fromLiteral("\n")));
        SLANG_CHECK(_checkLineParser(UnownedStringSlice::fromLiteral("")));
    }
    {
        Int value;
        SLANG_CHECK(SLANG_SUCCEEDED(StringUtil::parseInt(UnownedStringSlice("-10"), value)) && value == -10);
        SLANG_CHECK(SLANG_SUCCEEDED(StringUtil::parseInt(UnownedStringSlice("0"), value)) && value == 0);
        SLANG_CHECK(SLANG_SUCCEEDED(StringUtil::parseInt(UnownedStringSlice("-0"), value)) && value == 0);

        SLANG_CHECK(SLANG_SUCCEEDED(StringUtil::parseInt(UnownedStringSlice("13824"), value)) && value == 13824);
        SLANG_CHECK(SLANG_SUCCEEDED(StringUtil::parseInt(UnownedStringSlice("-13824"), value)) && value == -13824);
    }

    {
        UnownedStringSlice values[] = { UnownedStringSlice("hello"), UnownedStringSlice("world"), UnownedStringSlice("!") };
        ArrayView<UnownedStringSlice> valuesView(values, SLANG_COUNT_OF(values));

        List<UnownedStringSlice> checkValues;
        StringBuilder builder;

        {
            builder.Clear();
            StringUtil::join(values, 0, ',', builder);
            SLANG_CHECK(builder == "");
        }

        {
            builder.Clear();
            StringUtil::join(values, 1, ',', builder);
            SLANG_CHECK(builder == "hello");

            StringUtil::split(builder.getUnownedSlice(), ',', checkValues);
            SLANG_CHECK(checkValues.getArrayView() == ArrayView<UnownedStringSlice>(values, 1));
        }

        {
            builder.Clear();
            StringUtil::join(values, 2, ',', builder);
            SLANG_CHECK(builder == "hello,world");

            StringUtil::split(builder.getUnownedSlice(), ',', checkValues);
            SLANG_CHECK(checkValues.getArrayView() == ArrayView<UnownedStringSlice>(values, 2));
        }

        {
            builder.Clear();
            StringUtil::join(values, 3, UnownedStringSlice("ab"), builder);
            SLANG_CHECK(builder == "helloabworldab!");

            StringUtil::split(builder.getUnownedSlice(), UnownedStringSlice("ab"), checkValues);
            SLANG_CHECK(checkValues.getArrayView() == ArrayView<UnownedStringSlice>(values, 3));
        }
    }
}

SLANG_UNIT_TEST("String", stringUnitTest);
