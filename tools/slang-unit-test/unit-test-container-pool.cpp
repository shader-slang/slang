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

static void _fillPointerDictionary(Dictionary<int*, int*>* dict, List<int>& values, Index count)
{
    values.setCount(count);
    for (Index i = 0; i < count; i++)
    {
        values[i] = int(i);
        dict->add(&values[i], &values[i]);
    }
}

static size_t _growAndReturnLargeHashSet(ContainerPool& pool, List<int>& values)
{
    auto set = pool.getHashSet<int>();
    _fillPointerSet(set, values, Index(kContainerPoolMinRetireBucketCount));

    auto bucketCount = set->getBucketCount();
    SLANG_CHECK(bucketCount >= kContainerPoolMinRetireBucketCount);
    pool.free(set);
    return bucketCount;
}

static size_t _growAndReturnLargeDictionary(ContainerPool& pool, List<int>& values)
{
    auto dict = pool.getDictionary<int, int>();
    _fillPointerDictionary(dict, values, Index(kContainerPoolMinRetireBucketCount));

    auto bucketCount = dict->getBucketCount();
    SLANG_CHECK(bucketCount >= kContainerPoolMinRetireBucketCount);
    pool.free(dict);
    return bucketCount;
}

SLANG_UNIT_TEST(containerPoolHashSetClearAndDeallocate)
{
    HashSet<int> set;
    for (Index i = 0; i < Index(kContainerPoolMinRetireBucketCount); i++)
        set.add(int(i));

    auto largeBucketCount = set.getBucketCount();
    SLANG_CHECK(largeBucketCount >= kContainerPoolMinRetireBucketCount);

    set.clear();
    SLANG_CHECK(set.getCount() == 0);
    SLANG_CHECK(set.getBucketCount() == largeBucketCount);

    set.add(0);
    set.clearAndDeallocate();
    SLANG_CHECK(set.getCount() == 0);
    SLANG_CHECK(set.getBucketCount() < largeBucketCount);
    SLANG_CHECK(set.getBucketCount() < kContainerPoolMinRetireBucketCount);
}

SLANG_UNIT_TEST(containerPoolDictionaryClearAndDeallocate)
{
    Dictionary<int, int> dict;
    for (Index i = 0; i < Index(kContainerPoolMinRetireBucketCount); i++)
        dict.add(int(i), int(i));

    auto largeBucketCount = dict.getBucketCount();
    SLANG_CHECK(largeBucketCount >= kContainerPoolMinRetireBucketCount);

    dict.clear();
    SLANG_CHECK(dict.getCount() == 0);
    SLANG_CHECK(dict.getBucketCount() == largeBucketCount);

    dict.add(0, 0);
    dict.clearAndDeallocate();
    SLANG_CHECK(dict.getCount() == 0);
    SLANG_CHECK(dict.getBucketCount() < largeBucketCount);
    SLANG_CHECK(dict.getBucketCount() < kContainerPoolMinRetireBucketCount);
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
    SLANG_CHECK(set->getBucketCount() < kContainerPoolMinRetireBucketCount);
    pool.free(set);
}

SLANG_UNIT_TEST(containerPoolDictionaryHysteresisRetiresAfterSecondUnderuse)
{
    ContainerPool pool;
    List<int> values;

    auto largeBucketCount = _growAndReturnLargeDictionary(pool, values);

    auto dict = pool.getDictionary<int, int>();
    SLANG_CHECK(dict->getBucketCount() == largeBucketCount);
    _fillPointerDictionary(dict, values, 1);
    pool.free(dict);

    dict = pool.getDictionary<int, int>();
    SLANG_CHECK(dict->getBucketCount() == largeBucketCount);
    _fillPointerDictionary(dict, values, 1);
    pool.free(dict);

    dict = pool.getDictionary<int, int>();
    SLANG_CHECK(dict->getBucketCount() < largeBucketCount);
    SLANG_CHECK(dict->getBucketCount() < kContainerPoolMinRetireBucketCount);
    pool.free(dict);
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
    auto substantialUseCount = Index(largeBucketCount / kContainerPoolRetireUnderuseDivisor + 1);
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
    SLANG_CHECK(set->getBucketCount() < kContainerPoolMinRetireBucketCount);
    pool.free(set);
}

SLANG_UNIT_TEST(containerPoolDictionaryHysteresisResetsAfterSubstantialUse)
{
    ContainerPool pool;
    List<int> values;

    auto largeBucketCount = _growAndReturnLargeDictionary(pool, values);

    auto dict = pool.getDictionary<int, int>();
    SLANG_CHECK(dict->getBucketCount() == largeBucketCount);
    _fillPointerDictionary(dict, values, 1);
    pool.free(dict);

    dict = pool.getDictionary<int, int>();
    SLANG_CHECK(dict->getBucketCount() == largeBucketCount);
    auto substantialUseCount = Index(largeBucketCount / kContainerPoolRetireUnderuseDivisor + 1);
    _fillPointerDictionary(dict, values, substantialUseCount);
    pool.free(dict);

    dict = pool.getDictionary<int, int>();
    SLANG_CHECK(dict->getBucketCount() == largeBucketCount);
    _fillPointerDictionary(dict, values, 1);
    pool.free(dict);

    dict = pool.getDictionary<int, int>();
    SLANG_CHECK(dict->getBucketCount() == largeBucketCount);
    _fillPointerDictionary(dict, values, 1);
    pool.free(dict);

    dict = pool.getDictionary<int, int>();
    SLANG_CHECK(dict->getBucketCount() < largeBucketCount);
    SLANG_CHECK(dict->getBucketCount() < kContainerPoolMinRetireBucketCount);
    pool.free(dict);
}
