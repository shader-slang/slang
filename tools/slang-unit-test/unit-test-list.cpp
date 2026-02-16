// unit-test-list.cpp

#include "core/slang-basic.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

namespace
{
template<typename T, typename GetValueFunc>
void checkListIsSortedBy(const List<T>& list, const GetValueFunc& getValue)
{
    for (Index i = 1; i < list.getCount(); i++)
        SLANG_CHECK(getValue(list[i - 1]) <= getValue(list[i]));
}

template<typename T>
void checkListIsSorted(const List<T>& list)
{
    checkListIsSortedBy(list, [](const T& value) { return value; });
}
} // namespace

SLANG_UNIT_TEST(list)
{
    {
        List<int> values = {10, 20, 30, 40};

        SLANG_CHECK(values.binarySearch(10) == 0);
        SLANG_CHECK(values.binarySearch(30) == 2);

        auto insertAndCheck = [&](int queryValue, Index expectedInsertIndex)
        {
            Index searchResult = values.binarySearch(queryValue);
            SLANG_CHECK(searchResult == ~expectedInsertIndex);

            Index insertIndex = ~searchResult;
            values.insert(insertIndex, queryValue);

            checkListIsSorted(values);

            Index foundIndex = values.binarySearch(queryValue);
            SLANG_CHECK(foundIndex >= 0);
            SLANG_CHECK(values[foundIndex] == queryValue);
        };

        insertAndCheck(5, 0);
        insertAndCheck(25, 3);
        insertAndCheck(50, 6);
    }

    {
        struct Entry
        {
            int key;
        };

        List<Entry> entries;
        entries.add(Entry{1});
        entries.add(Entry{4});
        entries.add(Entry{9});

        auto comparer = [](const Entry& entry, int key)
        {
            if (entry.key < key)
                return -1;
            if (entry.key > key)
                return 1;
            return 0;
        };

        auto insertAndCheck = [&](int queryKey, Index expectedInsertIndex)
        {
            Index searchResult = entries.binarySearch(queryKey, comparer);
            SLANG_CHECK(searchResult == ~expectedInsertIndex);

            Index insertIndex = ~searchResult;
            entries.insert(insertIndex, Entry{queryKey});

            checkListIsSortedBy(entries, [](const Entry& entry) { return entry.key; });

            Index foundIndex = entries.binarySearch(queryKey, comparer);
            SLANG_CHECK(foundIndex >= 0);
            SLANG_CHECK(entries[foundIndex].key == queryKey);
        };

        SLANG_CHECK(entries.binarySearch(4, comparer) == 1);

        insertAndCheck(0, 0);
        insertAndCheck(6, 3);
        insertAndCheck(10, 5);
    }

    // Empty list
    {
        List<int> values;
        SLANG_CHECK(values.binarySearch(42) == ~0);
    }

    // Single element
    {
        List<int> values = {10};
        SLANG_CHECK(values.binarySearch(10) >= 0);
        SLANG_CHECK(values.binarySearch(5) == ~0);
        SLANG_CHECK(values.binarySearch(15) == ~1);

        Index searchResult = values.binarySearch(5);
        values.insert(~searchResult, 5);
        SLANG_CHECK(values.getCount() == 2);
        checkListIsSorted(values);
        SLANG_CHECK(values[0] == 5);
        SLANG_CHECK(values[1] == 10);
        SLANG_CHECK(values.binarySearch(5) >= 0);
    }

    // Duplicate values
    {
        List<int> values = {10, 20, 20, 30};

        Index foundIndex = values.binarySearch(20);
        SLANG_CHECK(foundIndex >= 0);
        SLANG_CHECK(values[foundIndex] == 20);

        Index searchResult = values.binarySearch(15);
        SLANG_CHECK(searchResult < 0);
        values.insert(~searchResult, 15);

        checkListIsSorted(values);

        SLANG_CHECK(values.binarySearch(15) >= 0);
    }
}
