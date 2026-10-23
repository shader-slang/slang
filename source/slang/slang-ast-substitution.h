#pragma once

#include "core/slang-dictionary.h"
#include "core/slang-short-dictionary.h"
#include "slang-ast-support-types.h"

namespace Slang
{

/// Supplies composition-dependent bindings to the ordinary semantic substitution traversal.
/// Implementations replace a type, value, or conformance witness with an existing checked value
/// of the same semantic category. Returning the input leaves ordinary substitution in charge.
/// The provider and the substitution cache belong to one operation; neither is stored in a Val.
struct LinkTimeSubstitution
{
    virtual Val* trySubstitute(Val* val) = 0;
    /// Report a cycle in this operation and abort compilation. No substitute value is returned:
    /// a cycle can close on any semantic category, including a conformance witness.
    [[noreturn]] virtual void diagnoseCycle() = 0;
};

/// Caches the completed Val substitutions performed by one substitution operation.
///
/// Consider `struct Node<T : IModel> : IModel {}` and a type such as
/// `Node<Node<Leaf>>`. Each layer contains its inner type both as an ordinary generic argument and
/// in the declared conformance witness for `T : IModel`. Substitution follows both edges, so
/// without this cache the shared Val DAG is traversed as if it were a tree. The cache belongs to
/// the first substituteImpl dispatch on the stack and is propagated through SubstitutionSet copies.
///
/// Backed by `ShortDictionary` rather than `Dictionary` directly: most substitution operations
/// substitute through ordinary, shallow (non-shared) trees, where nearly every lookup misses and
/// the cache ends up holding only a few entries. A plain `Dictionary` pays for a heap-allocated
/// hash table on the very first `add`, even when the whole operation never revisits a single Val
/// -- pure overhead for the common case, and the root cause of #12139's front-end regression on
/// shallow generic code. `ShortDictionary` defers that allocation until an operation actually
/// substitutes through more than a handful of unique Vals, which is exactly the deeply-shared-DAG
/// case this cache exists for (see #12106 / #12100).
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

    SubstitutionCache(ASTBuilder* astBuilder, const SubstitutionSet& subst)
        : m_astBuilder(astBuilder)
        , m_substitutionDeclRef(subst.declRef)
        , m_linkTimeSubstitution(subst.linkTimeSubstitution)
    {
    }

    void validateContext(ASTBuilder* astBuilder, const SubstitutionSet& subst) const
    {
        SLANG_ASSERT(astBuilder == m_astBuilder);
        SLANG_ASSERT(subst.declRef == m_substitutionDeclRef);
        SLANG_ASSERT(subst.linkTimeSubstitution == m_linkTimeSubstitution);
        SLANG_ASSERT(subst.substitutionCache == this);
    }

    const Result* tryGet(const Key& key) const { return m_entries.tryGetValue(key); }

    void add(const Key& key, const Result& result) { m_entries.add(key, result); }

    // Use the same key as completed substitutions: distinct pack elements are distinct visits.
    bool beginLinkTimeSubstitution(const Key& key) { return m_activeLinkTimeValues.add(key); }
    void endLinkTimeSubstitution(const Key& key) { m_activeLinkTimeValues.remove(key); }

private:
    ASTBuilder* m_astBuilder = nullptr;
    DeclRefBase* m_substitutionDeclRef = nullptr;
    LinkTimeSubstitution* m_linkTimeSubstitution = nullptr;
    ShortDictionary<Key, Result> m_entries;
    HashSet<Key> m_activeLinkTimeValues;
};

/// Dispatches a Val substitution through the operation-local cache.
///
/// Entries are added only after dispatch returns. Ordinary generic substitutions retain their
/// existing recursion behavior; composition bindings additionally detect cycles introduced by
/// replacing extern declarations. The saved diff is a delta because substitution increments ioDiff.
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
        SubstitutionCache cache(astBuilder, subst);
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

    auto bindings = subst.linkTimeSubstitution;
    if (bindings && !cache->beginLinkTimeSubstitution(key))
    {
        bindings->diagnoseCycle();
    }
    SLANG_DEFER(if (bindings) cache->endLinkTimeSubstitution(key));

    int diff = 0;
    Val* result = nullptr;
    if (subst.linkTimeSubstitution)
    {
        auto replacement = subst.linkTimeSubstitution->trySubstitute(val);
        if (replacement != val)
        {
            // Follow dependencies in the selected definition with the same bindings and cache.
            result = replacement->substituteImpl(astBuilder, subst, &diff);
            SLANG_RELEASE_ASSERT(result);
            ++diff;
        }
    }
    if (!result)
    {
        result = dispatcher(subst, &diff);
        if (bindings && result != val)
        {
            // Requirement lookup can expose another link-time dependency. For example,
            // A.Assoc can resolve to B.Assoc, whose implementation refers back to A.Assoc.
            // Finish resolving that result before leaving this visit, so the same active
            // set detects the cycle instead of layout starting a fresh substitution query.
            SLANG_RELEASE_ASSERT(result);
            result = result->substituteImpl(astBuilder, subst, &diff);
        }
    }
    cache->add(key, {result, diff});
    *ioDiff += diff;
    return result;
}

} // namespace Slang
