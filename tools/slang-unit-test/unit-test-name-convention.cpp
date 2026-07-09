// unit-test-name-convention.cpp
//
// Contract under test
// -------------------
// `NameConventionUtil` decomposes a name into "words" and rebuilds
// it in another convention. Used by Slang's reflection / option-
// printing code to surface enum names in whichever style the
// caller wants. The contract:
//
//   * `inferStyleFromText` infers the *style* (Kabab / Snake /
//     Camel) from the separator chars in a name; no separators →
//     Camel.
//   * `inferConventionFromText` additionally infers Upper vs
//     Lower from the alphabetic case.
//   * `split` decomposes a name into words for a given style.
//     Camel splits at every uppercase boundary; Kabab/Snake split
//     at the separator char.
//   * `convert` is round-trippable: convert(A, slice, B) followed
//     by convert(B, that, A) reproduces the original (modulo case
//     for case-insensitive styles).
//   * Bit-mask helpers (`isUpper`, `isLower`, `getNameStyle`,
//     `makeUpper`, `makeLower`) decode/encode the
//     NameConvention enum's flag layout.

#include "../../source/compiler-core/slang-name-convention-util.h"
#include "../../source/core/slang-string.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

namespace
{

String convertVia(NameStyle from, const char* input, NameConvention to)
{
    StringBuilder out;
    NameConventionUtil::convert(from, UnownedStringSlice(input), to, out);
    return out.produceString();
}

String convertInferred(const char* input, NameConvention to)
{
    StringBuilder out;
    NameConventionUtil::convert(UnownedStringSlice(input), to, out);
    return out.produceString();
}

bool slicesEqual(
    const List<UnownedStringSlice>& slices,
    const char* const* expected,
    Index expectedCount)
{
    if (slices.getCount() != expectedCount)
        return false;
    for (Index i = 0; i < expectedCount; ++i)
    {
        if (slices[i] != UnownedStringSlice(expected[i]))
            return false;
    }
    return true;
}

} // namespace

SLANG_UNIT_TEST(nameConventionInferStyle)
{
    using NCU = NameConventionUtil;
    SLANG_CHECK(NCU::inferStyleFromText(toSlice("foo-bar-baz")) == NameStyle::Kabab);
    SLANG_CHECK(NCU::inferStyleFromText(toSlice("foo_bar_baz")) == NameStyle::Snake);
    SLANG_CHECK(NCU::inferStyleFromText(toSlice("FooBarBaz")) == NameStyle::Camel);
    SLANG_CHECK(NCU::inferStyleFromText(toSlice("fooBarBaz")) == NameStyle::Camel);
    // No separators → assumed Camel.
    SLANG_CHECK(NCU::inferStyleFromText(toSlice("singleword")) == NameStyle::Camel);
}

SLANG_UNIT_TEST(nameConventionInferConvention)
{
    using NCU = NameConventionUtil;
    SLANG_CHECK(NCU::inferConventionFromText(toSlice("foo-bar")) == NameConvention::LowerKabab);
    SLANG_CHECK(NCU::inferConventionFromText(toSlice("FOO-BAR")) == NameConvention::UpperKabab);
    SLANG_CHECK(NCU::inferConventionFromText(toSlice("foo_bar")) == NameConvention::LowerSnake);
    SLANG_CHECK(NCU::inferConventionFromText(toSlice("FOO_BAR")) == NameConvention::UpperSnake);
    SLANG_CHECK(NCU::inferConventionFromText(toSlice("fooBar")) == NameConvention::LowerCamel);
    SLANG_CHECK(NCU::inferConventionFromText(toSlice("FooBar")) == NameConvention::UpperCamel);
}

SLANG_UNIT_TEST(nameConventionSplit)
{
    using NCU = NameConventionUtil;
    List<UnownedStringSlice> slices;

    slices.clear();
    NCU::split(NameStyle::Kabab, toSlice("foo-bar-baz"), slices);
    const char* expectedKabab[] = {"foo", "bar", "baz"};
    SLANG_CHECK(slicesEqual(slices, expectedKabab, 3));

    slices.clear();
    NCU::split(NameStyle::Snake, toSlice("foo_bar_baz"), slices);
    const char* expectedSnake[] = {"foo", "bar", "baz"};
    SLANG_CHECK(slicesEqual(slices, expectedSnake, 3));

    // Camel splits at every uppercase boundary.
    slices.clear();
    NCU::split(NameStyle::Camel, toSlice("FooBarBaz"), slices);
    const char* expectedCamel[] = {"Foo", "Bar", "Baz"};
    SLANG_CHECK(slicesEqual(slices, expectedCamel, 3));

    slices.clear();
    NCU::split(NameStyle::Camel, toSlice("singleword"), slices);
    SLANG_CHECK(slices.getCount() == 1);
    SLANG_CHECK(slices[0] == toSlice("singleword"));
}

SLANG_UNIT_TEST(nameConventionSplitInferred)
{
    using NCU = NameConventionUtil;
    List<UnownedStringSlice> slices;

    // Style is inferred from the input.
    NCU::split(toSlice("hello-world-now"), slices);
    SLANG_CHECK(slices.getCount() == 3);
    SLANG_CHECK(slices[0] == toSlice("hello"));
    SLANG_CHECK(slices[2] == toSlice("now"));
}

SLANG_UNIT_TEST(nameConventionConvertRoundTrips)
{
    // Each pair should round-trip through every style.
    struct Case
    {
        NameStyle from;
        const char* input;
        NameConvention toCamelLower;
        const char* expectedCamelLower;
        NameConvention toSnakeLower;
        const char* expectedSnakeLower;
        NameConvention toKababLower;
        const char* expectedKababLower;
    };

    Case cases[] = {
        {NameStyle::Kabab,
         "foo-bar-baz",
         NameConvention::LowerCamel,
         "fooBarBaz",
         NameConvention::LowerSnake,
         "foo_bar_baz",
         NameConvention::LowerKabab,
         "foo-bar-baz"},
        {NameStyle::Snake,
         "foo_bar_baz",
         NameConvention::LowerCamel,
         "fooBarBaz",
         NameConvention::LowerSnake,
         "foo_bar_baz",
         NameConvention::LowerKabab,
         "foo-bar-baz"},
        {NameStyle::Camel,
         "FooBarBaz",
         NameConvention::LowerCamel,
         "fooBarBaz",
         NameConvention::LowerSnake,
         "foo_bar_baz",
         NameConvention::LowerKabab,
         "foo-bar-baz"},
    };

    for (const auto& c : cases)
    {
        SLANG_CHECK(convertVia(c.from, c.input, c.toCamelLower) == c.expectedCamelLower);
        SLANG_CHECK(convertVia(c.from, c.input, c.toSnakeLower) == c.expectedSnakeLower);
        SLANG_CHECK(convertVia(c.from, c.input, c.toKababLower) == c.expectedKababLower);
    }
}

SLANG_UNIT_TEST(nameConventionConvertUpperVariants)
{
    SLANG_CHECK(convertVia(NameStyle::Snake, "foo_bar", NameConvention::UpperSnake) == "FOO_BAR");
    SLANG_CHECK(convertVia(NameStyle::Snake, "foo_bar", NameConvention::UpperKabab) == "FOO-BAR");
    SLANG_CHECK(convertVia(NameStyle::Snake, "foo_bar", NameConvention::UpperCamel) == "FooBar");
}

SLANG_UNIT_TEST(nameConventionConvertInferred)
{
    // No `from` style argument: convert() infers it.
    SLANG_CHECK(convertInferred("foo-bar-baz", NameConvention::LowerSnake) == "foo_bar_baz");
    SLANG_CHECK(convertInferred("foo_bar_baz", NameConvention::LowerCamel) == "fooBarBaz");
    SLANG_CHECK(convertInferred("FooBarBaz", NameConvention::LowerKabab) == "foo-bar-baz");
    SLANG_CHECK(convertInferred("FooBarBaz", NameConvention::UpperKabab) == "FOO-BAR-BAZ");
}

SLANG_UNIT_TEST(nameConventionFlags)
{
    SLANG_CHECK(isUpper(NameConvention::UpperKabab));
    SLANG_CHECK(isUpper(NameConvention::UpperSnake));
    SLANG_CHECK(isUpper(NameConvention::UpperCamel));
    SLANG_CHECK(isLower(NameConvention::LowerKabab));
    SLANG_CHECK(isLower(NameConvention::LowerSnake));
    SLANG_CHECK(isLower(NameConvention::LowerCamel));

    SLANG_CHECK(getNameStyle(NameConvention::UpperSnake) == NameStyle::Snake);
    SLANG_CHECK(getNameStyle(NameConvention::LowerCamel) == NameStyle::Camel);
    SLANG_CHECK(getNameStyle(NameConvention::UpperKabab) == NameStyle::Kabab);

    SLANG_CHECK(makeUpper(NameStyle::Snake) == NameConvention::UpperSnake);
    SLANG_CHECK(makeLower(NameStyle::Kabab) == NameConvention::LowerKabab);
}

SLANG_UNIT_TEST(nameConventionJoinWithJoinChar)
{
    UnownedStringSlice slices[] = {
        toSlice("foo"),
        toSlice("bar"),
        toSlice("baz"),
    };
    StringBuilder out;
    // join with explicit separator, lowering camel-cased inputs.
    NameConventionUtil::join(slices, 3, NameConvention::LowerSnake, '_', out);
    SLANG_CHECK(out.produceString() == "foo_bar_baz");
}
