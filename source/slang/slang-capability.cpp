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
enum class CapabilityAtomFlavor : int32_t
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

// Certain capability atoms will conflict with one another,
// such that a concrete target should never be able to support
// both.
//
// It is possible in theory to define "conflicting" capabilities
// in terms of the inheritance graph, but that makes checking
// for conflicts more difficult.
//
// Instead, we are going to allow each capability to define a
// mask to indicate group(s) of conflicting capabilities it
// belongs to. Two different capability atoms that have
// overlapping masks will be considered to conflict.
//
enum class CapabilityAtomConflictMask : uint32_t
{
    // By default, most capability atoms do not conflict with one another.
    None                = 0,

    // Capability atoms that reprsent target code generation formats always conflict.
    // (e.g., you cannot generate both HLSL and C++ output at once)
    TargetFormat        = 1 << 0,

    // Capability atoms that represent GLSL ray tracing extensions conflict with
    // one another (we only want to use one such extension at a time).
    RayTracingExtension = 1 << 1,

    // Capability atoms that represent GLSL fragment shader barycentric extensions conflict with
    // one another (we only want to use one such extension at a time).
    FragmentShaderBarycentricExtension = 1 << 2,
};

// For simplicity in building up our data structure representing
// all capability atoms, we will limit the number of bases that
// a capability atom is allowed to inherit from.
//
static const int kCapabilityAtom_MaxBases = 4;

// The macros in the `slang-capability-defs.h` file will be used
// to fill out a `static const` array of information about each
// capability atom.
//
struct CapabilityAtomInfo
{
        /// The API-/language-exposed name of the capability.
    char const*                 name;

        /// Flavor of atom: concrete, abstract, or alias
    CapabilityAtomFlavor        flavor;

        /// A mask to indicate which other categories of atoms this one conflicts with
    CapabilityAtomConflictMask  conflictMask;

        /// Ranking to use when deciding if this atom is a "better" one to select.
    uint32_t                    rank;

        /// Base atoms this one "inherits" from (terminated with `Invalid` if not all entries used).
    CapabilityAtom              bases[kCapabilityAtom_MaxBases];
};
//
static const CapabilityAtomInfo kCapabilityAtoms[Int(CapabilityAtom::Count)] =
{
    { "invalid", CapabilityAtomFlavor::Concrete, CapabilityAtomConflictMask::None, 0, { CapabilityAtom::Invalid, CapabilityAtom::Invalid, CapabilityAtom::Invalid, CapabilityAtom::Invalid } },

#define SLANG_CAPABILITY_ATOM(ENUMERATOR, NAME, FLAVOR, CONFLICT, RANK, BASE0, BASE1, BASE2, BASE3) \
    { #NAME, CapabilityAtomFlavor::FLAVOR, CapabilityAtomConflictMask::CONFLICT, RANK, { CapabilityAtom::BASE0, CapabilityAtom::BASE1, CapabilityAtom::BASE2, CapabilityAtom::BASE3 } },
#include "slang-capability-defs.h"
};

    /// Get the extended information structure for the given capability `atom`
static CapabilityAtomInfo const& _getInfo(CapabilityAtom atom)
{
    SLANG_ASSERT(Int(atom) < Int(CapabilityAtom::Count));
    return kCapabilityAtoms[Int(atom)];
}

void getCapabilityAtomNames(List<UnownedStringSlice>& ioNames)
{
    ioNames.setCount(Count(CapabilityAtom::Count));
    for (Index i = 0; i < Count(CapabilityAtom::Count); ++i)
    {
        ioNames[i] = UnownedStringSlice(_getInfo(CapabilityAtom(i)).name);
    }
}

CapabilityAtom findCapabilityAtom(UnownedStringSlice const& name)
{
    // For now we are implementing a linear search over the
    // array of capability atoms to perform name lookup.
    //
    for( Index i = 0; i < Index(CapabilityAtom::Count); ++i )
    {
        auto& capInfo = _getInfo(CapabilityAtom(i));
        if(name == UnownedTerminatedStringSlice(capInfo.name))
            return CapabilityAtom(i);
    }
    return CapabilityAtom::Invalid;
}

bool isCapabilityDerivedFrom(CapabilityAtom atom, CapabilityAtom base)
{
    if (atom == base)
    {
        return true;
    }

    const auto& info = kCapabilityAtoms[Index(atom)];

    for (auto cur : info.bases)
    {
        if (cur == CapabilityAtom::Invalid)
        {
            return false;
        }

        if (isCapabilityDerivedFrom(cur, base))
        {
            return true;
        }
    }

    return false;
}

//
// CapabilitySet
//

// The current design choice in `CapabilitySet` is that it stores
// an expanded, deduplicated, and sorted list of the capability
// atoms in the set. "Expanded" here means that it includes the
// transitive closure of the inheritance graph of those atoms.
//
// This choice is intended to make certain operations on
// capability sets more efficient, since use things like
// binary searches to efficiently detect whether an atom
// is present in a set.

CapabilitySet::CapabilitySet()
{}

CapabilitySet::CapabilitySet(Int atomCount, CapabilityAtom const* atoms)
{
    _init(atomCount, atoms);
}

CapabilitySet::CapabilitySet(CapabilityAtom atom)
{
    _init(1, &atom);
}

CapabilitySet::CapabilitySet(List<CapabilityAtom> const& atoms)
{
    _init(atoms.getCount(), atoms.getBuffer());
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
    result.m_expandedAtoms.add(CapabilityAtom::Invalid);
    return result;
}

    /// Helper routine for `CapabilitySet::_init`.
    ///
    /// Recursively add all atoms implied by `atom` to `ioExpandedAtoms`.
    ///
static void _addAtomsRec(
    CapabilityAtom              atom,
    HashSet<CapabilityAtom>&    ioExpandedAtoms)
{
    auto& atomInfo = _getInfo(atom);

    // The first step is to add `atom` itself, *unless*
    // it is an alias, because an alias shouldn't impact
    // whether one set is considered a subset/superset of
    // another.
    //
    if(atomInfo.flavor != CapabilityAtomFlavor::Alias)
    {
        ioExpandedAtoms.add(atom);
    }

    // Next we add all the atoms transitively implied by `atom`.
    //
    for(auto baseAtom : atomInfo.bases)
    {
        // Note: the list of `bases` is a fixed-size array, but
        // can be terminated with `Invalid` to indicate that
        // not all of the entries are being used.
        //
        // If we see the sentinel, then we know we are at the end
        // of the list.
        //
        if(baseAtom == CapabilityAtom::Invalid)
            break;

        _addAtomsRec(baseAtom, ioExpandedAtoms);
    }
}

void CapabilitySet::_init(Int atomCount, CapabilityAtom const* atoms)
{
    // In order to fill in the expanded and deduplicated
    // set of atoms, we will use an explicit hash set
    // and then recursively walk the tree of atoms and
    // their bases.
    //
    HashSet<CapabilityAtom> expandedAtomsSet;
    for(Int i = 0; i < atomCount; ++i)
    {
        _addAtomsRec(atoms[i], expandedAtomsSet);
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

void CapabilitySet::calcCompactedAtoms(List<CapabilityAtom>& outAtoms) const
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
        for(auto baseAtom : atomInfo.bases)
        {
            // Note: dealing with possible early termination of the `bases` list.
            if(baseAtom == CapabilityAtom::Invalid)
                break;

            redundantAtomsSet.add(baseAtom);
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

bool CapabilitySet::isEmpty() const
{
    // Checking if a capability set is empty is trivial in any representation;
    // all we need to know is if it has zero atoms in its definition.
    //
    return m_expandedAtoms.getCount() == 0;
}

bool CapabilitySet::isInvalid() const
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

bool CapabilitySet::isIncompatibleWith(CapabilityAtom that) const
{
    // Checking for incompatibility is complicated, and it is best
    // to only implement it for full (expanded) sets.
    //
    return isIncompatibleWith(CapabilitySet(that));
}

uint32_t CapabilitySet::_calcConflictMask() const
{
    // Given a capbility set, we want to compute the mask representing
    // all groups of features for which it holds a potentially-conflicting atom.
    //
    uint32_t mask = 0;
    for( auto atom : m_expandedAtoms )
    {
        mask |= uint32_t(_getInfo(atom).conflictMask);
    }
    return mask;
}

bool CapabilitySet::isIncompatibleWith(CapabilitySet const& that) const
{
    // The `this` and `that` sets are incompatible if there exists
    // an atom A in `this` and an atom `B` in `that` such that
    // A and B are not equal, but the two have overlapping "conflict mask."
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
    uint32_t thisMask = this->_calcConflictMask();
    uint32_t thatMask = that._calcConflictMask();

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
            auto thisAtomMask = uint32_t(_getInfo(thisAtom).conflictMask);
            if(thisAtomMask & thatMask)
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
            auto thatAtomMask = uint32_t(_getInfo(thatAtom).conflictMask);
            if(thatAtomMask & thisMask)
                return true;
            thatIndex++;
        }
    }

    return false;
}

bool CapabilitySet::implies(CapabilitySet const& that) const
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
    return true;
}

    /// Helper functor for binary search on lists of `CapabilityAtom`
struct CapabilityAtomComparator
{
    int operator()(CapabilityAtom left, CapabilityAtom right)
    {
        return int(Int(left) - Int(right));
    }
};

bool CapabilitySet::implies(CapabilityAtom atom) const
{
    // The common case here is when `atom` is not an alias.
    //
    if( _getInfo(atom).flavor != CapabilityAtomFlavor::Alias )
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
    else
    {
        // In the case where `atom` is an alias, then it won't
        // appear in the expanded list, and we need to check
        // whether `this` set implies everything that `atom`
        // transitively inherits from.
        //
        // The simplest way to do that is to expand `atom`
        // into the full capability set it stands for and
        // check that.
        //
        return implies(CapabilitySet(atom));
    }
}

Int CapabilitySet::countIntersectionWith(CapabilitySet const& that) const
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

bool CapabilitySet::isBetterForTarget(
    CapabilitySet const& existingCaps,
    CapabilitySet const& targetCaps)
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
    // TODO: This logic has the negative effect of always preferring
    // to enable optional features even if they aren't necessary.
    // It would prefer the set {glsl, optionalFeature} over the set
    // {glsl}, even though we might argue that a default implementaton
    // that works without any optional features is "obviously" what
    // the user means if they didn't enable those features.
    //
    // TODO: The right answer is possibly that we want to partition
    // `candidateCaps` and `existingCaps` into two parts: their
    // intersection with `targetCaps` and their difference with it.
    //
    // For the intersection part of things, we'd want to favor a
    // definition that is more specialized, while for the difference
    // part we'd actually wnat to favor a definition that is less
    // specialized.
    //
    if(candidateCaps.implies(existingCaps)) return true;
    if(existingCaps.implies(candidateCaps)) return true;

    // At this point we have the problem that neither candidate
    // appears to be "obviously" better for the target, but we
    // want some way to disambiguate them.
    //
    // What we want to do now is scan through what makes each candidate
    // different from the other, and see if anything in either case
    // has a ranking that should make it be preferred.
    //
    // TODO: This should probably *not* be considering anything that
    // is implied/supported by the target.
    //
    auto candidateScore = candidateCaps._calcDifferenceScoreWith(existingCaps);
    auto existingScore = existingCaps._calcDifferenceScoreWith(candidateCaps);
    if(candidateScore != existingScore)
        return candidateScore > existingScore;

    return false;
}

uint32_t CapabilitySet::_calcDifferenceScoreWith(CapabilitySet const& that) const
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


bool CapabilitySet::operator==(CapabilitySet const& other) const
{
    // TODO: We should be able to implement this more efficiently
    // by scanning over the two sets in tandem.

    return this->implies(other) && other.implies(*this);
}

}
