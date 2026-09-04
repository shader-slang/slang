// slang-ast-decl-ref.cpp

#include "slang-ast-builder.h"
#include "slang-ast-dispatch.h"
#include "slang-ast-forward-declarations.h"
#include "slang-ast-substitution.h"
#include "slang-check-impl.h"
#include "slang-syntax.h"

namespace Slang
{

DeclRefBase* DirectDeclRef::_substituteImplOverride(
    ASTBuilder* astBuilder,
    SubstitutionSet subst,
    int* ioDiff)
{
    SLANG_UNUSED(astBuilder);
    SLANG_UNUSED(subst);
    SLANG_UNUSED(ioDiff);
    return this;
}

void DirectDeclRef::_toTextOverride(StringBuilder& out)
{
    if (getDecl()->getName() && getDecl()->getName()->text.getLength() != 0)
    {
        out << getDecl()->getName()->text;
    }
}

Val* DirectDeclRef::_resolveImplOverride()
{
    return this;
}

DeclRefBase* DirectDeclRef::_getBaseOverride()
{
    return nullptr;
}

DeclRefBase* _getDeclRefFromVal(Val* val)
{
    if (auto declRefType = as<DeclRefType>(val))
        return declRefType->getDeclRef();
    else if (auto genParamIntVal = as<DeclRefIntVal>(val))
        return genParamIntVal->getDeclRef();
    else if (auto declaredSubtypeWitness = as<DeclaredSubtypeWitness>(val))
        return declaredSubtypeWitness->getDeclRef();
    else if (auto declRef = as<DeclRefBase>(val))
        return declRef;
    return nullptr;
}

DeclRefBase* _resolveAsDeclRef(DeclRefBase* declRefToResolve)
{
    if (auto rs = _getDeclRefFromVal(declRefToResolve->resolve()))
        return rs;
    return declRefToResolve;
}

static AccessorDecl* _tryGetCorrespondingAccessorDecl(Decl* memberDecl, Decl* substParentDecl)
{
    // Once substitution has resolved the parent requirement to a satisfying declaration, the child
    // must be selected from that same parent. Accessors are anonymous role declarations (`get` or
    // `set`) nested under a storage declaration, so a requirement getter can be mapped to the
    // getter under the selected override/default subscript while preserving the selected parent's
    // generic substitutions.
    //
    // Conceptually this is not quite an ordinary static `MemberDeclRef`; it is projecting an
    // accessor role from the resolved storage declaration. Consider this example:
    //
    //     interface ITensor<T, int D>
    //     {
    //         __subscript<each TIndex>(TIndex indices)
    //             where TIndex == int
    //             where countof(TIndex) == D
    //         {
    //             [Differentiable]
    //             get { return load(indices); }
    //         }
    //     }
    //
    // During conformance checking, the requirement getter is represented as:
    //
    //     MemberDeclRef(GenericAppDeclRef(Lookup(This, operator[]), TIndex), get)
    //
    // After `Lookup(This, operator[])` resolves through the witness table to the selected concrete
    // or default subscript, the `get` accessor must be re-selected under that same resolved
    // storage declaration while preserving the subscript's generic arguments. That behavior is
    // closer to a possible future
    // `AccessorProjectionDeclRef(parentStorageDeclRef, AccessorKind::Get)`. We encode that
    // projection here to avoid adding a new decl-ref kind and the corresponding lookup/cache
    // machinery on storage declarations. If this pattern grows beyond accessors, consider making
    // that projection explicit instead of extending this remap.
    auto accessorDecl = as<AccessorDecl>(memberDecl);
    if (!accessorDecl)
        return nullptr;

    if (accessorDecl->parentDecl == substParentDecl)
        return accessorDecl;

    auto originalParentDecl = accessorDecl->parentDecl;
    if (!originalParentDecl || originalParentDecl->astNodeType != substParentDecl->astNodeType)
        return nullptr;

    auto substParentContainer = as<ContainerDecl>(substParentDecl);
    if (!substParentContainer)
        return nullptr;

    AccessorDecl* result = nullptr;
    for (auto candidateDecl : substParentContainer->getDirectMemberDeclsOfType<AccessorDecl>())
    {
        if (candidateDecl->astNodeType != accessorDecl->astNodeType)
            continue;
        if (result)
            return nullptr;
        result = candidateDecl;
    }

    return result;
}

DeclRefBase* MemberDeclRef::_substituteImplOverride(
    ASTBuilder* astBuilder,
    SubstitutionSet subst,
    int* ioDiff)
{
    int diff = 0;
    auto substParent = getParentOperand()->substituteImpl(astBuilder, subst, &diff);
    if (diff)
    {
        (*ioDiff)++;

        if (!getDecl()->isChildOf(substParent->getDecl()))
        {
            if (auto correspondingAccessor =
                    _tryGetCorrespondingAccessorDecl(getDecl(), substParent->getDecl()))
            {
                return astBuilder
                    ->getMemberDeclRef(DeclRef<Decl>(substParent), correspondingAccessor)
                    .declRefBase;
            }
        }

        return astBuilder->getMemberDeclRef(substParent, getDecl());
    }
    return this;
}

void MemberDeclRef::_toTextOverride(StringBuilder& out)
{
    getParentOperand()->toText(out);
    if (out.getLength() && !out.endsWith("."))
        out << ".";
    if (getDecl()->getName() && getDecl()->getName()->text.getLength() != 0)
    {
        out << getDecl()->getName()->text;
    }
}

Val* MemberDeclRef::_resolveImplOverride()
{
    auto resolvedParent = _resolveAsDeclRef(getParentOperand());
    if (resolvedParent != getParentOperand())
    {
        auto newChild = getDecl();

        if (newChild->parentDecl != resolvedParent->getDecl())
        {
            if (auto correspondingAccessor =
                    _tryGetCorrespondingAccessorDecl(newChild, resolvedParent->getDecl()))
            {
                newChild = correspondingAccessor;
            }
        }

        return getCurrentASTBuilder()->getMemberDeclRef(resolvedParent, newChild);
    }
    return this;
}

DeclRefBase* MemberDeclRef::_getBaseOverride()
{
    return getParentOperand();
}

Decl* LookupDeclRef::getSupDecl()
{
    if (auto supType = as<DeclRefType>(getWitness()->getSup()))
    {
        return supType->getDeclRef().getDecl();
    }
    // If we reach here, something is wrong.
    SLANG_UNEXPECTED("Invalid lookup declref");
}

DeclRefBase* LookupDeclRef::_substituteImplOverride(
    ASTBuilder* astBuilder,
    SubstitutionSet subst,
    int* ioDiff)
{
    int diff = 0;

    auto substWitness = as<SubtypeWitness>(getWitness()->substituteImpl(astBuilder, subst, &diff));
    if (diff == 0)
        return this;
    (*ioDiff)++;

    auto substSource = as<Type>(getLookupSource()->substituteImpl(astBuilder, subst, &diff));
    SLANG_ASSERT(substSource);

    if (auto resolved = _getDeclRefFromVal(tryResolve(substWitness, substSource)))
        return resolved;

    return astBuilder->getLookupDeclRef(substSource, substWitness, getDecl());
}

void LookupDeclRef::_toTextOverride(StringBuilder& out)
{
    if (!as<ThisType>(getLookupSource()))
    {
        getLookupSource()->toText(out);
        if (out.getLength() && !out.endsWith("."))
            out << ".";
    }
    if (getDecl()->getName() && getDecl()->getName()->text.getLength() != 0)
    {
        out << getDecl()->getName()->text;
    }
}

Val* LookupDeclRef::_resolveImplOverride()
{
    auto astBuilder = getCurrentASTBuilder();
    Val* resolved = this;

    auto newLookupSource = as<Type>(getLookupSource()->resolve());
    SLANG_ASSERT(newLookupSource);

    auto newWitness = as<SubtypeWitness>(getWitness()->resolve());
    SLANG_ASSERT(newWitness);

    if (auto resolvedVal = tryResolve(newWitness, newLookupSource))
        return resolvedVal;
    if (newLookupSource != getLookupSource() || newWitness != getWitness())
        resolved = astBuilder->getLookupDeclRef(newLookupSource, newWitness, getDecl());
    return resolved;
}

DeclRefBase* LookupDeclRef::_getBaseOverride()
{
    auto supType = getWitness()->getSup();
    if (auto declRefType = as<DeclRefType>(supType))
    {
        return declRefType->getDeclRef();
    }
    return nullptr;
}

// Requirement projections are represented by a requirement declaration plus a subtype witness.
// Witness tables store answers in the declaration context of the conformance that owns the table;
// the subtype-witness path supplies the substitutions needed by a particular projection. Lookup
// consequently has two phases: select an unspecialized table entry without copying intermediate
// tables, and then apply the substitutions from the traversed witness path to the selected leaf.
//
// Consider this example:
//
//     interface ISidekick { associatedtype Hero; }
//     struct Sidekick<H> : ISidekick { typealias Hero = H; }
//
// The `Sidekick<H> : ISidekick` table stores `Hero = Sidekick<H>.Hero` in declaration context. A
// projection through `Sidekick<Batman> : ISidekick` must select that same entry and only then apply
// `H -> Batman`. An inherited-interface projection may cross several such tables, so the forceful
// semantic path reports the first missing table or entry and restarts after publishing it. Ordinary
// `Val::resolve()` cannot publish entries and therefore uses a separate allocation-free traversal.

/// Stores a passive lookup result and the declaration-context substitutions not yet applied to it.
struct UnspecializedRequirementWitnessLookupFrontier
{
    RequirementWitnessLookupFrontier frontier;

    /// The inner-to-outer conformance decl-refs whose substitutions specialize a found leaf.
    List<DeclRef<Decl>> specializationDeclRefs;
};

/// Applies the declaration contexts accumulated while locating one table entry.
static RequirementWitness _specializeRequirementWitnessAlongLookupPath(
    ASTBuilder* astBuilder,
    RequirementWitness witness,
    List<DeclRef<Decl>> const& specializationDeclRefs)
{
    for (auto specializationDeclRef : specializationDeclRefs)
    {
        witness = witness.specialize(astBuilder, SubstitutionSet(specializationDeclRef));
    }
    return witness;
}

/// Identifies the declaration used as a key in one witness table and the generic result shape that
/// must be reconstructed after retrieving the stored witness.
struct RequirementWitnessTableLookupKey
{
    RequirementWitnessTableLookupKey(InterfaceRequirementKey key, UCount genericWrapperCount)
        : key(key), genericWrapperCount(genericWrapperCount)
    {
    }

    InterfaceRequirementKey key;
    UCount genericWrapperCount;
};

static RequirementWitnessTableLookupKey _getRequirementWitnessTableLookupKey(
    DeclRef<Decl> requirementDeclRef)
{
    UCount genericWrapperCount = 0;
    auto key = InterfaceRequirementKey::createWithGenericWrapperCount(
        requirementDeclRef.getDecl(),
        genericWrapperCount);
    return RequirementWitnessTableLookupKey(key, genericWrapperCount);
}

/// Resolves a stored table entry and restores the outer generic declaration requested by lookup.
///
/// A generic interface requirement is keyed by its innermost non-generic declaration. Its
/// satisfying witness likewise names the corresponding inner declaration, with the same consecutive
/// chain of `GenericDecl` parents. Lookup climbs exactly the number of wrappers removed from the
/// requested key and returns the decl-ref for the outermost corresponding satisfying wrapper.
/// Substitutions from the conformance path remain unapplied here; the caller applies them after
/// selecting the final leaf entry.
static RequirementWitness _resolveRequirementWitnessForTableLookup(
    RequirementWitness requirementWitness,
    UCount genericWrapperCount)
{
    switch (requirementWitness.getFlavor())
    {
    default:
        SLANG_UNEXPECTED("unknown requirement witness flavor");
    case RequirementWitness::Flavor::none:
    case RequirementWitness::Flavor::witnessTable:
        return requirementWitness;
    case RequirementWitness::Flavor::declRef:
        {
            auto satisfyingDeclRef =
                as<DeclRefBase>(requirementWitness.getDeclRef().declRefBase->resolve());
            while (genericWrapperCount > 0)
            {
                SLANG_RELEASE_ASSERT(satisfyingDeclRef);
                auto parent = satisfyingDeclRef->getParent();
                SLANG_RELEASE_ASSERT(parent && as<GenericDecl>(parent->getDecl()));
                satisfyingDeclRef = parent;
                genericWrapperCount--;
            }
            SLANG_RELEASE_ASSERT(satisfyingDeclRef);
            return RequirementWitness(satisfyingDeclRef);
        }
    case RequirementWitness::Flavor::val:
        {
            SLANG_RELEASE_ASSERT(genericWrapperCount == 0);
            return RequirementWitness(requirementWitness.getVal()->resolve());
        }
    }
}

/// Looks up one declaration-context entry without performing semantic checking.
///
/// Generic wrappers are removed only to form the table-local key. The returned witness is still
/// unspecialized with respect to the subtype-witness path that led to this table.
static bool _tryLookUpRequirementEntryInTable(
    WitnessTable* witnessTable,
    DeclRef<Decl> requirementDeclRef,
    RequirementWitness* outRequirementWitness)
{
    auto lookupKey = _getRequirementWitnessTableLookupKey(requirementDeclRef);
    RequirementWitness requirementWitness;
    if (!witnessTable->tryGetRequirementWitness(lookupKey.key, requirementWitness))
    {
        return false;
    }
    *outRequirementWitness =
        _resolveRequirementWitnessForTableLookup(requirementWitness, lookupKey.genericWrapperCount);
    return true;
}

/// Selects the branch of `packBranchWitness` whose cardinality is structurally known.
///
/// Returning null means that the pack cardinality remains indeterminate. Keeping this dispatch in
/// one helper prevents passive selection, passive specialization, and forceful frontier lookup from
/// choosing different branches as the variadic witness representation evolves.
static SubtypeWitness* _tryGetKnownPackBranchWitness(PackBranchSubtypeWitness* packBranchWitness)
{
    switch (getKnownPackCardinality(packBranchWitness->getPackOperand()))
    {
    case VariadicPackCardinality::Empty:
        return packBranchWitness->getEmptyWitness();
    case VariadicPackCardinality::NonEmpty:
        return packBranchWitness->getNonEmptyWitness();
    default:
        return nullptr;
    }
}

/// Follows existing witness tables and returns the unspecialized leaf entry.
///
/// This is the allocation-free passive path used by ordinary `Val::resolve()`. It intentionally
/// does not construct missing-frontier metadata, because a passive caller cannot act on it.
static RequirementWitness _tryLookUpExistingRequirementWitnessRec(
    ASTBuilder* astBuilder,
    SubtypeWitness* subtypeWitness,
    DeclRef<Decl> requirementDeclRef)
{
    if (auto packBranchWitness = as<PackBranchSubtypeWitness>(subtypeWitness))
    {
        auto selectedWitness = _tryGetKnownPackBranchWitness(packBranchWitness);
        if (!selectedWitness)
            return RequirementWitness();
        return _tryLookUpExistingRequirementWitnessRec(
            astBuilder,
            selectedWitness,
            requirementDeclRef);
    }

    if (auto declaredSubtypeWitness = as<DeclaredSubtypeWitness>(subtypeWitness))
    {
        RefPtr<WitnessTable> witnessTable;
        auto conformanceDeclRef = declaredSubtypeWitness->getDeclRef();
        if (auto nestedLookupDeclRef = as<LookupDeclRef>(conformanceDeclRef.declRefBase))
        {
            auto nestedWitness = _tryLookUpExistingRequirementWitnessRec(
                astBuilder,
                nestedLookupDeclRef->getWitness(),
                DeclRef<Decl>(nestedLookupDeclRef));
            if (nestedWitness.getFlavor() != RequirementWitness::Flavor::witnessTable)
            {
                // A conformance requirement is stored as a `SubtypeWitness` value. Continuing
                // through it may require resolving its projected subtype after semantic checking
                // publishes another entry. This passive path cannot cause that progress; the
                // forceful frontier path reports `NeedsConcreteConformance` instead.
                return RequirementWitness();
            }
            witnessTable = nestedWitness.getWitnessTable();
        }
        else if (auto inheritanceDeclRef = conformanceDeclRef.as<InheritanceDecl>())
        {
            witnessTable = inheritanceDeclRef.getDecl()->witnessTable;
        }
        else if (auto constraintDeclRef = conformanceDeclRef.as<GenericTypeConstraintDecl>())
        {
            witnessTable = constraintDeclRef.getDecl()->pathResolutionTable;
        }

        if (!witnessTable)
            return RequirementWitness();

        RequirementWitness requirementWitness;
        if (!_tryLookUpRequirementEntryInTable(
                witnessTable,
                requirementDeclRef,
                &requirementWitness))
        {
            return RequirementWitness();
        }
        return requirementWitness;
    }

    if (auto transitiveWitness = as<TransitiveSubtypeWitness>(subtypeWitness))
    {
        if (auto midToSupWitness = as<DeclaredSubtypeWitness>(transitiveWitness->getMidToSup()))
        {
            auto midRequirementDeclRef = midToSupWitness->getDeclRef();
            auto midWitness = _tryLookUpExistingRequirementWitnessRec(
                astBuilder,
                as<SubtypeWitness>(transitiveWitness->getSubToMid()),
                midRequirementDeclRef);
            if (midWitness.getFlavor() != RequirementWitness::Flavor::witnessTable)
                return RequirementWitness();

            RequirementWitness requirementWitness;
            if (!_tryLookUpRequirementEntryInTable(
                    midWitness.getWitnessTable(),
                    requirementDeclRef,
                    &requirementWitness))
            {
                return RequirementWitness();
            }
            return requirementWitness;
        }
    }

    return RequirementWitness();
}

/// Replays an existing lookup path and applies its substitutions to the selected leaf.
///
/// A second traversal keeps the structural lookup allocation-free and avoids specializing an
/// intermediate witness table, which would copy every entry before selecting the next one.
/// This traversal is deliberately paired with `_tryLookUpExistingRequirementWitnessRec`: every
/// witness shape accepted by the selection pass must visit the same conformance declarations here,
/// in the same inner-to-outer order. The forceful frontier traversal below must recognize that same
/// structural vocabulary, although it stops at the first missing step instead of reaching a leaf.
/// The shared shapes are a known `PackBranchSubtypeWitness`, a `DeclaredSubtypeWitness` backed by a
/// nested lookup, inheritance declaration, or generic constraint, and a `TransitiveSubtypeWitness`
/// whose mid-to-super witness is declared. Selection distinguishes the three declared-witness
/// sources to find their table; specialization intentionally treats them uniformly because only
/// the substitutions on their conformance decl-ref matter at that stage.
static RequirementWitness _specializeExistingRequirementWitnessRec(
    ASTBuilder* astBuilder,
    SubtypeWitness* subtypeWitness,
    RequirementWitness requirementWitness)
{
    if (auto packBranchWitness = as<PackBranchSubtypeWitness>(subtypeWitness))
    {
        auto selectedWitness = _tryGetKnownPackBranchWitness(packBranchWitness);
        if (!selectedWitness)
            return RequirementWitness();
        return _specializeExistingRequirementWitnessRec(
            astBuilder,
            selectedWitness,
            requirementWitness);
    }

    if (auto declaredSubtypeWitness = as<DeclaredSubtypeWitness>(subtypeWitness))
    {
        auto conformanceDeclRef = declaredSubtypeWitness->getDeclRef();
        if (auto nestedLookupDeclRef = as<LookupDeclRef>(conformanceDeclRef.declRefBase))
        {
            requirementWitness = _specializeExistingRequirementWitnessRec(
                astBuilder,
                nestedLookupDeclRef->getWitness(),
                requirementWitness);
        }
        return requirementWitness.specialize(astBuilder, SubstitutionSet(conformanceDeclRef));
    }

    if (auto transitiveWitness = as<TransitiveSubtypeWitness>(subtypeWitness))
    {
        if (auto midToSupWitness = as<DeclaredSubtypeWitness>(transitiveWitness->getMidToSup()))
        {
            requirementWitness = _specializeExistingRequirementWitnessRec(
                astBuilder,
                as<SubtypeWitness>(transitiveWitness->getSubToMid()),
                requirementWitness);
            return requirementWitness.specialize(
                astBuilder,
                SubstitutionSet(midToSupWitness->getDeclRef()));
        }
    }

    return RequirementWitness();
}

/// Passively looks up an existing requirement witness for ordinary value resolution.
///
/// Forceful semantic clients use `locateNextRequirementWitnessLookupFrontier` instead. Keeping the
/// operations separate lets this hot path avoid allocating a specialization list or producing
/// missing-frontier state that its callers cannot consume.
RequirementWitness tryLookUpRequirementWitness(
    ASTBuilder* astBuilder,
    SubtypeWitness* subtypeWitness,
    Decl* requirementKey)
{
    auto requirementDeclRef = makeDeclRef(requirementKey);
    auto requirementWitness =
        _tryLookUpExistingRequirementWitnessRec(astBuilder, subtypeWitness, requirementDeclRef);

    if (requirementWitness.getFlavor() == RequirementWitness::Flavor::none)
    {
        if (as<ThisTypeDecl>(requirementKey))
            return RequirementWitness(subtypeWitness->getSub());
        if (as<ThisTypeConstraintDecl>(requirementKey))
            return RequirementWitness(subtypeWitness);
        return RequirementWitness();
    }

    // These clients consume values and declaration references. Keep an intermediate table in its
    // declaration-context form so looking through inherited conformances does not specialize an
    // entire table only to select one entry from it.
    if (requirementWitness.getFlavor() == RequirementWitness::Flavor::witnessTable)
        return requirementWitness;
    auto specializedWitness =
        _specializeExistingRequirementWitnessRec(astBuilder, subtypeWitness, requirementWitness);
    // The passive selection pass already proved that this witness shape has a determinate path to
    // the leaf. A fallthrough here means the paired structural traversals have drifted apart.
    SLANG_RELEASE_ASSERT(specializedWitness.getFlavor() != RequirementWitness::Flavor::none);
    return specializedWitness;
}

static UnspecializedRequirementWitnessLookupFrontier _locateRequirementEntryInTable(
    RefPtr<WitnessTable> witnessTable,
    DeclRef<Decl> requirementDeclRef)
{
    UnspecializedRequirementWitnessLookupFrontier result;
    RequirementWitness requirementWitness;
    if (!_tryLookUpRequirementEntryInTable(witnessTable, requirementDeclRef, &requirementWitness))
    {
        // Preserve the caller's full decl-ref, including generic wrappers and substitutions. The
        // bare declaration above is only the table-local key; semantic checking may need the outer
        // generic requirement in order to synthesize the entry stored under its inner declaration.
        result.frontier =
            RequirementWitnessLookupFrontier::makeMissingEntry(witnessTable, requirementDeclRef);
        return result;
    }

    result.frontier = RequirementWitnessLookupFrontier::makeFound(requirementWitness);
    return result;
}

static UnspecializedRequirementWitnessLookupFrontier _locateNextRequirementWitnessLookupFrontierRec(
    ASTBuilder* astBuilder,
    SubtypeWitness* subtypeWitness,
    DeclRef<Decl> requirementDeclRef)
{
    UnspecializedRequirementWitnessLookupFrontier result;

    if (auto packBranchWitness = as<PackBranchSubtypeWitness>(subtypeWitness))
    {
        auto selectedWitness = _tryGetKnownPackBranchWitness(packBranchWitness);
        if (!selectedWitness)
            return result;
        return _locateNextRequirementWitnessLookupFrontierRec(
            astBuilder,
            selectedWitness,
            requirementDeclRef);
    }

    if (auto declaredSubtypeWitness = as<DeclaredSubtypeWitness>(subtypeWitness))
    {
        RefPtr<WitnessTable> witnessTable;
        List<DeclRef<Decl>> prefixSpecializations;
        auto declaredConformanceDeclRef = declaredSubtypeWitness->getDeclRef();

        if (auto nestedLookupDeclRef = as<LookupDeclRef>(declaredConformanceDeclRef.declRefBase))
        {
            auto nestedResult = _locateNextRequirementWitnessLookupFrontierRec(
                astBuilder,
                nestedLookupDeclRef->getWitness(),
                DeclRef<Decl>(nestedLookupDeclRef));
            if (nestedResult.frontier.getStatus() != RequirementWitnessLookupFrontierStatus::Found)
                return nestedResult;

            auto nestedWitness = nestedResult.frontier.getWitness();
            if (nestedWitness.getFlavor() == RequirementWitness::Flavor::witnessTable)
            {
                witnessTable = nestedWitness.getWitnessTable();
                prefixSpecializations = _Move(nestedResult.specializationDeclRefs);
            }
            else if (nestedWitness.getFlavor() == RequirementWitness::Flavor::val)
            {
                // A lookup-backed conformance requirement is stored as its `SubtypeWitness`
                // value, not as the table selected by that witness. Consider this example:
                //
                //     interface IInner { associatedtype Value; }
                //     interface IOuter { associatedtype Element : IInner; }
                //
                // The conformance witness used by `T.Element.Value` first projects the sibling
                // `Element : IInner` requirement from `T : IOuter`. Specialize that projected
                // witness into the concrete outer conformance, then continue the original `Value`
                // lookup through the resulting `T.Element : IInner` witness.
                auto specializedNestedWitness = _specializeRequirementWitnessAlongLookupPath(
                    astBuilder,
                    nestedWitness,
                    nestedResult.specializationDeclRefs);
                auto nestedSubtypeWitness = as<SubtypeWitness>(specializedNestedWitness.getVal());
                SLANG_RELEASE_ASSERT(nestedSubtypeWitness);
                result.frontier = RequirementWitnessLookupFrontier::makeNeedsConcreteConformance(
                    nestedSubtypeWitness);
                return result;
            }
            else if (nestedWitness.getFlavor() == RequirementWitness::Flavor::none)
            {
                // An absent optional conformance does not provide a concrete path to continue.
                return result;
            }
            else
            {
                SLANG_UNEXPECTED("conformance requirement did not produce a subtype witness");
            }
        }
        else if (auto inheritanceDeclRef = declaredConformanceDeclRef.as<InheritanceDecl>())
        {
            witnessTable = inheritanceDeclRef.getDecl()->witnessTable;
            if (!witnessTable)
            {
                auto parentDecl = as<ContainerDecl>(inheritanceDeclRef.getDecl()->parentDecl);
                if (parentDecl && !as<InterfaceDecl>(parentDecl) && !as<AssocTypeDecl>(parentDecl))
                {
                    result.frontier = RequirementWitnessLookupFrontier::makeMissingConcreteTable(
                        inheritanceDeclRef);
                }
                return result;
            }
        }
        else if (
            auto constraintDeclRef = declaredConformanceDeclRef.as<GenericTypeConstraintDecl>())
        {
            // Generic type constraints use a table that stores canonical paths through diamond
            // conformances. This passive operation cannot create that table if it is absent.
            witnessTable = constraintDeclRef.getDecl()->pathResolutionTable;
        }

        if (witnessTable)
        {
            result = _locateRequirementEntryInTable(witnessTable, requirementDeclRef);
            if (result.frontier.getStatus() == RequirementWitnessLookupFrontierStatus::Found)
            {
                result.specializationDeclRefs = _Move(prefixSpecializations);
                result.specializationDeclRefs.add(declaredConformanceDeclRef);
            }
        }
    }
    else if (auto transitiveWitness = as<TransitiveSubtypeWitness>(subtypeWitness))
    {
        if (auto midToSupWitness = as<DeclaredSubtypeWitness>(transitiveWitness->getMidToSup()))
        {
            auto midRequirementDeclRef = midToSupWitness->getDeclRef();
            auto midResult = _locateNextRequirementWitnessLookupFrontierRec(
                astBuilder,
                as<SubtypeWitness>(transitiveWitness->getSubToMid()),
                midRequirementDeclRef);
            if (midResult.frontier.getStatus() != RequirementWitnessLookupFrontierStatus::Found)
                return midResult;

            auto midWitness = midResult.frontier.getWitness();
            if (midWitness.getFlavor() == RequirementWitness::Flavor::witnessTable)
            {
                result = _locateRequirementEntryInTable(
                    midWitness.getWitnessTable(),
                    requirementDeclRef);
                if (result.frontier.getStatus() == RequirementWitnessLookupFrontierStatus::Found)
                {
                    result.specializationDeclRefs = _Move(midResult.specializationDeclRefs);
                    result.specializationDeclRefs.add(midRequirementDeclRef);
                }
            }
        }
    }

    return result;
}

RequirementWitnessLookupFrontier locateNextRequirementWitnessLookupFrontier(
    ASTBuilder* astBuilder,
    SubtypeWitness* subtypeWitness,
    DeclRef<Decl> requirementDeclRef)
{
    if (!subtypeWitness || !requirementDeclRef)
        return RequirementWitnessLookupFrontier();

    auto result = _locateNextRequirementWitnessLookupFrontierRec(
        astBuilder,
        subtypeWitness,
        requirementDeclRef);

    // These two entries are structural properties of the original subtype witness. Preserve the
    // historical lookup precedence by using a table entry when present and synthesizing the value
    // only after traversal fails. This fallback belongs at the public boundary: an intermediate
    // missing table on a nested witness path must not hide the original witness's `ThisType`.
    if (result.frontier.getStatus() != RequirementWitnessLookupFrontierStatus::Found)
    {
        if (as<ThisTypeDecl>(requirementDeclRef.getDecl()))
        {
            result = UnspecializedRequirementWitnessLookupFrontier();
            result.frontier = RequirementWitnessLookupFrontier::makeFound(
                RequirementWitness(subtypeWitness->getSub()));
        }
        else if (as<ThisTypeConstraintDecl>(requirementDeclRef.getDecl()))
        {
            result = UnspecializedRequirementWitnessLookupFrontier();
            result.frontier =
                RequirementWitnessLookupFrontier::makeFound(RequirementWitness(subtypeWitness));
        }
    }

    if (result.frontier.getStatus() == RequirementWitnessLookupFrontierStatus::Found)
    {
        // The recursive traversal records each declaration context on the way back out: first the
        // context that owns the selected table, then each context whose projected conformance led
        // to it. Replay that same inner-to-outer order so every subsequent substitution sees the
        // declaration-context result produced by the preceding step.
        auto specializedWitness = _specializeRequirementWitnessAlongLookupPath(
            astBuilder,
            result.frontier.getWitness(),
            result.specializationDeclRefs);
        result.frontier.setFoundWitness(specializedWitness);
    }
    return result.frontier;
}


Val* LookupDeclRef::tryResolve(SubtypeWitness* newWitness, Type* newLookupSource)
{
    auto astBuilder = getCurrentASTBuilder();
    Decl* requirementKey = getDecl();

    RequirementWitness lookedUpVal =
        tryLookUpRequirementWitness(astBuilder, newWitness, requirementKey);
    switch (lookedUpVal.getFlavor())
    {
    default:
        break;
    case RequirementWitness::Flavor::declRef:
        {
            return lookedUpVal.getDeclRef().declRefBase;
        }
    case RequirementWitness::Flavor::val:
        return lookedUpVal.getVal();
    }

    // If we didn't find anything using a simple lookup, we might need to handle some special-case
    // rules.

    // Hard code implementation of T.Differential.Differential == T.Differential rule.
    auto builtinReq = requirementKey->findModifier<BuiltinRequirementModifier>();
    bool isConstraint = false;
    if (!builtinReq)
    {
        // The requirement key is a constraint, not the associated type itself.
        // Determine which associated type the constraint constrains. This must
        // be answered from the constraint's endpoints, not from where the
        // sibling constraint happens to be declared.
        if (auto constraintDecl = as<GenericTypeConstraintDecl>(requirementKey))
        {
            // Look for the built-in requirement modifier on *either* endpoint of the
            // constraint. We search both sides for the modifier itself rather than
            // committing to the first side that happens to be an associated type: a
            // constraint such as `A == Differential` pairs a (non-built-in) assoc on
            // one side with the built-in `Differential` assoc on the other, and `==`
            // is symmetric, so `A == Differential` and `Differential == A` must
            // resolve identically.
            auto builtinReqFromExp = [](TypeExp const& exp) -> BuiltinRequirementModifier*
            {
                if (auto assoc = isDeclRefTypeOf<AssocTypeDecl>(exp.type))
                    return assoc.getDecl()->findModifier<BuiltinRequirementModifier>();
                return nullptr;
            };
            builtinReq = builtinReqFromExp(constraintDecl->sub);
            if (!builtinReq)
                builtinReq = builtinReqFromExp(constraintDecl->sup);
            if (builtinReq)
                isConstraint = true;
        }
        if (!builtinReq)
            return nullptr;
    }
    if (builtinReq->kind != BuiltinRequirementKind::DifferentialType &&
        builtinReq->kind != BuiltinRequirementKind::DifferentialPtrType)
        return nullptr;
    // Is the concrete type a Differential associated type?
    auto innerDeclRefType = as<DeclRefType>(newLookupSource);
    if (!innerDeclRefType)
        return nullptr;
    auto innerBuiltinReq =
        innerDeclRefType->getDeclRef().getDecl()->findModifier<BuiltinRequirementModifier>();
    if (!innerBuiltinReq)
        return nullptr;
    if (innerBuiltinReq->kind != BuiltinRequirementKind::DifferentialType &&
        innerBuiltinReq->kind != BuiltinRequirementKind::DifferentialPtrType)
        return nullptr;
    if (isConstraint)
        return newWitness;
    if (innerDeclRefType->getDeclRef() != this)
    {
        auto result = innerDeclRefType->getDeclRef().declRefBase->resolve();
        if (result)
            return result;
    }
    return innerDeclRefType;
}

DeclRefBase* GenericAppDeclRef::_substituteImplOverride(
    ASTBuilder* astBuilder,
    SubstitutionSet subst,
    int* ioDiff)
{
    int diff = 0;
    auto substGenericDeclRef = getGenericDeclRef()->substituteImpl(astBuilder, subst, &diff);
    List<Val*> substArgs;
    for (auto arg : getArgs())
    {
        substArgs.add(arg->substituteImpl(astBuilder, subst, &diff));
    }
    if (diff == 0)
        return this;
    (*ioDiff)++;

    if (getDecl()->isChildOf(substGenericDeclRef->getDecl()))
        return astBuilder->getGenericAppDeclRef(
            substGenericDeclRef,
            substArgs.getArrayView(),
            getDecl());
    else
    {
        // If decl is no longer the child of the new parent, it's most likely due to
        // the base lookup resolving to a different decl.
        //
        if (auto baseLookup = as<LookupDeclRef>(getGenericDeclRef()))
        {
            // Otherwise, we need to get the effective inner decl-ref for the generic app.
            auto resolvedTargetDecl = astBuilder
                                          ->getLookupDeclRef(
                                              baseLookup->getLookupSource(),
                                              baseLookup->getWitness(),
                                              getDecl())
                                          .substituteImpl(astBuilder, subst, &diff)
                                          .declRefBase->resolve();

            if (as<DeclRefBase>(resolvedTargetDecl))
            {
                return astBuilder->getGenericAppDeclRef(
                    substGenericDeclRef,
                    substArgs.getArrayView(),
                    as<DeclRefBase>(resolvedTargetDecl)->getDecl());
            }
        }

        SLANG_UNEXPECTED(
            "GenericAppDeclRef::substituteImpl: generic decl ref is not a child of the new parent "
            "& base is not a lookup");
    }
}

GenericDecl* GenericAppDeclRef::getGenericDecl()
{
    return as<GenericDecl>(getGenericDeclRef()->getDecl());
}


void GenericAppDeclRef::_toTextOverride(StringBuilder& out)
{
    auto genericDecl = as<GenericDecl>(getGenericDeclRef()->getDecl());
    Index paramCount = 0;
    for (auto member : genericDecl->getDirectMemberDecls())
        if (isGenericParam(member))
            paramCount++;
    getGenericDeclRef()->toText(out);
    out << "<";
    auto args = getArgs();
    Index argCount = args.getCount();
    for (Index aa = 0; aa < Math::Min(paramCount, argCount); ++aa)
    {
        if (aa != 0)
            out << ", ";
        args[aa]->toText(out);
    }
    out << ">";
}

Val* GenericAppDeclRef::_resolveImplOverride()
{
    auto astBuilder = getCurrentASTBuilder();
    Val* resolvedVal = this;
    auto resolvedGenericDeclRef = _resolveAsDeclRef(getGenericDeclRef());
    bool diff = false;
    if (resolvedGenericDeclRef != getGenericDeclRef())
        diff = true;
    List<Val*> resolvedArgs;
    for (auto arg : getArgs())
    {
        auto resolvedArg = arg->resolve();
        resolvedArgs.add(resolvedArg);
        if (resolvedArg != arg)
            diff = true;
    }
    if (diff)
    {
        if (getDecl()->isChildOf(resolvedGenericDeclRef->getDecl()))
        {
            resolvedVal = astBuilder->getGenericAppDeclRef(
                resolvedGenericDeclRef,
                resolvedArgs.getArrayView(),
                getDecl());
        }
        else if (getDecl() == getGenericDecl()->inner)
        {
            // Use the inner of the resolved generic decl ref.
            resolvedVal = astBuilder->getGenericAppDeclRef(
                resolvedGenericDeclRef,
                resolvedArgs.getArrayView());
        }
        else
        {
            // If we hit this case, we're referencing something that isn't
            // the direct child (->inner) of the generic decl.
            // There's no easy way to figure out which child of the new generic
            // we should be referencing, so we'll assert out here instead of
            // trying to continue with an ill-formed decl ref.
            //
            SLANG_ASSERT(
                "Cannot resolve generic app decl ref to a non-direct child of the resolved generic "
                "decl "
                "ref");
        }
    }
    return resolvedVal;
}

DeclRefBase* GenericAppDeclRef::_getBaseOverride()
{
    return getGenericDeclRef();
}

// Convenience accessors for common properties of declarations

DeclRefBase* DeclRefBase::substituteImpl(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff)
{
    return static_cast<DeclRefBase*>(substituteValWithCache(
        this,
        astBuilder,
        subst,
        ioDiff,
        [&](SubstitutionSet cachedSubst, int* cachedDiff) -> Val*
        {
            return ASTNodeDispatcher<DeclRefBase, DeclRefBase*>::dispatch(
                this,
                [&](auto declRef) -> DeclRefBase*
                { return declRef->_substituteImplOverride(astBuilder, cachedSubst, cachedDiff); });
        }));
}

DeclRefBase* DeclRefBase::getBase()
{
    SLANG_AST_NODE_VIRTUAL_CALL(DeclRefBase, getBase, ());
}
void DeclRefBase::toText(StringBuilder& out)
{
    if (auto lookupDeclRef = as<LookupDeclRef>(this))
    {
        lookupDeclRef->_toTextOverride(out);
        return;
    }

    if (as<GenericTypeParamDeclBase>(this->getDecl()))
    {
        SLANG_ASSERT(as<DirectDeclRef>(this));
        out << this->getDecl()->getName()->text;
        return;
    }
    else if (isGenericValueParam(this->getDecl()))
    {
        SLANG_ASSERT(as<DirectDeclRef>(this));
        out << this->getDecl()->getName()->text;
        return;
    }

    SubstitutionSet substSet(this);

    // Build a list of parent DeclRefs instead of just Decls
    List<DeclRefBase*> declRefs;

    for (DeclRefBase* dr = this; dr; dr = dr->getParent())
    {
        auto dd = dr->getDecl();

        // If this declaration is an extension, add it and then stop gathering parents
        if (as<ExtensionDecl>(dd))
        {
            declRefs.add(dr);
            break; // Stop gathering parent DeclRefs to exclude namespace
        }

        // Skip the module, file & include decls since their names are
        // considered "transparent"
        if (as<ModuleDecl>(dd) || as<FileDecl>(dd) || as<IncludeDecl>(dd))
            continue;

        // Skip base decls in generic containers. We will handle them when we handle the generic
        // decl.
        if (dd->parentDecl && as<GenericDecl>(dd->parentDecl))
            continue;

        declRefs.add(dr);
    }

    declRefs.reverse();

    bool first = true;
    for (auto declRef : declRefs)
    {
        auto decl = declRef->getDecl();
        if (!first)
            out << ".";
        first = false;

        if (auto name = decl->getName())
        {
            out << name->text;

            // If there are any specializations for this decl, emit them here:
            if (auto genericDecl = as<GenericDecl>(decl))
            {
                if (auto genericAppDeclRef = substSet.findGenericAppDeclRef(genericDecl))
                {
                    Index paramCount = 0;
                    for (auto member : genericDecl->getDirectMemberDecls())
                        if (isGenericParam(member))
                            paramCount++;
                    out << "<";
                    auto args = genericAppDeclRef->getArgs();
                    Index argCount = args.getCount();
                    for (Index aa = 0; aa < Math::Min(paramCount, argCount); ++aa)
                    {
                        if (aa != 0)
                            out << ", ";
                        args[aa]->toText(out);
                    }
                    out << ">";
                }
            }
        }
        else if (auto extDecl = as<ExtensionDecl>(decl))
        {
            if (extDecl->targetType)
            {
                getTargetType(getCurrentASTBuilder(), DeclRef(declRef).as<ExtensionDecl>())
                    ->toText(out);
            }
        }
    }
}

Name* DeclRefBase::getName() const
{
    return getDecl()->nameAndLoc.name;
}

SourceLoc DeclRefBase::getNameLoc() const
{
    return getDecl()->nameAndLoc.loc;
}

SourceLoc DeclRefBase::getLoc() const
{
    return getDecl()->loc;
}

// Keep this function here for better debuggin purpose
String DeclRefBase::toString() const
{
    StringBuilder sb;
    const_cast<DeclRefBase*>(this)->toText(sb);
    return sb.produceString();
}

DeclRefBase* DeclRefBase::getParent()
{
    auto astBuilder = getCurrentASTBuilder();
    if (!getDecl()->parentDecl)
        return nullptr;

    if (auto genericAppDeclRef = as<GenericAppDeclRef>(this))
    {
        auto parentDecl = getDecl()->parentDecl;
        auto genericDeclRef = genericAppDeclRef->getGenericDeclRef();
        auto genericDecl = genericDeclRef->getDecl();

        if (parentDecl != genericDecl && parentDecl->isChildOf(genericDecl))
        {
            // A generic application can name a declaration nested under the generic's inner
            // declaration, not only the inner declaration itself:
            //
            //     GenericAppDeclRef(Generic<T>, ..., inner = InnerNestedDecl)
            //
            // In that case, `getParent()` should preserve the same generic arguments while moving
            // to the lexical parent of `InnerNestedDecl`. This is not a replacement for the normal
            // `MemberDeclRef(GenericAppDeclRef(...), member)` projection form; that form is still
            // what represents an ordinary member selected from a specialized parent. This branch
            // only handles a decl-ref that is already a generic application to a nested `inner`
            // decl, so parent traversal remains consistent with the decl-ref's existing shape.
            return astBuilder->getGenericAppDeclRef(
                genericDeclRef,
                genericAppDeclRef->getArgs(),
                parentDecl);
        }
    }

    auto parentDecl = getDecl()->parentDecl;
    for (auto base = getBase(); base; base = base->getBase())
    {
        if (base->getDecl() == parentDecl)
            return base;
        bool parentIsChildOfBase = false;
        for (auto dd = parentDecl->parentDecl; dd; dd = dd->parentDecl)
        {
            if (dd == base->getDecl())
            {
                parentIsChildOfBase = true;
                break;
            }
        }
        if (parentIsChildOfBase)
            return astBuilder->getMemberDeclRef(base, parentDecl);
    }
    return astBuilder->getDirectDeclRef(parentDecl);
}

SubstitutionSet::operator bool() const
{
    return declRef != nullptr && !as<DirectDeclRef>(declRef);
}

Val::OperandView<Val> tryGetGenericArguments(SubstitutionSet substSet, Decl* genericDecl)
{
    if (!substSet.declRef)
        return Val::OperandView<Val>();

    DeclRefBase* currentDeclRef = substSet.declRef;
    // search for a substitution that might apply to us
    for (auto s = currentDeclRef; s; s = s->getBase())
    {
        auto genericAppDeclRef = as<GenericAppDeclRef>(s);
        if (!genericAppDeclRef)
            continue;

        // the generic decl associated with the substitution list must be
        // the generic decl that declared this parameter
        auto parentGeneric = genericAppDeclRef->getGenericDecl();
        if (parentGeneric != genericDecl)
            continue;

        return genericAppDeclRef->getArgs();
    }
    return Val::OperandView<Val>();
}

Type* SubstitutionSet::applyToType(ASTBuilder* astBuilder, Type* type) const
{
    if (!type)
        return nullptr;
    int diff = 0;
    auto newType = as<Type>(type->substituteImpl(astBuilder, *this, &diff));
    if (diff && newType)
        return newType;
    return type;
}

SubstExpr<Expr> applySubstitutionToExpr(SubstitutionSet substSet, Expr* expr)
{
    return SubstExpr<Expr>(expr, substSet);
}


DeclRefBase* SubstitutionSet::applyToDeclRef(ASTBuilder* astBuilder, DeclRefBase* otherDeclRef)
    const
{
    int diff = 0;
    return otherDeclRef->substituteImpl(astBuilder, *this, &diff);
}

LookupDeclRef* SubstitutionSet::findLookupDeclRef() const
{
    for (auto s = declRef; s; s = s->getBase())
    {
        if (auto lookupDeclRef = as<LookupDeclRef>(s))
            return lookupDeclRef;
    }
    return nullptr;
}

DeclRefBase* SubstitutionSet::getInnerMostNodeWithSubstInfo() const
{
    for (auto s = declRef; s; s = s->getBase())
    {
        if (as<LookupDeclRef>(s) || as<GenericAppDeclRef>(s))
            return s;
    }
    return nullptr;
}


GenericAppDeclRef* SubstitutionSet::findGenericAppDeclRef(GenericDecl* genericDecl) const
{
    for (auto s = declRef; s; s = s->getBase())
    {
        if (auto genApp = as<GenericAppDeclRef>(s))
        {
            if (genApp->getGenericDecl() == genericDecl)
                return genApp;
        }
    }
    return nullptr;
}

GenericAppDeclRef* SubstitutionSet::findGenericAppDeclRef() const
{
    for (auto s = declRef; s; s = s->getBase())
    {
        if (auto genApp = as<GenericAppDeclRef>(s))
        {
            return genApp;
        }
        else if (as<LookupDeclRef>(s))
        {
            return nullptr;
        }
    }
    return nullptr;
}

DeclRef<Decl> createDefaultSubstitutionsIfNeeded(
    ASTBuilder* astBuilder,
    SemanticsVisitor* semantics,
    DeclRef<Decl> declRef)
{
    if (isGenericParam(declRef))
        return declRef;

    ShortList<GenericDecl*> genericParentDecls;
    auto lastSubstNode = SubstitutionSet(declRef).getInnerMostNodeWithSubstInfo();
    auto lastGenApp = as<GenericAppDeclRef>(lastSubstNode);
    auto lastLookup = as<LookupDeclRef>(lastSubstNode);
    for (auto dd = declRef.getDecl()->parentDecl; dd; dd = dd->parentDecl)
    {
        if (lastGenApp && dd == lastGenApp->getGenericDecl())
            break;
        if (lastLookup && lastLookup->getDecl()->isChildOf(dd))
            break;
        if (isGenericConstraintParameterDecl(declRef.getDecl()) &&
            dd == declRef.getDecl()->parentDecl)
        {
            // A generic signature constraint is already represented as a witness argument of the
            // surrounding generic app, so do not add the immediate generic parent as another
            // default-substitution layer for the constraint decl itself. A standalone generic
            // interface requirement has shape `GenericDecl { inner = ConstraintDecl }` and is not a
            // generic constraint parameter; it must keep the generic parent so callers can form
            // `constraint<T, proofs...>` through this helper.
            continue;
        }
        if (auto gen = as<GenericDecl>(dd))
            genericParentDecls.add(gen);
    }
    DeclRef<Decl> parentDeclRef = lastSubstNode;
    for (auto i = genericParentDecls.getCount() - 1; i >= 0; i--)
    {
        auto current = genericParentDecls[i];
        auto args = getDefaultSubstitutionArgs(astBuilder, semantics, current);
        if (parentDeclRef)
        {
            // If the parent is a generic, we can skip directly to creating a generic app decl-ref.
            if (!parentDeclRef.as<GenericDecl>())
                parentDeclRef = astBuilder->getMemberDeclRef(parentDeclRef, current);
        }
        else
        {
            parentDeclRef = astBuilder->getDirectDeclRef(current);
        }

        parentDeclRef =
            astBuilder->getGenericAppDeclRef(parentDeclRef.as<GenericDecl>(), args.getArrayView());
    }
    if (!parentDeclRef)
        return declRef;
    if (parentDeclRef.getDecl() == declRef.getDecl())
        return parentDeclRef;
    return astBuilder->getMemberDeclRef(parentDeclRef, declRef.getDecl());
}
} // namespace Slang
