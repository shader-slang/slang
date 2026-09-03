#ifndef SLANG_CORE_SHORT_DICTIONARY_H
#define SLANG_CORE_SHORT_DICTIONARY_H

#include "slang-dictionary.h"

namespace Slang
{

/// A key-value map optimized for the common case of few entries.
///
/// Holds up to `kInlineCapacity` entries in a fixed inline array, looked up by linear scan, so
/// construction and the first several inserts do no heap allocation at all. Only once the entry
/// count exceeds `kInlineCapacity` does it promote to a real `Dictionary`, copying the inline
/// entries across once. After that, a lookup costs `Dictionary`'s O(1) average plus a fixed
/// `kInlineCapacity`-comparison linear scan of the inline array (checked first) -- that scan
/// does not keep growing past `kInlineCapacity`, but it also never goes away once the map has
/// promoted.
///
/// Add-only by design: there is no `remove` or in-place update, and `add` asserts on a
/// duplicate key. That is exactly what keeps a promoted entry's inline and `Dictionary` copies
/// from ever diverging (see the promotion comment on `add`), and what makes the `const TValue*`
/// `tryGetValue` returns into the inline array safe to hand out (the pointee is never mutated
/// after being written). A future `remove`/`set`-style addition would need to account for both
/// copies to preserve that.
///
/// `TKey`/`TValue` must be default-constructible: the inline arrays value-initialize
/// `kInlineCapacity` slots of each up front, even for an empty map -- a constraint `Dictionary`
/// itself does not impose.
///
/// Intended for caches that are constructed fresh per operation and, in the common case, end up
/// holding only a handful of entries -- e.g. a cache scoped to one substitution operation or one
/// visibility query, walking a DAG that usually has little internal sharing. For such a cache, a
/// plain `Dictionary` pays for a heap-allocated hash table on the very first insert even when the
/// whole operation never needs more than one or two entries.
template<typename TKey, typename TValue, int kInlineCapacity = 8>
class ShortDictionary
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

    /// Adds `key` and `value` to this dictionary. `key` must not already be present -- call
    /// `tryGetValue` first (this type is add-only; see the class comment).
    ///
    /// Asserts uniformly -- in both debug and release builds, and the same way whether or not
    /// this map has promoted -- when `key` is already present, matching `Dictionary::add`'s
    /// duplicate-key contract.
    void add(const TKey& key, const TValue& value)
    {
        if (!m_overflowed)
        {
            for (Index i = 0; i < m_inlineCount; i++)
                SLANG_RELEASE_ASSERT(!(m_inlineKeys[i] == key));

            if (m_inlineCount < kInlineCapacity)
            {
                m_inlineKeys[m_inlineCount] = key;
                m_inlineValues[m_inlineCount] = value;
                m_inlineCount++;
                return;
            }

            // Promote once: copy the inline entries into the real Dictionary so a case with many
            // unique keys still gets O(1) average lookups instead of an ever-growing linear scan.
            // The inline array is left populated (not cleared): later lookups still check it
            // first, and every entry there is byte-identical to its Dictionary copy (this type is
            // add-only, so neither copy can drift after being written), so reading through either
            // one gives the same answer.
            for (Index i = 0; i < m_inlineCount; i++)
                m_overflow.add(m_inlineKeys[i], m_inlineValues[i]);
            m_overflowed = true;
        }

        // `Dictionary::add`'s own duplicate check is debug-only strength (skippable under
        // `SLANG_ASSERT=release-asserts-only`), which would otherwise make a duplicate add here
        // behave differently -- silently keeping the old value instead of asserting -- than the
        // inline path above for the exact same misuse. Check explicitly first so both paths give
        // the same guarantee at the same strength.
        SLANG_RELEASE_ASSERT(!m_overflow.tryGetValue(key));
        m_overflow.add(key, value);
    }

private:
    TKey m_inlineKeys[kInlineCapacity] = {};
    TValue m_inlineValues[kInlineCapacity] = {};
    Index m_inlineCount = 0;
    bool m_overflowed = false;
    Dictionary<TKey, TValue> m_overflow;
};

} // namespace Slang

#endif
