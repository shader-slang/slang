// unit-test-short-dictionary.cpp

#include "core/slang-basic.h"
#include "core/slang-short-dictionary.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

SLANG_UNIT_TEST(shortDictionary)
{
    // Stays within the inline capacity: every entry should be findable, and a lookup for a key
    // that was never added should miss.
    {
        ShortDictionary<int, String, 4> dict;
        dict.add(1, "one");
        dict.add(2, "two");
        dict.add(3, "three");

        SLANG_CHECK(dict.tryGetValue(1) && *dict.tryGetValue(1) == "one");
        SLANG_CHECK(dict.tryGetValue(2) && *dict.tryGetValue(2) == "two");
        SLANG_CHECK(dict.tryGetValue(3) && *dict.tryGetValue(3) == "three");
        SLANG_CHECK(dict.tryGetValue(4) == nullptr);
    }

    // Crosses the inline capacity: entries added before, at, and after the promotion boundary
    // must all remain findable, with none lost in the move to the overflow Dictionary.
    {
        ShortDictionary<int, int, 4> dict;
        for (int i = 0; i < 10; i++)
            dict.add(i, i * i);

        for (int i = 0; i < 10; i++)
        {
            auto found = dict.tryGetValue(i);
            SLANG_CHECK(found != nullptr);
            if (found)
                SLANG_CHECK(*found == i * i);
        }
        SLANG_CHECK(dict.tryGetValue(10) == nullptr);
    }

    // Exactly at the inline capacity: the last inline slot and the first overflow slot must both
    // work, since that boundary is where an off-by-one in the promotion logic would show up.
    {
        ShortDictionary<int, int, 2> dict;
        dict.add(0, 100);
        dict.add(1, 101);
        SLANG_CHECK(dict.tryGetValue(0) && *dict.tryGetValue(0) == 100);
        SLANG_CHECK(dict.tryGetValue(1) && *dict.tryGetValue(1) == 101);

        dict.add(2, 102);
        SLANG_CHECK(dict.tryGetValue(0) && *dict.tryGetValue(0) == 100);
        SLANG_CHECK(dict.tryGetValue(1) && *dict.tryGetValue(1) == 101);
        SLANG_CHECK(dict.tryGetValue(2) && *dict.tryGetValue(2) == 102);
    }

    // A duplicate key must be rejected, matching Dictionary::add's contract, while entries are
    // still inline. SLANG_RELEASE_ASSERT throws by default (no SLANG_ASSERT env var set), so the
    // failure is catchable here instead of crashing the test process; the rejected add must also
    // leave the original value in place, not shadow or overwrite it.
    {
        ShortDictionary<int, int, 4> dict;
        dict.add(1, 100);
        bool threw = false;
        try
        {
            dict.add(1, 200);
        }
        catch (const Exception&)
        {
            threw = true;
        }
        SLANG_CHECK(threw);
        SLANG_CHECK(dict.tryGetValue(1) && *dict.tryGetValue(1) == 100);
    }

    // Same contract after promotion, at the same assert strength. This exercises a different code
    // path (the explicit `m_overflow.tryGetValue` check ahead of `m_overflow.add`, not the inline
    // linear scan) -- without that explicit check, this would instead hit `Dictionary::add`'s own
    // debug-only-strength assert, which is skippable under `SLANG_ASSERT=release-asserts-only`
    // while the inline path's SLANG_RELEASE_ASSERT above is not, making the two paths disagree
    // under that mode.
    {
        ShortDictionary<int, int, 2> dict;
        dict.add(1, 100);
        dict.add(2, 200);
        dict.add(3, 300); // promotes past the inline capacity of 2

        bool threw = false;
        try
        {
            dict.add(1, 999);
        }
        catch (const Exception&)
        {
            threw = true;
        }
        SLANG_CHECK(threw);
        SLANG_CHECK(dict.tryGetValue(1) && *dict.tryGetValue(1) == 100);
    }
}
