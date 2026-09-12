// unit-test-text-key.cpp

#include "core/slang-dictionary.h"
#include "core/slang-string.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// `Dictionary` lets a map keyed by any of the text key types be probed with
// any of the others, so that a lookup does not have to materialise the key
// type first (see `TextKeyHash` in slang-dictionary.h). That is only correct
// while the three types agree on both hashing and equality, which is an
// invariant spread across three classes and easy to break by accident -- for
// instance by giving one of them a cached hash that is not refreshed, or by
// changing what `getUnownedSlice` returns. These tests pin it down.
SLANG_UNIT_TEST(textKeyHashAgreement)
{
    const char* const cases[] = {
        "",
        "a",
        "hello",
        "_S12MyModule4FooC_a_fairly_long_mangled_name_to_exercise_the_byte_hash",
    };

    for (const char* text : cases)
    {
        const UnownedStringSlice slice(text);
        const String string(slice);
        const ImmutableHashedString hashed(slice);

        // The empty string is worth checking explicitly: some of the byte-hash
        // implementations special-case a zero length.
        SLANG_CHECK(slice.getHashCode() == string.getHashCode());
        SLANG_CHECK(slice.getHashCode() == hashed.getHashCode());

        SLANG_CHECK(TextKeyHash{}(slice) == TextKeyHash{}(string));
        SLANG_CHECK(TextKeyHash{}(slice) == TextKeyHash{}(hashed));

        // Both argument orders, because which one a hash map uses when it
        // compares a probe against a stored key is an unspecified
        // implementation detail of the map.
        SLANG_CHECK(TextKeyEqual{}(slice, string));
        SLANG_CHECK(TextKeyEqual{}(string, slice));
        SLANG_CHECK(TextKeyEqual{}(slice, hashed));
        SLANG_CHECK(TextKeyEqual{}(hashed, slice));
        SLANG_CHECK(TextKeyEqual{}(string, hashed));
        SLANG_CHECK(TextKeyEqual{}(hashed, string));
    }

    const UnownedStringSlice one = UnownedStringSlice::fromLiteral("one");
    const UnownedStringSlice two = UnownedStringSlice::fromLiteral("two");
    SLANG_CHECK(!TextKeyEqual{}(one, String(two)));
    SLANG_CHECK(!TextKeyEqual{}(String(one), two));
}

// A key stored as one text type must be findable through any of the others,
// which is the property the callers actually rely on.
SLANG_UNIT_TEST(textKeyHeterogeneousLookup)
{
    const auto text =
        UnownedStringSlice::fromLiteral("_S12MyModule4FooC_a_fairly_long_mangled_name");
    const auto absent = UnownedStringSlice::fromLiteral("not_present");

    {
        Dictionary<String, int> dictionary;
        dictionary.add(String(text), 7);

        SLANG_CHECK(dictionary.containsKey(text));
        SLANG_CHECK(dictionary.containsKey(String(text)));
        SLANG_CHECK(dictionary.containsKey(ImmutableHashedString(text)));
        SLANG_CHECK(!dictionary.containsKey(absent));

        const int* found = dictionary.tryGetValue(text);
        SLANG_CHECK(found && *found == 7);
    }

    {
        Dictionary<ImmutableHashedString, int> dictionary;
        dictionary.add(ImmutableHashedString(text), 9);

        SLANG_CHECK(dictionary.containsKey(text));
        SLANG_CHECK(dictionary.containsKey(String(text)));
        SLANG_CHECK(!dictionary.containsKey(absent));

        const int* found = dictionary.tryGetValue(String(text));
        SLANG_CHECK(found && *found == 9);
    }

    // Enough entries to force the map to grow at least once, so that stored
    // keys get rehashed and any disagreement between the two hashes shows up.
    {
        Dictionary<String, Index> dictionary;
        const Index count = 512;
        for (Index i = 0; i < count; ++i)
        {
            StringBuilder builder;
            builder << "key_" << i;
            dictionary.add(String(builder.getUnownedSlice()), i);
        }
        for (Index i = 0; i < count; ++i)
        {
            StringBuilder builder;
            builder << "key_" << i;
            const Index* found = dictionary.tryGetValue(builder.getUnownedSlice());
            SLANG_CHECK(found && *found == i);
        }
    }
}
