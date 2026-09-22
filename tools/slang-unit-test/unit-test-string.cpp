// unit-test-path.cpp

#include "core/slang-string-util.h"
#include "unit-test/slang-unit-test.h"

// #include <math.h>

#include <array>
#include <cstring>
#include <limits>
#include <sstream>

using namespace Slang;

static bool _areEqual(
    const List<UnownedStringSlice>& lines,
    const UnownedStringSlice* checkLines,
    Int checkLinesCount)
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

static bool _checkLines(
    const UnownedStringSlice& input,
    const UnownedStringSlice* checkLines,
    Int checkLinesCount)
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

static void _append(double v, StringBuilder& buf)
{
    std::ostringstream stream;
    stream.imbue(std::locale::classic());
    stream.setf(std::ios::fixed, std::ios::floatfield);
    stream.precision(20);

    stream << std::scientific << v;

    buf << stream.str().c_str();
}

// Unit of least precision
static int64_t _calcULPDistance(double a, double b)
{
    // Save work if the floats are equal.
    // Also handles +0 == -0
    if (a == b)
    {
        return 0;
    }

    const int64_t max = int64_t((~uint64_t(0)) >> 1);

#if 0
    // Max distance for NaN
    if (isnan(a) || isnan(b))
    {
        return max;
    }

    // If one's infinite and they're not equal, max distance.
    if (isinf(a) || isinf(b))
    {
        return max;
    }
#endif

    int64_t ia, ib;
    memcpy(&ia, &a, sizeof(a));
    memcpy(&ib, &b, sizeof(b));

    // Don't compare differently-signed floats.
    if ((ia < 0) != (ib < 0))
    {
        return max;
    }

    // Return the absolute value of the distance in ULPs.
    int64_t distance = ia - ib;
    return distance < 0 ? -distance : distance;
}

static bool _areApproximatelyEqual(
    double a,
    double b,
    double fixedEpsilon = 1e-10,
    int ulpsEpsilon = 100)
{
    // Handle the near-zero case.
    const double difference = abs(a - b);
    if (difference <= fixedEpsilon)
    {
        return true;
    }

    return _calcULPDistance(a, b) <= ulpsEpsilon;
}

SLANG_UNIT_TEST(string)
{
    {
        UnownedStringSlice checkLines[] = {UnownedStringSlice::fromLiteral("")};
        SLANG_CHECK(_checkLines(
            UnownedStringSlice::fromLiteral(""),
            checkLines,
            SLANG_COUNT_OF(checkLines)));
    }
    {
        // Will emit no lines
        SLANG_CHECK(_checkLines(UnownedStringSlice(nullptr, nullptr), nullptr, 0));
    }
    {
        // Two lines - both empty
        UnownedStringSlice checkLines[] = {UnownedStringSlice(), UnownedStringSlice()};
        SLANG_CHECK(_checkLines(
            UnownedStringSlice::fromLiteral("\n"),
            checkLines,
            SLANG_COUNT_OF(checkLines)));
    }
    {
        UnownedStringSlice checkLines[] = {
            UnownedStringSlice::fromLiteral("Hello"),
            UnownedStringSlice::fromLiteral("World!")};
        SLANG_CHECK(_checkLines(
            UnownedStringSlice::fromLiteral("Hello\nWorld!"),
            checkLines,
            SLANG_COUNT_OF(checkLines)));
    }
    {
        UnownedStringSlice checkLines[] = {
            UnownedStringSlice::fromLiteral("Hello"),
            UnownedStringSlice::fromLiteral("World!"),
            UnownedStringSlice()};
        SLANG_CHECK(_checkLines(
            UnownedStringSlice::fromLiteral("Hello\n\rWorld!\n"),
            checkLines,
            SLANG_COUNT_OF(checkLines)));
    }

    {
        SLANG_CHECK(_checkLineParser(UnownedStringSlice::fromLiteral("Hello\n\rWorld!\n")));
        SLANG_CHECK(_checkLineParser(UnownedStringSlice::fromLiteral("\n")));
        SLANG_CHECK(_checkLineParser(UnownedStringSlice::fromLiteral("")));
    }
    {
        Int value;
        SLANG_CHECK(
            SLANG_SUCCEEDED(StringUtil::parseInt(UnownedStringSlice("-10"), value)) &&
            value == -10);
        SLANG_CHECK(
            SLANG_SUCCEEDED(StringUtil::parseInt(UnownedStringSlice("0"), value)) && value == 0);
        SLANG_CHECK(
            SLANG_SUCCEEDED(StringUtil::parseInt(UnownedStringSlice("-0"), value)) && value == 0);

        SLANG_CHECK(
            SLANG_SUCCEEDED(StringUtil::parseInt(UnownedStringSlice("13824"), value)) &&
            value == 13824);
        SLANG_CHECK(
            SLANG_SUCCEEDED(StringUtil::parseInt(UnownedStringSlice("-13824"), value)) &&
            value == -13824);
    }

    {
        UnownedStringSlice values[] = {
            UnownedStringSlice("hello"),
            UnownedStringSlice("world"),
            UnownedStringSlice("!")};
        ArrayView<UnownedStringSlice> valuesView(values, SLANG_COUNT_OF(values));

        List<UnownedStringSlice> checkValues;
        StringBuilder builder;

        {
            builder.clear();
            StringUtil::join(values, 0, ',', builder);
            SLANG_CHECK(builder == "");
        }

        {
            builder.clear();
            StringUtil::join(values, 1, ',', builder);
            SLANG_CHECK(builder == "hello");

            StringUtil::split(builder.getUnownedSlice(), ',', checkValues);
            SLANG_CHECK(checkValues.getArrayView() == ArrayView<UnownedStringSlice>(values, 1));
        }

        {
            builder.clear();
            StringUtil::join(values, 2, ',', builder);
            SLANG_CHECK(builder == "hello,world");

            StringUtil::split(builder.getUnownedSlice(), ',', checkValues);
            SLANG_CHECK(checkValues.getArrayView() == ArrayView<UnownedStringSlice>(values, 2));
        }

        {
            builder.clear();
            StringUtil::join(values, 3, UnownedStringSlice("ab"), builder);
            SLANG_CHECK(builder == "helloabworldab!");

            StringUtil::split(builder.getUnownedSlice(), UnownedStringSlice("ab"), checkValues);
            SLANG_CHECK(checkValues.getArrayView() == ArrayView<UnownedStringSlice>(values, 3));
        }
    }
    {

        List<double> values;
        values.add(0.0);
        values.add(-0.0);

        for (Index i = -300; i < 300; ++i)
        {
            double value = pow(10, i);

            values.add(value);
            values.add(-value);

            values.addRange(value / 3);
            values.addRange(-value / 3);
        }

        StringBuilder buf;

        for (auto value : values)
        {
            buf.clear();
            _append(value, buf);

            UnownedStringSlice slice = buf.getUnownedSlice();

            double parsedValue;
            SlangResult res = StringUtil::parseDouble(slice, parsedValue);

            auto ulpsParsed = _calcULPDistance(value, parsedValue);

            SLANG_CHECK(SLANG_SUCCEEDED(res));

            // Check that they are equal
            SLANG_CHECK(_areApproximatelyEqual(value, parsedValue));
        }
    }

    // number->ascii->number round trip tests
    {
        List<int64_t> values;
        values.add(0);

        for (Index i = 0; i < 63; ++i)
        {
            auto value = int64_t(1) << i;

            values.add(value);
            values.add(-value);
        }
        values.add(std::numeric_limits<int64_t>::min());
        values.add(std::numeric_limits<int64_t>::max());

        StringBuilder buf;

        for (auto value : values)
        {
            buf.clear();
            buf << value;

            int64_t parsedValue{-1};

            UnownedStringSlice slice = buf.getUnownedSlice();
            SlangResult res = StringUtil::parseInt64(slice, parsedValue);

            SLANG_CHECK(SLANG_SUCCEEDED(res));

            // Check that they are equal
            SLANG_CHECK(value == parsedValue);
        }
    }

    // integer->ascii expected value tests
    {
        // basic values
        for (int radix = 2; radix <= 36; ++radix)
        {
            SLANG_CHECK(String(int32_t(0), radix) == "0");
            SLANG_CHECK(String(uint32_t(0), radix) == "0");
            SLANG_CHECK(String(int64_t(0), radix) == "0");
            SLANG_CHECK(String(uint64_t(0), radix) == "0");

            SLANG_CHECK(String(int32_t(1), radix) == "1");
            SLANG_CHECK(String(uint32_t(1), radix) == "1");
            SLANG_CHECK(String(int64_t(1), radix) == "1");
            SLANG_CHECK(String(uint64_t(1), radix) == "1");

            SLANG_CHECK(String(int32_t(-1), radix) == "-1");
            SLANG_CHECK(String(int64_t(-1), radix) == "-1");
        }

        // extremes in the usual radixes
        SLANG_CHECK(
            String(std::numeric_limits<int32_t>::min(), 2) == "-10000000000000000000000000000000");
        SLANG_CHECK(
            String(std::numeric_limits<int32_t>::max(), 2) == "1111111111111111111111111111111");
        SLANG_CHECK(
            String(std::numeric_limits<int64_t>::min(), 2) ==
            "-1000000000000000000000000000000000000000000000000000000000000000");
        SLANG_CHECK(
            String(std::numeric_limits<int64_t>::max(), 2) ==
            "111111111111111111111111111111111111111111111111111111111111111");

        SLANG_CHECK(
            String(std::numeric_limits<uint32_t>::max(), 2) == "11111111111111111111111111111111");
        SLANG_CHECK(
            String(std::numeric_limits<uint64_t>::max(), 2) ==
            "1111111111111111111111111111111111111111111111111111111111111111");

        SLANG_CHECK(String(std::numeric_limits<int32_t>::min(), 8) == "-20000000000");
        SLANG_CHECK(String(std::numeric_limits<int32_t>::max(), 8) == "17777777777");
        SLANG_CHECK(String(std::numeric_limits<int64_t>::min(), 8) == "-1000000000000000000000");
        SLANG_CHECK(String(std::numeric_limits<int64_t>::max(), 8) == "777777777777777777777");

        SLANG_CHECK(String(std::numeric_limits<uint32_t>::max(), 8) == "37777777777");
        SLANG_CHECK(String(std::numeric_limits<uint64_t>::max(), 8) == "1777777777777777777777");

        SLANG_CHECK(String(std::numeric_limits<int32_t>::min(), 10) == "-2147483648");
        SLANG_CHECK(String(std::numeric_limits<int32_t>::max(), 10) == "2147483647");
        SLANG_CHECK(String(std::numeric_limits<int64_t>::min(), 10) == "-9223372036854775808");
        SLANG_CHECK(String(std::numeric_limits<int64_t>::max(), 10) == "9223372036854775807");

        SLANG_CHECK(String(std::numeric_limits<uint32_t>::max(), 10) == "4294967295");
        SLANG_CHECK(String(std::numeric_limits<uint64_t>::max(), 10) == "18446744073709551615");

        SLANG_CHECK(String(std::numeric_limits<int32_t>::min(), 16) == "-80000000");
        SLANG_CHECK(String(std::numeric_limits<int32_t>::max(), 16) == "7FFFFFFF");
        SLANG_CHECK(String(std::numeric_limits<int64_t>::min(), 16) == "-8000000000000000");
        SLANG_CHECK(String(std::numeric_limits<int64_t>::max(), 16) == "7FFFFFFFFFFFFFFF");

        SLANG_CHECK(String(std::numeric_limits<uint32_t>::max(), 16) == "FFFFFFFF");
        SLANG_CHECK(String(std::numeric_limits<uint64_t>::max(), 16) == "FFFFFFFFFFFFFFFF");

        // max radix cases
        SLANG_CHECK(String(std::numeric_limits<int32_t>::min(), 36) == "-ZIK0ZK");
        SLANG_CHECK(String(std::numeric_limits<int32_t>::max(), 36) == "ZIK0ZJ");
        SLANG_CHECK(String(std::numeric_limits<int64_t>::min(), 36) == "-1Y2P0IJ32E8E8");
        SLANG_CHECK(String(std::numeric_limits<int64_t>::max(), 36) == "1Y2P0IJ32E8E7");
        SLANG_CHECK(String(std::numeric_limits<uint32_t>::max(), 36) == "1Z141Z3");
        SLANG_CHECK(String(std::numeric_limits<uint64_t>::max(), 36) == "3W5E11264SGSF");

        // intToAscii: basic pad-to + returned length test
        struct PadToTestCaseInt64
        {
            int64_t number;
            int radix;
            int padTo;
            const char* expected;
        };

        struct PadToTestCaseUint64
        {
            uint64_t number;
            int radix;
            int padTo;
            const char* expected;
        };

        constexpr auto padToI64Cases = std::to_array<PadToTestCaseInt64>({
            // pad-to smaller than length
            {12345, 10, 0, "12345"},
            {12345, 10, 4, "12345"},
            {-12345, 10, 4, "-12345"},

            // pad-to exact length
            {12345, 10, 5, "12345"},
            {-12345, 10, 5, "-12345"},

            // pad-to larger than length
            {12345, 10, 6, "012345"},
            {-12345, 10, 6, "-012345"},
            {12345, 10, 10, "0000012345"},
            {-12345, 10, 10, "-0000012345"},
        });

        for (const auto& c : padToI64Cases)
        {
            char buf[66]{};

            const int len = intToAscii(buf, c.number, c.radix, c.padTo);
            SLANG_CHECK(strcmp(buf, c.expected) == 0);
            SLANG_CHECK(len >= 0);
            SLANG_CHECK(strlen(c.expected) == size_t(len));
        }

        constexpr auto padToU64Cases = std::to_array<PadToTestCaseUint64>({
            // pad-to smaller than length
            {12345, 10, 0, "12345"},
            {12345, 10, 4, "12345"},

            // pad-to exact length
            {12345, 10, 5, "12345"},

            // pad-to larger than length
            {12345, 10, 6, "012345"},
            {12345, 10, 10, "0000012345"},

            // hash print cases
            {uint64_t(0), 16, 16, "0000000000000000"},
            {uint64_t(0x000123456789ABCD), 16, 16, "000123456789ABCD"},
            {uint64_t(0x1234567890ABCDEF), 16, 16, "1234567890ABCDEF"},
            {uint64_t(0xFFFFFFFFFFFFFFFF), 16, 16, "FFFFFFFFFFFFFFFF"},
        });

        for (const auto& c : padToU64Cases)
        {
            char buf[66]{};

            const int len = intToAscii(buf, c.number, c.radix, c.padTo);
            SLANG_CHECK(strcmp(buf, c.expected) == 0);
            SLANG_CHECK(len >= 0);
            SLANG_CHECK(strlen(c.expected) == size_t(len));
        }
    }
}
