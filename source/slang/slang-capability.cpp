// slang-capability.cpp
#include "slang-capability.h"

#include "../core/slang-dictionary.h"

// This file implements the core of the "capability" system.

namespace Slang
{

//
// CapabilityAtom
//

// We are going to divide capability atoms into a few categories.
//
enum class CapabilityNameFlavor : int32_t
{
    // A concrete capability atom is something that a target
    // can directly support, where the presence of the feature
    // directly provides functionality. A specific OpenGL
    // or Vulkan extension would be an example of a concrete
    // capability.
    //
    Concrete,

    // An abstract capability represents a class of feature
    // where multiple different implementations might be possible.
    // For example, "ray tracing" might be an abstract feature
    // that a function can require, but a specific target will
    // only be able to provide that abstract feature via some
    // specific concrete feature (e.g., `GL_EXT_ray_tracing`).
    Abstract,

    // An alias capability atom is one that is exactly equivalent
    // to the things it inherits from.
    //
    // For example, a `ps_5_1` capability would just be an
    // alias for the combination of the `fragment` capability
    // and the `sm_5_1` capability.
    //
    Alias,
};

// The macros in the `slang-capability-defs.h` file will be used
// to fill out a `static const` array of information about each
// capability atom.
//
struct CapabilityAtomInfo
{
    /// The API-/language-exposed name of the capability.
    char const* name;

    /// Flavor of atom: concrete, abstract, or alias
    CapabilityNameFlavor        flavor;

    /// If the atom is a direct descendent of an abstract base, keep that for reference here.
    CapabilityName abstractBase;

    /// Ranking to use when deciding if this atom is a "better" one to select.
    uint32_t                    rank;

    /// Canonical representation in the form of disjunction-of-conjunction of atoms.
    ArrayView<ArrayView<CapabilityName>> canonicalRepresentation;
};

#include "slang-generated-capability-defs-impl.h"

static CapabilityAtom asAtom(CapabilityName name)
{
    SLANG_ASSERT((CapabilityAtom)name < CapabilityAtom::Count);
    return (CapabilityAtom)name;
}

/// Get the extended information structure for the given capability `atom`
static CapabilityAtomInfo const& _getInfo(CapabilityName atom)
{
    SLANG_ASSERT(Int(atom) < Int(CapabilityName::Count));
    return kCapabilityNameInfos[Int(atom)];
}
static CapabilityAtomInfo const& _getInfo(CapabilityAtom atom)
{
    SLANG_ASSERT(Int(atom) < Int(CapabilityAtom::Count));
    return kCapabilityNameInfos[Int(atom)];
}

void getCapabilityNames(List<UnownedStringSlice>& ioNames)
{
    ioNames.reserve(Count(CapabilityName::Count));
    for (Index i = 0; i < Count(CapabilityName::Count); ++i)
    {
        if (_getInfo(CapabilityName(i)).flavor != CapabilityNameFlavor::Abstract)
        {
            ioNames.add(UnownedStringSlice(_getInfo(CapabilityName(i)).name));
        }
    }
}

UnownedStringSlice capabilityNameToString(CapabilityName name)
{
    return UnownedStringSlice(_getInfo(name).name);
}

bool isDirectChildOfAbstractAtom(CapabilityAtom name)
{
    return _getInfo(name).abstractBase != CapabilityName::Invalid;
}

bool lookupCapabilityName(const UnownedStringSlice& str, CapabilityName& value);

CapabilityName findCapabilityName(UnownedStringSlice const& name)
{
    CapabilityName result;
    if (!lookupCapabilityName(name, result))
        return CapabilityName::Invalid;
    return result;
}

bool isCapabilityDerivedFrom(CapabilityAtom atom, CapabilityAtom base)
{
    if (atom == base)
    {
        return true;
    }

    const auto& info = kCapabilityNameInfos[Index(atom)];
    for (auto cur : info.canonicalRepresentation)
    {
        for (auto cbase : cur)
            if (asAtom(cbase) == base)
                return true;
    }

    return false;
}

//
// CapabilityConjunctionSet
//

// The current design choice in `CapabilityConjunctionSet` is that it stores
// an expanded, deduplicated, and sorted list of the capability
// atoms in the set. "Expanded" here means that it includes the
// transitive closure of the inheritance graph of those atoms.
//
// This choice is intended to make certain operations on
// capability sets more efficient, since use things like
// binary searches to efficiently detect whether an atom
// is present in a set.

CapabilityConjunctionSet::CapabilityConjunctionSet()
{}

CapabilityConjunctionSet::CapabilityConjunctionSet(Int atomCount, CapabilityAtom const* atoms)
{
    _init(atomCount, atoms);
}

CapabilityConjunctionSet::CapabilityConjunctionSet(CapabilityAtom atom)
{
    _init(1, &atom);
}

CapabilityConjunctionSet::CapabilityConjunctionSet(List<CapabilityAtom> const& atoms)
{
    _init(atoms.getCount(), atoms.getBuffer());
}

CapabilityConjunctionSet CapabilityConjunctionSet::makeEmpty()
{
    return CapabilityConjunctionSet();
}

CapabilityConjunctionSet CapabilityConjunctionSet::makeInvalid()
{
    // An invalid capability set will always be a singleton
    // set of the `Invalid` atom, and we will construct
    // the set directly rather than use the more expensive
    // logic in `_init()`.
    //
    CapabilityConjunctionSet result;
    result.m_expandedAtoms.add(CapabilityAtom::Invalid);
    return result;
}

void CapabilityConjunctionSet::_init(Int atomCount, CapabilityAtom const* atoms)
{
    // We will use an explicit hash set to deduplicate input atoms.
    //
    HashSet<CapabilityAtom> expandedAtomsSet;
    for(Int i = 0; i < atomCount; ++i)
    {
        if (expandedAtomsSet.add(atoms[i]))
        {
            auto& info = _getInfo(atoms[i]);

            // Add the base items that this atom implies.
            if (info.canonicalRepresentation.getCount() == 1)
            {
                // The atom must have only one conjunction.
                SLANG_ASSERT(info.canonicalRepresentation.getCount() == 1);

                for (auto base : info.canonicalRepresentation[0])
                {
                    expandedAtomsSet.add(asAtom(base));
                }
            }
        }
    }

    // We can then translate the set of atoms into a list,
    // and then sort that list to produce the data that
    // we use in all our other queries.
    //
    for(auto atom : expandedAtomsSet)
    {
        m_expandedAtoms.add(atom);
    }
    m_expandedAtoms.sort();
}

void CapabilityConjunctionSet::calcCompactedAtoms(List<CapabilityAtom>& outAtoms) const
{
    // A "compacted" list of atoms is one that starts with
    // the "expanded" list and removes any atoms that are
    // implied by another atom already in the list.
    //
    // If the expanded list contains atom A, and A inherits
    // from B, then we know that the expanded list also contains B,
    // but the compacted list should not.
    //
    // We can thus look through the list of atoms A and for
    // each base B of A, add it to a set of "redundant" atoms
    // that need not appear in the compacted list.
    //
    HashSet<CapabilityAtom> redundantAtomsSet;
    for( auto atom : m_expandedAtoms )
    {
        auto& atomInfo = _getInfo(atom);
        if (atomInfo.canonicalRepresentation.getCount() != 1)
        {
            // If the atom is not a single conjunction, skip.
            continue;
        }
        for(auto baseAtom : atomInfo.canonicalRepresentation[0])
        {
            // Note: don't add atom itself into redundant set.
            if(asAtom(baseAtom) == atom)
                continue;

            redundantAtomsSet.add(asAtom(baseAtom));
        }
    }

    // Once we are done figuring out which atoms are redundant,
    // we can iterate over the expanded list and add all the
    // non-redundant ones to the compacted output list.
    //
    outAtoms.clear();
    for( auto atom : m_expandedAtoms )
    {
        if(!redundantAtomsSet.contains(atom))
        {
            outAtoms.add(atom);
        }
    }
}

bool CapabilityConjunctionSet::isEmpty() const
{
    // Checking if a capability set is empty is trivial in any representation;
    // all we need to know is if it has zero atoms in its definition.
    //
    return m_expandedAtoms.getCount() == 0;
}

bool CapabilityConjunctionSet::isInvalid() const
{
    // We will assume here that there is only one canonical representation of
    // an invalid capability set, which is a singleton set of the `Invalid`
    // atom.
    //
    // TODO: We should ensure that any algorithms that make new capability
    // sets by combining others properly ensure that they return the
    // canonical invalid set rather than any other set that happens to be
    // invalid (e.g., a set {A,B} would be invalid if A and B are incompatible,
    // but it would not be in the canonical form this subroutine checks).
    //
    if(m_expandedAtoms.getCount() != 1) return false;
    return m_expandedAtoms[0] == CapabilityAtom::Invalid;
}

bool CapabilityConjunctionSet::isIncompatibleWith(CapabilityAtom that) const
{
    // Checking for incompatibility is complicated, and it is best
    // to only implement it for full (expanded) sets.
    //
    return isIncompatibleWith(CapabilityConjunctionSet(that));
}

static UIntSet _calcConflictMask(CapabilityAtom atom)
{
    UIntSet mask;
    auto abstractBase = _getInfo(atom).abstractBase;
    if (abstractBase != CapabilityName::Invalid)
    {
        mask.add((UInt)abstractBase);
    }
    return mask;
}

static UIntSet _calcConflictMask(const CapabilityConjunctionSet& set)
{
    // Given a capbility set, we want to compute the mask representing
    // all groups of features for which it holds a potentially-conflicting atom.
    //
    UIntSet mask;
    for (auto atom : set.getExpandedAtoms())
    {
        auto abstractBase = _getInfo(atom).abstractBase;
        if (abstractBase != CapabilityName::Invalid)
        {
            mask.add((UInt)abstractBase);
        }
    }
    return mask;
}

bool CapabilityConjunctionSet::isIncompatibleWith(CapabilityConjunctionSet const& that) const
{
    // The `this` and `that` sets are incompatible if there exists
    // an atom A in `this` and an atom `B` in `that` such that
    // A and B are not equal, but the two have overlapping "conflict group."
    //
    // Equivalently, we can say that the two are in conflict if
    //
    // * One of the two sets contains an atom A with conflict mask M
    // * The other set contains at least one atom that conflicts with M
    // * The other set does not contain A
    //
    // Our approach here is all about minimizing the number of
    // iterations we take over lists of atoms, and trying to
    // avoid anything super-linear.

    // We start by identifying the OR of the conflict masks for
    // all features in `this` and `that`.
    //
    UIntSet thisMask = _calcConflictMask(*this);
    UIntSet thatMask = _calcConflictMask(that);

    // Note: there is a possible early-exit opportunity here if
    // `thisMask` and `thatMask` have no overlap: there could
    // be no conflicts in that case.

    // Next we will iterate over the two sets in tandem (O(N) time
    // in the size of the larger set), and identify any elements
    // that are present in one and not the other.
    //
    Index thisCount = this->m_expandedAtoms.getCount();
    Index thatCount = that.m_expandedAtoms.getCount();
    Index thisIndex = 0;
    Index thatIndex = 0;
    for(;;)
    {
        if(thisIndex == thisCount) break;
        if(thatIndex == thatCount) break;

        auto thisAtom = this->m_expandedAtoms[thisIndex];
        auto thatAtom = that.m_expandedAtoms[thatIndex];

        if(thisAtom == thatAtom)
        {
            thisIndex++;
            thatIndex++;
            continue;
        }

        if( thisAtom < thatAtom )
        {
            // `thisAtom` is present in `this` but not `that.
            //
            // If `thisAtom` has a conflict mask that overlaps
            // with `thatMask`, then we have a conflict: the
            // other set doesn't include `thisAtom`, but *does*
            // include something with an overlapping mask
            // (we don't know what at this point in the code).
            //
            auto thisConflictMask = Slang::_calcConflictMask(thisAtom);
            if(UIntSet::hasIntersection(thisConflictMask, thatMask))
                return true;
            thisIndex++;
        }
        else
        {
            SLANG_ASSERT(thisAtom > thatAtom);

            // `thatAtom` is present in `that` but not `this.
            //
            // The logic here is the mirror image of the case above.
            //
            auto thatConflictMask = Slang::_calcConflictMask(thatAtom);
            if(UIntSet::hasIntersection(thatConflictMask, thisMask))
                return true;
            thatIndex++;
        }
    }

    return false;
}

bool CapabilityConjunctionSet::implies(CapabilityConjunctionSet const& that) const
{
    // One capability set implies another if it is a super-set
    // of the other one. Think of it this way: if your target
    // supports features {X, Y, Z}, then that implies it also
    // supports features {X,Z}.
    //
    // Because both `this` and `that` have expanded lists
    // of all the capability atoms they imply *and* those
    // lists are sorted, we can simply walk through the
    // lists in tandem and see if there are any entries
    // in `that` which are not present in `this.

    Index thisCount = this->m_expandedAtoms.getCount();
    Index thatCount = that.m_expandedAtoms.getCount();

    // We cannot possibly have `this` contain all the atoms
    // in `that` if the latter is has more atoms.
    //
    if(thatCount > thisCount)
        return false;

    // Note: the following iteration is O(N) in the size
    // of the larger of the two sets, which is probably
    // needlessly inefficient. We might expect that `that`
    // will often be a much smaller set, and we'd like to
    // scale in its size rather than the size of `this`.
    //
    // A more advanced algorithm here would be to do
    // something recursive:
    //
    // * If `that` is  singleton set, then we can find
    //   whether `this` contains it via binary search.
    //
    // * Otherwise, we can split `that` into two
    //   equally-sized subsets. By taking a "pivot" value
    //   from where that split took place we can then
    //   use a binary search to partition `this` into
    //   two subsets and recurse on each side of that
    //   partition.
    //
    // In practice, the size of the sets we are dealing
    // with right now doesn't justify such a "clever" algorithm.

    Index thisIndex = 0;
    Index thatIndex = 0;
    for(;;)
    {
        if(thisIndex == thisCount) break;
        if(thatIndex == thatCount) break;

        auto thisAtom = this->m_expandedAtoms[thisIndex];
        auto thatAtom = that.m_expandedAtoms[thatIndex];

        if( thisAtom == thatAtom )
        {
            // We have an atom that both sets contain;
            // we should skip past it and keep looking.
            //
            thisIndex++;
            thatIndex++;
            continue;
        }

        if( thisAtom < thatAtom )
        {
            // We have an atom that `this` contains,
            // but `that` doesn't; that is consistent
            // with `this` being a super-set, so we
            // just skip the item and keep searching.
            //
            thisIndex++;
        }
        else
        {
            SLANG_ASSERT(thisAtom > thatAtom);

            // We have an atom in `that` which isn't
            // also in `this`, so we know it cannot
            // be a subset.
            //
            return false;
        }
    }
    // We reached the end of either this or that atom.
    // If we reached the end of 'that', we know everything in 'that'
    // is also contained in this, so this implies that.
    return thatIndex == thatCount;
}

    /// Helper functor for binary search on lists of `CapabilityAtom`
struct CapabilityAtomComparator
{
    int operator()(CapabilityAtom left, CapabilityAtom right)
    {
        return int(Int(left) - Int(right));
    }
};

bool CapabilityConjunctionSet::implies(CapabilityAtom atom) const
{
    // Every non-alias atom that `this` implies should
    // be presented in the `m_expandedAtoms` list.
    //
    // Because the list is sorted, we can find out whether
    // it contains `atom` with a binary search.
    //
    Index result = m_expandedAtoms.binarySearch(atom, CapabilityAtomComparator());
    return result >= 0;
}

Int CapabilityConjunctionSet::countIntersectionWith(CapabilityConjunctionSet const& that) const
{
    // The goal of this subroutine is to count the number of
    // elements in the intersection of `this` and `that`,
    // without explicitly forming that intersection.
    //
    // Our approach here will be to iterate over the two
    // sets in tandem (O(N) in the size of the larger set)
    // and check for elements that both contain.
    //
    // TODO: There should be an asymptotically faster
    // recursive algorithm here.

    Int intersectionCount = 0;

    Index thisCount = this->m_expandedAtoms.getCount();
    Index thatCount = that.m_expandedAtoms.getCount();
    Index thisIndex = 0;
    Index thatIndex = 0;
    for(;;)
    {
        if(thisIndex == thisCount) break;
        if(thatIndex == thatCount) break;

        auto thisAtom = this->m_expandedAtoms[thisIndex];
        auto thatAtom = that.m_expandedAtoms[thatIndex];

        if( thisAtom == thatAtom )
        {
            // An item both contain.

            intersectionCount++;
            thisIndex++;
            thatIndex++;
            continue;
        }

        if( thisAtom < thatAtom )
        {
            // An item in `this` but not `that`.

            thisIndex++;
        }
        else
        {
            SLANG_ASSERT(thisAtom > thatAtom);

            // An item in `that` but not `this`.

            thatIndex++;
        }
    }
    return intersectionCount;
}

bool CapabilityConjunctionSet::isBetterForTarget(
    CapabilityConjunctionSet const& existingCaps,
    CapabilityConjunctionSet const& targetCaps) const
{
    auto& candidateCaps = *this;

    // The task here is to determine if `candidateCaps` should
    // be considered "better" than `existingCaps` in the context
    // of compilation for a target with the given `targetCaps`.
    //
    // In an ideal world, this computation could be quite simple:
    //
    // * If either `candidateCaps` or `existingCaps` is not implied by
    //   `targetCaps` (that is, they include requirements that aren't
    //   provided by the target), then the other is automatically "better."
    //
    // * Otherwise, one set is "better" than the other if it is a
    //   super-set (which is what `implies()` tests).
    //
    // There are two main reasons we can't use that simple logic:
    //
    // 1. Currently a user of Slang can compile for a target but
    //    not actually spell out its capabilities fully or correctly.
    //    They might compile for `sm_5_0` but use ray tracing features
    //    that require `sm_6_2` and expect the compiler to figure out
    //    what they "obviously" meant. Thus we cannot assume that
    //    `targetCaps` can be used to rule out candidates fully.
    //
    // 2. Sometimes there are multiple ways for a target to provide
    //    the same feature (e.g., multiple extensions) and because of (1)
    //    we cannot always rely on the `targetCaps` to tell us which to
    //    use. Thus we cannot rely on pure subset/`implies()` to define
    //    better-ness, and need some way to break ties.
    //
    // The following logic is a bunch of "do what I mean" nonsense that
    // tries to capture a reasonable intuition of what "better"-ness
    // should mean with these caveats.

    // First, if either candidate is fundamentally incompatible
    // with the target, we shouldn't favor it.
    //
    if(candidateCaps.isIncompatibleWith(targetCaps)) return false;
    if(existingCaps.isIncompatibleWith(targetCaps)) return true;

    // Next, we want to compare the candidates to the `targetCaps`
    // to figure out whether one is obviously "more specialized" for
    // the target.
    //
    // We measure the degree to which a candidate is specialized for
    // the target as the size of its set intersection with `targetCaps`.
    //
    // TODO: If both `candidateCaps` and `existingCaps` are implied
    // by `targetCaps`, then this amounts to just measuring the
    // size of each set. We probably want this size-based check to
    // come later in the overall process.
    //
    // TODO: A better model here might be to actually compute the actual
    // intersected sets, and then check if one is a super-set of the other.
    //
    auto candidateIntersectionSize = targetCaps.countIntersectionWith(candidateCaps);
    auto existingIntersectionSize = targetCaps.countIntersectionWith(existingCaps);
    if(candidateIntersectionSize != existingIntersectionSize)
        return candidateIntersectionSize > existingIntersectionSize;

    // Next we want to consider that if one of the two candidates
    // is actually available on the target (meaning that it is
    // implied by `targetCaps`) then we probably want to pick that one
    // (since we can use that candidate on the chosen target without
    // enabling any additional features the user didn't ask for).
    //
    // TODO: This step currently needs to come after the preceeding
    // one because otherwise we risk selecting a `__target_intrinsic`
    // decoration with *no* requirements (which are currently being
    // added implicitly in many places) over any one with explicit
    // requirements (since every target implies the empty set of
    // requirements).
    //
    // In many ways the counting-based logic above amounts to a quick
    // fix to prefer a non-empty set of requirements over an empty one,
    // so long as something in that non-empty set overlaps with the target.
    //
    // TODO: The best fix is probably to figure out how "catch-all"
    // intrinsic function definitions should be encoded; we clearly
    // want them to be used only as a fallback when no target-specific
    // variants are present.
    //
    bool candidateIsAvailable = targetCaps.implies(candidateCaps);
    bool existingIsAvailable = targetCaps.implies(existingCaps);
    if(candidateIsAvailable != existingIsAvailable)
        return candidateIsAvailable;

    // All preceding factors being equal, we prefer
    // a candidate that is strictly more specialized than the other.
    //
    // We want to avoid choosing the candidate that uses
    // optional features if they aren't necessary.
    // For example, the set {glsl, optionalFeature} should not be preferred
    // over the set {glsl}, if optionalFeature isn't requested explictly.
    //
    // The solution here is that we want to partition
    // `candidateCaps` and `existingCaps` into two parts: their
    // intersection with `targetCaps` and their difference with it.
    //
    // For the intersection part of things, we'd want to favor a
    // definition that is more specialized, while for the difference
    // part we'd actually wnat to favor a definition that is less
    // specialized.
    //
    CapabilityConjunctionSet candidateCapsIntersection;
    CapabilityConjunctionSet candidateCapsDifference;
    for (auto atom : candidateCaps.m_expandedAtoms)
    {
        if (targetCaps.implies(atom))
            candidateCapsIntersection.m_expandedAtoms.add(atom);
        else
            candidateCapsDifference.m_expandedAtoms.add(atom);
    }
    CapabilityConjunctionSet existingCapsIntersection;
    CapabilityConjunctionSet existingCapsDifference;
    for (auto atom : existingCaps.m_expandedAtoms)
    {
        if (targetCaps.implies(atom))
            existingCapsIntersection.m_expandedAtoms.add(atom);
        else
            existingCapsDifference.m_expandedAtoms.add(atom);
    }
    auto scoreCandidate = candidateCapsIntersection.m_expandedAtoms.getCount() - candidateCapsDifference.m_expandedAtoms.getCount();
    auto scoreExisting = existingCapsIntersection.m_expandedAtoms.getCount() - existingCapsDifference.m_expandedAtoms.getCount();
    if (scoreCandidate != scoreExisting)
        return scoreCandidate > scoreExisting;

    // At this point we have the problem that neither candidate
    // appears to be "obviously" better for the target, but we
    // want some way to disambiguate them.
    //
    // What we want to do now is scan through what makes each candidate
    // different from the other, and see if anything in either case
    // has a ranking that should make it be preferred.
    //
    auto candidateScore = candidateCapsDifference._calcDifferenceScoreWith(existingCapsDifference);
    auto existingScore = existingCapsDifference._calcDifferenceScoreWith(candidateCapsDifference);
    if(candidateScore != existingScore)
        return candidateScore > existingScore;

    return false;
}

uint32_t CapabilityConjunctionSet::_calcDifferenceScoreWith(CapabilityConjunctionSet const& that) const
{
    uint32_t score = 0;

    // Our approach here will be to scan through `this` and `that`
    // to identify atoms that are in `this` but not `that` (that is,
    // the atoms that would be present in the set difference `this - that`)
    // and then compute the maximum rank/score of those atoms.

    Index thisCount = this->m_expandedAtoms.getCount();
    Index thatCount = that.m_expandedAtoms.getCount();
    Index thisIndex = 0;
    Index thatIndex = 0;
    for(;;)
    {
        if(thisIndex == thisCount) break;
        if(thatIndex == thatCount) break;

        auto thisAtom = this->m_expandedAtoms[thisIndex];
        auto thatAtom = that.m_expandedAtoms[thatIndex];

        if( thisAtom == thatAtom )
        {
            thisIndex++;
            thatIndex++;
            continue;
        }

        if( thisAtom < thatAtom )
        {
            // `thisAtom` is not present in `that`, so it
            // should contribute to our ranking of the difference.
            //
            auto thisAtomInfo = _getInfo(thisAtom);
            auto thisAtomRank = thisAtomInfo.rank;

            if( thisAtomRank > score )
            {
                score = thisAtomRank;
            }

            thisIndex++;
        }
        else
        {
            SLANG_ASSERT(thisAtom > thatAtom);
            thatIndex++;
        }
    }
    return score;
}


bool CapabilityConjunctionSet::operator==(CapabilityConjunctionSet const& other) const
{
    return m_expandedAtoms == other.m_expandedAtoms;
}

bool CapabilityConjunctionSet::operator<(CapabilityConjunctionSet const& that) const
{
    for (Index i = 0; i < Math::Min(m_expandedAtoms.getCount(), that.m_expandedAtoms.getCount()); i++)
    {
        if (m_expandedAtoms[i] < that.m_expandedAtoms[i])
            return true;
        else if (m_expandedAtoms[i] > that.m_expandedAtoms[i])
            return false;
    }
    return m_expandedAtoms.getCount() < that.m_expandedAtoms.getCount();
}


CapabilitySet::CapabilitySet()
{}

CapabilitySet::CapabilitySet(Int atomCount, CapabilityName const* atoms)
{
    for (Int i = 0; i < atomCount; i++)
        addCapability(atoms[i]);
}

CapabilitySet::CapabilitySet(CapabilityName atom)
{
    auto info = _getInfo(atom);
    for (auto conjunction : info.canonicalRepresentation)
    {
        CapabilityConjunctionSet set;
        for (auto atomName : conjunction)
            set.getExpandedAtoms().add(asAtom(atomName));
        m_conjunctions.add(_Move(set));
    }
}

CapabilitySet::CapabilitySet(List<CapabilityName> const& atoms)
{
    for (auto atom : atoms)
        addCapability(atom);
}

CapabilitySet CapabilitySet::makeEmpty()
{
    return CapabilitySet();
}

CapabilitySet CapabilitySet::makeInvalid()
{
    // An invalid capability set will always be a singleton
    // set of the `Invalid` atom, and we will construct
    // the set directly rather than use the more expensive
    // logic in `_init()`.
    //
    CapabilitySet result;
    result.m_conjunctions.add(CapabilityConjunctionSet(CapabilityAtom::Invalid));
    return result;
}

void CapabilitySet::addCapability(CapabilityName name)
{
    join(CapabilitySet(name));
}

bool CapabilitySet::isEmpty() const
{
    return m_conjunctions.getCount() == 0;
}

bool CapabilitySet::isInvalid() const
{
    return m_conjunctions.getCount() == 1 && m_conjunctions[0].isInvalid();
}

bool CapabilitySet::isIncompatibleWith(CapabilityAtom other) const
{
    if (isEmpty())
        return false;

    // If all conjunctions are incompatible with the atom, then we are incompatible.
    for (auto& c : m_conjunctions)
        if (!c.isIncompatibleWith(other))
            return false;
    return true;
}

bool CapabilitySet::isIncompatibleWith(CapabilityName other) const
{
    if (isEmpty())
        return false;
    auto otherSet = CapabilitySet(other);
    return isIncompatibleWith(otherSet);
}

bool CapabilitySet::isIncompatibleWith(CapabilityConjunctionSet const& other) const
{
    if (isEmpty())
        return false;

    // If all conjunctions are incompatible with the atom, then we are incompatible.
    for (auto& c : m_conjunctions)
        if (!c.isIncompatibleWith(other))
            return false;
    return true;
}

bool CapabilitySet::isIncompatibleWith(CapabilitySet const& other) const
{
    if (isEmpty())
        return false;
    if (other.isEmpty())
        return false;

    // If all conjunctions in other are incompatible with the this set, then we are incompatible.
    for (auto& oc : other.m_conjunctions)
        for (auto& c : m_conjunctions)
            if (!c.isIncompatibleWith(oc))
                return false;
    return true;
}

bool CapabilitySet::implies(CapabilityAtom atom) const
{
    if (isEmpty())
        return false;

    for (auto& c : m_conjunctions)
        if (c.implies(atom))
            return true;

    return false;
}

bool CapabilitySet::implies(const CapabilityConjunctionSet& set) const
{
    if (isEmpty())
        return false;

    for (auto& c : m_conjunctions)
        if (c.implies(set))
            return true;

    return false;
}

bool CapabilitySet::implies(CapabilitySet const& other) const
{
    // x implies (c | d) only if (x implies c) and (x implies d).
    if (other.isEmpty())
        return true;
    for (auto& c : other.m_conjunctions)
        if (!implies(c))
            return false;
    return true;
}

bool CapabilitySet::operator==(CapabilitySet const& that) const
{
    return m_conjunctions == that.m_conjunctions;
}

void CapabilitySet::calcCompactedAtoms(List<List<CapabilityAtom>>& outAtoms) const
{
    for (auto& c : m_conjunctions)
    {
        List<CapabilityAtom> atoms;
        c.calcCompactedAtoms(atoms);
        outAtoms.add(atoms);
    }
}

void CapabilitySet::unionWith(const CapabilityConjunctionSet& conjunctionToAdd)
{
    // We add conjunctionToAdd to resultSet only if it does not imply any existing conjunctions.
    // For example, if `resultSet` is (a), and conjunctionToAdd is (ab), then we don't want to add the conjunction
    // to form (a | ab) because that would reduce to (a).
    bool skipAdd = false;
    for (auto& c : m_conjunctions)
    {
        if (conjunctionToAdd.implies(c))
        {
            skipAdd = true;
            break;
        }
    }
    if (!skipAdd)
    {
        // Once we added the new conjunction, any existing conjunctions that implies the new one can be
        // removed.
        // For example, if resultSet was (ab), and we are adding (a), the result should be just (a).
        for (Index i = 0; i < m_conjunctions.getCount();)
        {
            if (m_conjunctions[i].implies(conjunctionToAdd))
            {
                m_conjunctions.fastRemoveAt(i);
            }
            else
            {
                i++;
            }
        }
        m_conjunctions.add(conjunctionToAdd);
    }
}

void CapabilitySet::canonicalize()
{
    // Make sure conjunctions are sorted so equality tests are trivial.
    m_conjunctions.sort();
}

void CapabilitySet::join(const CapabilitySet& other)
{
    if (isEmpty() || other.isInvalid())
    {
        *this = other;
        return;
    }
    if (isInvalid())
        return;
    if (other.isEmpty())
        return;

    CapabilitySet resultSet;
    for (auto& thatConjunction : other.m_conjunctions)
    {
        for (auto& thisConjunction : m_conjunctions)
        {
            if (thisConjunction.isIncompatibleWith(thatConjunction))
                continue;

            CapabilityConjunctionSet conjunction;
            CapabilityConjunctionSet *conjunctionToAdd = nullptr;

            // Add atoms from thatConjunction that are not existant in thisConjunction.
            for (auto atom : thatConjunction.getExpandedAtoms())
            {
                if (thisConjunction.getExpandedAtoms().binarySearch(atom, CapabilityAtomComparator()) == -1)
                {
                    conjunction.getExpandedAtoms().add(atom);
                }
            }

            if (conjunction.getExpandedAtoms().getCount() != 0)
            {
                // If we find any capabilities in thatConjunction that is missing from thisConjunction,
                // create a new ConjunctionSet that contains atoms from both, and add it to the disjunction set.
                conjunction.getExpandedAtoms().addRange(thisConjunction.getExpandedAtoms());
                conjunction.getExpandedAtoms().sort();
                conjunctionToAdd = &conjunction;
            }
            else
            {
                // Otherwise, thisConjunction implies thatConjunction, so we just add thisConjunction to resultSet.
                conjunctionToAdd = &thisConjunction;
            }
            resultSet.unionWith(*conjunctionToAdd);
        }
    }
    m_conjunctions = _Move(resultSet.m_conjunctions);

    if (m_conjunctions.getCount() == 0)
    {
        // If the result is empty, then we should return as impossible.
        *this = CapabilitySet::makeInvalid();
    }
    else
    {
        canonicalize();
    }
}

bool CapabilitySet::isBetterForTarget(CapabilitySet const& that, CapabilitySet const& targetCaps) const
{
    if (targetCaps.isIncompatibleWith(*this))
        return false;
    if (targetCaps.isIncompatibleWith(that))
        return true;

    ArrayView<CapabilityConjunctionSet> thisSets = m_conjunctions.getArrayView();
    ArrayView<CapabilityConjunctionSet> thatSets = that.m_conjunctions.getArrayView();
    CapabilityConjunctionSet emtpySet = CapabilityConjunctionSet::makeEmpty();

    if (isEmpty())
        thisSets = makeArrayViewSingle(emtpySet);
    if (that.isEmpty())
        thatSets = makeArrayViewSingle(emtpySet);

    // It is hard to think about what it means exactly to compare a general disjunction set to another with regard
    // to a target that itself is also a disjunction set.
    // Instead of trying to find a meaning for the general case, we just want to extend the logic
    // for conjunction sets to disjunction sets in a way that common situations are handled correctly.
    // Note that when we reach here, most of these sets are likely to contain only one conjunction, so
    // we just need to make sure the more general logic here yields correct result for that case.
    // 
    // Right now, we define betterness for disjunctions as follows:
    // A capability set X is determined to be better for a target T than capability set Y,
    // if we find a conjunction A in X and a conjunction B in Y and a conjunction C in T such that
    // A is better then B for target C.
    //
    struct ViableConjunctionIndex
    {
        Index index;
        UIntSet targetConjunctionIndices;
    };
    auto getViableConjunction = [&](ArrayView<CapabilityConjunctionSet> set, List<ViableConjunctionIndex>& outList)
        {
            for (Index i = 0; i < set.getCount(); i++)
            {
                auto& conjunction = set[i];
                ViableConjunctionIndex viableConjunction;
                viableConjunction.index = i;
                for (Index j = 0; j < targetCaps.m_conjunctions.getCount(); j++)
                {
                    auto& targetConjunction = targetCaps.m_conjunctions[j];
                    if (conjunction.isIncompatibleWith(targetConjunction))
                        continue;
                    viableConjunction.targetConjunctionIndices.add(j);
                }
                if (!viableConjunction.targetConjunctionIndices.isEmpty())
                {
                    outList.add(viableConjunction);
                }
            }
        };
    List<ViableConjunctionIndex> viableConjunctionsThis;
    List<ViableConjunctionIndex> viableConjunctionsThat;

    getViableConjunction(thisSets, viableConjunctionsThis);
    getViableConjunction(thatSets, viableConjunctionsThat);
    
    for (auto& thisConjunctionIndex : viableConjunctionsThis)
    {
        auto& thisConjunction = thisSets[thisConjunctionIndex.index];
        for (auto& thatConjunctionIndex : viableConjunctionsThat)
        {
            auto& thatConjunction = thatSets[thatConjunctionIndex.index];
            UIntSet intersection = thisConjunctionIndex.targetConjunctionIndices;
            intersection.intersectWith(thatConjunctionIndex.targetConjunctionIndices);
            if (!intersection.isEmpty())
            {
                for (Index targetConjunctionIndex = 0; targetConjunctionIndex < targetCaps.m_conjunctions.getCount(); targetConjunctionIndex++)
                {
                    if (!intersection.contains((UInt)targetConjunctionIndex))
                        continue;
                    if (thisConjunction.isBetterForTarget(thatConjunction, targetCaps.m_conjunctions[targetConjunctionIndex]))
                    {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

bool CapabilitySet::checkCapabilityRequirement(CapabilitySet const& available, CapabilitySet const& required, const CapabilityConjunctionSet*& outFailedAvailableSet)
{
    // Requirements x are met by available disjoint capabilities (a | b) iff
    // both 'a' satisfies x and 'b' satisfies x.
    // If we have a caller function F() decorated with:
    //     [require(hlsl, _sm_6_3)] [require(spirv, _spv_ray_tracing)] void F() { g(); }
    // We'd better make sure that `g()` can be compiled with both (hlsl+_sm_6_3) and (spirv+_spv_ray_tracing) capability sets.
    // In this method, F()'s capability declaration is represented by `available`,
    // and g()'s capability is represented by `required`.
    // We will check that for every capability conjunction X of F(), there is one capability conjunction Y in g() such that X implies Y.
    // 

    outFailedAvailableSet = nullptr;

    if (required.isInvalid())
        return false;

    // If F's capability is empty, we can satisfy any non-empty requirements.
    //
    if (available.isEmpty() && !required.isEmpty())
        return false;

    for (auto& availTargetSet : available.getExpandedAtoms())
    {
        bool implied = false;
        for (auto& requiredTargetSet : required.getExpandedAtoms())
        {
            if (availTargetSet.implies(requiredTargetSet))
            {
                implied = true;
                break;
            }
        }
        if (!implied)
        {
            outFailedAvailableSet = &availTargetSet;
            return false;
        }
    }

    return true;
}

void printDiagnosticArg(StringBuilder& sb, const CapabilitySet& capSet)
{
    bool isFirstSet = true;
    for (auto& set : capSet.getExpandedAtoms())
    {
        List<CapabilityAtom> compactAtomList;
        set.calcCompactedAtoms(compactAtomList);

        if (!isFirstSet)
        {
            sb<< " | ";
        }
        bool isFirst = true;
        for (auto atom : compactAtomList)
        {
            if (!isFirst)
            {
                sb << " + ";
            }
            auto name = capabilityNameToString((CapabilityName)atom);
            if (name.startsWith("_"))
                name = name.tail(1);
            sb << name;
            isFirst = false;
        }
        isFirstSet = false;
    }
}

void printDiagnosticArg(StringBuilder& sb, CapabilityAtom atom)
{
    printDiagnosticArg(sb, (CapabilityName)atom);
}

void printDiagnosticArg(StringBuilder& sb, CapabilityName name)
{
    sb << _getInfo(name).name;
}

}
