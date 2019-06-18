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
    UnownedStringSlice text(input);
    while (true)
    {
        UnownedStringSlice line = StringUtil::extractLine(text);
        if (line.begin() == nullptr)
        {
            return _areEqual(lines, checkLines, checkLinesCount);
        }
        lines.add(line);
    }
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
        auto text = UnownedStringSlice::fromLiteral("Hello\n\rWorld!\n");

        UnownedStringSlice remaining(text);
        for (const auto line : LineParser(text))
        {
            UnownedStringSlice extractLine = StringUtil::extractLine(remaining);

            SLANG_CHECK(line == extractLine);

            // Handle hitting the end
            if (line.begin() == nullptr || extractLine.begin() == nullptr)
            {
                SLANG_CHECK(line.begin() == extractLine.begin());
                break;
            }
        }
    }

}

SLANG_UNIT_TEST("String", stringUnitTest);
