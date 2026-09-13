// unit-test-string-slice-lookup.cpp

#include "core/slang-basic.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// Check that `StringSliceHash`/`StringSliceEqual` let a `Dictionary<String, ...>` be probed with an
// `UnownedStringSlice`, and that they agree with the ordinary `String` hash and comparison. The
// agreement is the part worth testing: a transparent hash that disagreed with the stored key's hash
// would not fail to compile, it would silently fail to find entries that are present.
SLANG_UNIT_TEST(stringSliceLookup)
{
    // The two functors must produce the same hash for the same characters however they are spelled,
    // or a lookup by slice would probe a different bucket than the insertion by `String` used.
    {
        const char* texts[] = {"", "a", "identifier", "a rather longer string with spaces in it"};
        for (auto text : texts)
        {
            const String asString(text);
            const UnownedStringSlice asSlice(text, strlen(text));

            SLANG_CHECK(StringSliceHash{}(asString) == StringSliceHash{}(asSlice));
            SLANG_CHECK(StringSliceHash{}(asString) == asString.getHashCode());
            SLANG_CHECK(StringSliceEqual{}(asString, asSlice));
            SLANG_CHECK(StringSliceEqual{}(asSlice, asString));
        }
    }

    // A lookup by slice finds an entry inserted as a `String`, and misses one that is absent.
    {
        Dictionary<String, int, StringSliceHash, StringSliceEqual> dict;
        dict.add(String("one"), 1);
        dict.add(String("two"), 2);

        SLANG_CHECK(dict.tryGetValue(toSlice("one")) && *dict.tryGetValue(toSlice("one")) == 1);
        SLANG_CHECK(dict.tryGetValue(toSlice("two")) && *dict.tryGetValue(toSlice("two")) == 2);
        SLANG_CHECK(dict.tryGetValue(toSlice("three")) == nullptr);

        // A slice that is a prefix of a stored key must not match it: the comparison has to use the
        // slice's length rather than stopping at a terminator.
        SLANG_CHECK(dict.tryGetValue(toSlice("on")) == nullptr);

        // A slice into a larger buffer, with no terminator of its own, is the case this exists for.
        const char* buffer = "onetwothree";
        SLANG_CHECK(*dict.tryGetValue(UnownedStringSlice(buffer, 3)) == 1);
        SLANG_CHECK(*dict.tryGetValue(UnownedStringSlice(buffer + 3, 3)) == 2);
    }
}
