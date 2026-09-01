#ifndef SLANG_CORE_SMALL_DICTIONARY_H
#define SLANG_CORE_SMALL_DICTIONARY_H

#include "slang-dictionary.h"

namespace Slang
{

/// A key-value map optimized for the common case of few entries.
///
/// Holds up to `kInlineCapacity` entries in a fixed inline array, looked up by linear scan, so
/// construction and the first several inserts do no heap allocation at all. Only once the entry
/// count exceeds `kInlineCapacity` does it promote to a real `Dictionary`, moving the inline
/// entries across once, so later lookups on a case with many unique keys still get the same O(1)
/// average cost a plain `Dictionary` would give (the inline scan does not keep growing past that
/// point).
///
/// Intended for caches that are constructed fresh per operation and, in the common case, end up
/// holding only a handful of entries -- e.g. a cache scoped to one substitution operation or one
/// visibility query, walking a DAG that usually has little internal sharing. For such a cache, a
/// plain `Dictionary` pays for a heap-allocated hash table on the very first insert even when the
/// whole operation never needs more than one or two entries.
template<typename TKey, typename TValue, int kInlineCapacity = 8>
class SmallDictionary
{
public:
    const TValue* tryGetValue(const TKey& key) const
    {
        for (Index i = 0; i < m_inlineCount; i++)
        {
            if (m_inlineKeys[i] == key)
                return &m_inlineValues[i];
        }
        if (m_overflowed)
            return m_overflow.tryGetValue(key);
        return nullptr;
    }

    void add(const TKey& key, const TValue& value)
    {
        if (!m_overflowed)
        {
            if (m_inlineCount < kInlineCapacity)
            {
                m_inlineKeys[m_inlineCount] = key;
                m_inlineValues[m_inlineCount] = value;
                m_inlineCount++;
                return;
            }

            // Promote once: move the inline entries into the real Dictionary so a case with many
            // unique keys still gets O(1) average lookups instead of an ever-growing linear scan.
            for (Index i = 0; i < m_inlineCount; i++)
                m_overflow.add(m_inlineKeys[i], m_inlineValues[i]);
            m_overflowed = true;
        }
        m_overflow.add(key, value);
    }

private:
    TKey m_inlineKeys[kInlineCapacity];
    TValue m_inlineValues[kInlineCapacity];
    Index m_inlineCount = 0;
    bool m_overflowed = false;
    Dictionary<TKey, TValue> m_overflow;
};

} // namespace Slang

#endif
