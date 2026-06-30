// unit-test-container-pool.cpp

#include "core/slang-list.h"
#include "slang/slang-container-pool.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

static void _fillPointerSet(HashSet<int*>* set, List<int>& values, Index count)
{
    values.setCount(count);
    for (Index i = 0; i < count; i++)
    {
        values[i] = int(i);
        set->add(&values[i]);
    }
}

static size_t _growAndReturnLargeHashSet(ContainerPool& pool, List<int>& values)
{
    auto set = pool.getHashSet<int>();
    _fillPointerSet(set, values, Index(kContainerPoolHashSetMinRetireBucketCount));

    auto bucketCount = set->getBucketCount();
    SLANG_CHECK(bucketCount >= kContainerPoolHashSetMinRetireBucketCount);
    pool.free(set);
    return bucketCount;
}

SLANG_UNIT_TEST(containerPoolHashSetClearAndDeallocate)
{
    HashSet<int> set;
    for (Index i = 0; i < Index(kContainerPoolHashSetMinRetireBucketCount); i++)
        set.add(int(i));

    auto largeBucketCount = set.getBucketCount();
    SLANG_CHECK(largeBucketCount >= kContainerPoolHashSetMinRetireBucketCount);

    set.clear();
    SLANG_CHECK(set.getCount() == 0);
    SLANG_CHECK(set.getBucketCount() == largeBucketCount);

    set.add(0);
    set.clearAndDeallocate();
    SLANG_CHECK(set.getCount() == 0);
    SLANG_CHECK(set.getBucketCount() < largeBucketCount);
    SLANG_CHECK(set.getBucketCount() < kContainerPoolHashSetMinRetireBucketCount);
}

SLANG_UNIT_TEST(containerPoolHashSetHysteresisRetiresAfterSecondUnderuse)
{
    ContainerPool pool;
    List<int> values;

    auto largeBucketCount = _growAndReturnLargeHashSet(pool, values);

    auto set = pool.getHashSet<int>();
    SLANG_CHECK(set->getBucketCount() == largeBucketCount);
    _fillPointerSet(set, values, 1);
    pool.free(set);

    set = pool.getHashSet<int>();
    SLANG_CHECK(set->getBucketCount() == largeBucketCount);
    _fillPointerSet(set, values, 1);
    pool.free(set);

    set = pool.getHashSet<int>();
    SLANG_CHECK(set->getBucketCount() < largeBucketCount);
    SLANG_CHECK(set->getBucketCount() < kContainerPoolHashSetMinRetireBucketCount);
    pool.free(set);
}

SLANG_UNIT_TEST(containerPoolHashSetHysteresisResetsAfterSubstantialUse)
{
    ContainerPool pool;
    List<int> values;

    auto largeBucketCount = _growAndReturnLargeHashSet(pool, values);

    auto set = pool.getHashSet<int>();
    SLANG_CHECK(set->getBucketCount() == largeBucketCount);
    _fillPointerSet(set, values, 1);
    pool.free(set);

    set = pool.getHashSet<int>();
    SLANG_CHECK(set->getBucketCount() == largeBucketCount);
    auto substantialUseCount =
        Index(largeBucketCount / kContainerPoolHashSetRetireUnderuseDivisor + 1);
    _fillPointerSet(set, values, substantialUseCount);
    pool.free(set);

    set = pool.getHashSet<int>();
    SLANG_CHECK(set->getBucketCount() == largeBucketCount);
    _fillPointerSet(set, values, 1);
    pool.free(set);

    set = pool.getHashSet<int>();
    SLANG_CHECK(set->getBucketCount() == largeBucketCount);
    _fillPointerSet(set, values, 1);
    pool.free(set);

    set = pool.getHashSet<int>();
    SLANG_CHECK(set->getBucketCount() < largeBucketCount);
    SLANG_CHECK(set->getBucketCount() < kContainerPoolHashSetMinRetireBucketCount);
    pool.free(set);
}
