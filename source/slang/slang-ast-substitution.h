#pragma once

#include "core/slang-dictionary.h"
#include "slang-ast-support-types.h"

namespace Slang
{

/// Caches the completed Val substitutions performed by one substitution operation.
///
/// Consider `struct Node<T : IModel> : IModel {}` and a type such as
/// `Node<Node<Leaf>>`. Each layer contains its inner type both as an ordinary generic argument and
/// in the declared conformance witness for `T : IModel`. Substitution follows both edges, so
/// without this cache the shared Val DAG is traversed as if it were a tree. The cache belongs to
/// the first substituteImpl dispatch on the stack and is propagated through SubstitutionSet copies.
struct SubstitutionCache
{
    struct Key
    {
        Val* val = nullptr;
        int packExpansionIndex = -1;

        bool operator==(const Key& other) const
        {
            return val == other.val && packExpansionIndex == other.packExpansionIndex;
        }

        HashCode getHashCode() const
        {
            return combineHash(Slang::getHashCode(val), Slang::getHashCode(packExpansionIndex));
        }
    };

    struct Result
    {
        Val* val = nullptr;
        int diff = 0;
    };

    SubstitutionCache(ASTBuilder* astBuilder, DeclRefBase* substitutionDeclRef)
        : m_astBuilder(astBuilder), m_substitutionDeclRef(substitutionDeclRef)
    {
    }

    void validateContext(ASTBuilder* astBuilder, const SubstitutionSet& subst) const
    {
        SLANG_ASSERT(astBuilder == m_astBuilder);
        SLANG_ASSERT(subst.declRef == m_substitutionDeclRef);
        SLANG_ASSERT(subst.substitutionCache == this);
    }

    const Result* tryGet(const Key& key) const
    {
        if (m_usesDictionary)
            return m_entries.tryGetValue(key);

        for (Index i = 0; i < m_inlineEntryCount; ++i)
        {
            if (m_inlineEntries[i].key == key)
                return &m_inlineEntries[i].result;
        }
        return nullptr;
    }

    void add(const Key& key, const Result& result)
    {
        if (m_usesDictionary)
        {
            m_entries.add(key, result);
            return;
        }

        if (m_inlineEntryCount < kInlineEntryCount)
        {
            auto& entry = m_inlineEntries[m_inlineEntryCount++];
            entry.key = key;
            entry.result = result;
            return;
        }

        m_entries.reserve(kInlineEntryCount + 1);
        for (Index i = 0; i < m_inlineEntryCount; ++i)
            m_entries.add(m_inlineEntries[i].key, m_inlineEntries[i].result);
        m_entries.add(key, result);
        m_usesDictionary = true;
    }

private:
    static const Index kInlineEntryCount = 8;

    struct Entry
    {
        Key key;
        Result result;
    };

    ASTBuilder* m_astBuilder = nullptr;
    DeclRefBase* m_substitutionDeclRef = nullptr;
    Entry m_inlineEntries[kInlineEntryCount];
    Index m_inlineEntryCount = 0;
    Dictionary<Key, Result> m_entries;
    bool m_usesDictionary = false;
};

/// Dispatches a Val substitution through the operation-local cache.
///
/// Entries are added only after dispatch returns, so recursive cycles retain their existing
/// behavior. The saved diff is a delta because substituteImpl is specified to increment ioDiff.
template<typename TDispatcher>
Val* substituteValWithCache(
    Val* val,
    ASTBuilder* astBuilder,
    SubstitutionSet subst,
    int* ioDiff,
    const TDispatcher& dispatcher)
{
    if (!subst.substitutionCache)
    {
        SubstitutionCache cache(astBuilder, subst.declRef);
        subst.substitutionCache = &cache;
        return substituteValWithCache(val, astBuilder, subst, ioDiff, dispatcher);
    }

    auto cache = subst.substitutionCache;
    cache->validateContext(astBuilder, subst);

    SubstitutionCache::Key key = {val, subst.packExpansionIndex};
    if (auto cachedResult = cache->tryGet(key))
    {
        *ioDiff += cachedResult->diff;
        return cachedResult->val;
    }

    int diff = 0;
    auto result = dispatcher(subst, &diff);
    cache->add(key, {result, diff});
    *ioDiff += diff;
    return result;
}

} // namespace Slang
