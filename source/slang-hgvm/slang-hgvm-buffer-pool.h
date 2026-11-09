#pragma once

#include "core/slang-basic.h"
#include "core/slang-dictionary.h"
#include "slang-hgvm-events.h"
#include "slang-hgvm.h"

namespace Slang::HGVM
{
/// Caches whole buffers whose program and GPU leases have both ended. The pool owns no leases,
/// so a lease can retain this object without creating a cycle or depending on DeviceState lifetime.
class BufferPool : public RefObject
{
public:
    struct Key
    {
        uint64_t size;
        uint32_t elementSize;
        rhi::Format format;
        rhi::MemoryType memoryType;
        rhi::BufferUsage usage;
        rhi::ResourceState defaultState;

        explicit Key(const rhi::BufferDesc& desc)
            : size(desc.size)
            , elementSize(desc.elementSize)
            , format(desc.format)
            , memoryType(desc.memoryType)
            , usage(desc.usage)
            , defaultState(desc.defaultState)
        {
        }

        bool operator==(const Key& other) const
        {
            return size == other.size && elementSize == other.elementSize &&
                   format == other.format && memoryType == other.memoryType &&
                   usage == other.usage && defaultState == other.defaultState;
        }

        HashCode getHashCode() const
        {
            HashCode hash = Slang::getHashCode(size);
            hash = combineHash(hash, Slang::getHashCode(elementSize));
            hash = combineHash(hash, Slang::getHashCode(uint32_t(format)));
            hash = combineHash(hash, Slang::getHashCode(uint32_t(memoryType)));
            hash = combineHash(hash, Slang::getHashCode(uint32_t(usage)));
            return combineHash(hash, Slang::getHashCode(uint32_t(defaultState)));
        }
    };

    explicit BufferPool(uint64_t byteLimit, EventTracker* events = nullptr)
        : m_byteLimit(byteLimit)
    {
#if SLANG_ENABLE_HGVM_EVENT_TRACKING
        m_eventTracker = events;
#endif
    }

#if SLANG_ENABLE_HGVM_EVENT_TRACKING
    ~BufferPool()
    {
        // Destruction releases cached references without running the LRU eviction algorithm.
        for (const auto& bucket : m_buffers)
            for (const auto& entry : bucket.second)
                SLANG_HGVM_TRACK(
                    m_eventTracker.get(),
                    trackBufferEviction(entry.buffer.get(), bucket.first.size));
    }
#endif

    /// Takes an exact match. Descriptors with extensions or external sharing
    /// are deliberately not cached: their compatibility is not described by Key.
    ComPtr<rhi::IBuffer> acquire(const rhi::BufferDesc& desc)
    {
        if (isCacheable(desc))
        {
            Key key(desc);
            if (auto entries = m_buffers.tryGetValue(key))
            {
                ComPtr<rhi::IBuffer> buffer = std::move(entries->getLast().buffer);
                entries->removeLast();
                if (!entries->getCount())
                    m_buffers.remove(key);
                m_stats.cachedBytes -= desc.size;
                ++m_stats.hits;
                SLANG_HGVM_TRACK(m_eventTracker.get(), trackBufferReuse(buffer.get(), desc.size));
                return buffer;
            }
        }
        return nullptr;
    }

    void recordMiss() { ++m_stats.misses; }
    void beginLease(uint64_t size) { m_stats.leasedBytes += size; }

    /// Returns a completed lease without clearing its contents. In particular, callers still
    /// have to initialize buffers used as atomic accumulators before their first dispatch.
    void recycle(const rhi::BufferDesc& desc, ComPtr<rhi::IBuffer> buffer)
    {
        SLANG_ASSERT(buffer && m_stats.leasedBytes >= desc.size);
        m_stats.leasedBytes -= desc.size;
        SLANG_HGVM_TRACK(m_eventTracker.get(), trackBufferRecycle(buffer.get(), desc.size));
        if (!isCacheable(desc) || desc.size > m_byteLimit)
        {
            SLANG_HGVM_TRACK(m_eventTracker.get(), trackBufferEviction(buffer.get(), desc.size));
            return;
        }
        trim(m_byteLimit - desc.size);
        Key key(desc);
        auto entries = m_buffers.tryGetValue(key);
        if (!entries)
        {
            m_buffers.add(key, List<Entry>());
            entries = m_buffers.tryGetValue(key);
        }
        entries->add({std::move(buffer), ++m_serial});
        m_stats.cachedBytes += desc.size;
    }

    /// Evicts least-recently-returned allocations. Only eviction scans the cache; exact-match
    /// allocation uses a hash lookup. This drops RHI references, not necessarily physical memory.
    void trim(uint64_t targetBytes = 0)
    {
        while (m_stats.cachedBytes > targetBytes)
        {
            const Key* oldestKey = nullptr;
            uint64_t oldestSerial = UINT64_MAX;
            for (const auto& bucket : m_buffers)
            {
                if (bucket.second[0].serial < oldestSerial)
                {
                    oldestKey = &bucket.first;
                    oldestSerial = bucket.second[0].serial;
                }
            }
            SLANG_ASSERT(oldestKey);
            Key key = *oldestKey;
            auto entries = m_buffers.tryGetValue(key);
            SLANG_HGVM_TRACK(
                m_eventTracker.get(),
                trackBufferEviction((*entries)[0].buffer.get(), key.size));
            entries->removeAt(0);
            if (!entries->getCount())
                m_buffers.remove(key);
            m_stats.cachedBytes -= key.size;
            ++m_stats.evictions;
        }
    }

    BufferPoolStats getStats() const { return m_stats; }

private:
    bool isCacheable(const rhi::BufferDesc& desc) const
    {
        return m_byteLimit && desc.size && !desc.next &&
               (uint32_t(desc.usage) & uint32_t(rhi::BufferUsage::Shared)) == 0;
    }

    struct Entry
    {
        ComPtr<rhi::IBuffer> buffer;
        uint64_t serial;
    };
    Dictionary<Key, List<Entry>> m_buffers;
#if SLANG_ENABLE_HGVM_EVENT_TRACKING
    RefPtr<EventTracker> m_eventTracker;
#endif
    uint64_t m_byteLimit;
    uint64_t m_serial = 0;
    BufferPoolStats m_stats;
};
} // namespace Slang::HGVM
