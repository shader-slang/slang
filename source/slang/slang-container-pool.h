#ifndef SLANG_CONTAINER_POOL_H
#define SLANG_CONTAINER_POOL_H

#include "../core/slang-dictionary.h"
#include "../core/slang-list.h"
#include "../core/slang-virtual-object-pool.h"

// A pool to allow reuse of common types of containers to avoid
// frequent resizing and rehashing.

namespace Slang
{
static const int kContainerPoolSize = 1024;
static const size_t kContainerPoolMinRetireBucketCount = 4096;
static const size_t kContainerPoolRetireUnderuseDivisor = 8;
static const int kContainerPoolRetireUnderuseCount = 2;

template<typename T>
struct ObjectPool
{
    ObjectPool(int maxElementCount)
    {
        m_pool.initPool(maxElementCount);
        m_objects.setCount(maxElementCount);
    }

    T* getObject()
    {
        auto id = m_pool.alloc(1);
        if (id == -1)
            SLANG_UNEXPECTED("container pool allocation failure.");
        return &m_objects[id];
    }

    int getObjectIndex(T* object)
    {
        auto id = (int)(object - m_objects.getBuffer());
        SLANG_RELEASE_ASSERT(id >= 0 && id < m_objects.getCount());
        return id;
    }

    void freeObject(T* object)
    {
        auto id = getObjectIndex(object);
        m_pool.free(id, 1);
    }

    VirtualObjectPool m_pool;
    List<T> m_objects;
};

struct ContainerPool
{
    ObjectPool<List<void*>> m_listPool;
    ObjectPool<Dictionary<void*, void*>> m_dictionaryPool;
    ObjectPool<HashSet<void*>> m_hashSetPool;
    List<int> m_dictionaryUnderuseStreaks;
    List<int> m_hashSetUnderuseStreaks;

    ContainerPool()
        : m_listPool(kContainerPoolSize)
        , m_dictionaryPool(kContainerPoolSize)
        , m_hashSetPool(kContainerPoolSize)
    {
        m_dictionaryUnderuseStreaks.setCount(kContainerPoolSize);
        m_hashSetUnderuseStreaks.setCount(kContainerPoolSize);
        for (Index i = 0; i < kContainerPoolSize; i++)
        {
            m_dictionaryUnderuseStreaks[i] = 0;
            m_hashSetUnderuseStreaks[i] = 0;
        }
    }

    template<typename T>
    List<T*>* getList()
    {
        return (List<T*>*)m_listPool.getObject();
    }

    template<typename T, typename U>
    Dictionary<T*, U*>* getDictionary()
    {
        return (Dictionary<T*, U*>*)m_dictionaryPool.getObject();
    }

    template<typename T>
    HashSet<T*>* getHashSet()
    {
        return (HashSet<T*>*)m_hashSetPool.getObject();
    }

    bool updateUnderuseStreakAndShouldRetire(
        List<int>& underuseStreaks,
        int objectIndex,
        size_t liveCount,
        size_t bucketCount)
    {
        // Dictionaries and hash sets normally keep their buckets after `clear()` so the next large
        // use can reuse that storage. On long runs, though, a pool slot can become expensive if one
        // large use is followed by many tiny uses: every return clears the same large table even
        // though the live count stays small. Track that pattern per slot and only release the
        // buckets after it repeats, so an occasional small use after a large pass does not cause
        // allocation churn.
        bool isUnderused = bucketCount >= kContainerPoolMinRetireBucketCount &&
                           liveCount <= bucketCount / kContainerPoolRetireUnderuseDivisor;
        if (!isUnderused)
        {
            underuseStreaks[objectIndex] = 0;
            return false;
        }

        if (underuseStreaks[objectIndex] < kContainerPoolRetireUnderuseCount)
            underuseStreaks[objectIndex]++;

        if (underuseStreaks[objectIndex] < kContainerPoolRetireUnderuseCount)
            return false;

        underuseStreaks[objectIndex] = 0;
        return true;
    }

    template<typename T>
    void free(List<T*>* list)
    {
        list->clear();
        m_listPool.freeObject((List<void*>*)list);
    }

    template<typename T, typename U>
    void free(Dictionary<T*, U*>* dict)
    {
        auto pooledDict = (Dictionary<void*, void*>*)dict;
        auto objectIndex = m_dictionaryPool.getObjectIndex(pooledDict);
        auto liveCount = dict->getCount();
        auto bucketCount = dict->getBucketCount();
        bool shouldRetire = updateUnderuseStreakAndShouldRetire(
            m_dictionaryUnderuseStreaks,
            objectIndex,
            liveCount,
            bucketCount);

        if (shouldRetire)
        {
            dict->clearAndDeallocate();
        }
        else
        {
            dict->clear();
        }
        m_dictionaryPool.freeObject(pooledDict);
    }

    template<typename T>
    void free(HashSet<T*>* set)
    {
        auto pooledSet = (HashSet<void*>*)set;
        auto objectIndex = m_hashSetPool.getObjectIndex(pooledSet);
        auto liveCount = set->getCount();
        auto bucketCount = set->getBucketCount();
        bool shouldRetire = updateUnderuseStreakAndShouldRetire(
            m_hashSetUnderuseStreaks,
            objectIndex,
            liveCount,
            bucketCount);

        if (shouldRetire)
        {
            // Swap with an empty set instead of clearing first, because the oversized bucket array
            // is exactly the cost we are trying to stop paying on later small uses of this slot.
            set->clearAndDeallocate();
        }
        else
        {
            set->clear();
        }
        m_hashSetPool.freeObject(pooledSet);
    }
};
} // namespace Slang

#endif
