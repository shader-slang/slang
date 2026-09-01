// unit-test-small-dictionary.cpp

#include "core/slang-basic.h"
#include "core/slang-small-dictionary.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

SLANG_UNIT_TEST(smallDictionary)
{
    // Stays within the inline capacity: every entry should be findable, and a lookup for a key
    // that was never added should miss.
    {
        SmallDictionary<int, String, 4> dict;
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
        SmallDictionary<int, int, 4> dict;
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
        SmallDictionary<int, int, 2> dict;
        dict.add(0, 100);
        dict.add(1, 101);
        SLANG_CHECK(dict.tryGetValue(0) && *dict.tryGetValue(0) == 100);
        SLANG_CHECK(dict.tryGetValue(1) && *dict.tryGetValue(1) == 101);

        dict.add(2, 102);
        SLANG_CHECK(dict.tryGetValue(0) && *dict.tryGetValue(0) == 100);
        SLANG_CHECK(dict.tryGetValue(1) && *dict.tryGetValue(1) == 101);
        SLANG_CHECK(dict.tryGetValue(2) && *dict.tryGetValue(2) == 102);
    }
}
