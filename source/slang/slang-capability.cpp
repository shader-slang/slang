// slang-capability.cpp
#include "slang-capability.h"

// This file implements the core of the "capability" system.

namespace Slang
{

//
// CapabilityAtom
//

// We are going to divide capabilities into a few categories,
// which will be represented as flags for now.
//
// Every capability will be either concrete or abstract.
// An abstract capability basically represents a category
// of related capabilities that all fill a similar role.
// For example, we could have an abstract capability that
// represents "stages" and then the concrete capabilities
// `vertex`, `fragment`, etc. would inherit from it.
//
// Abstract capabilities are critical in our model for
// knowing when two capabilities are fundamentally incompatible.
// For example, it is meaningless to compile code for both
// the `vertex` and `fragment` capabilities at the same time,
// because no target processor supports both at once.
//
// TODO: It is possible that instead of flags this could simply
// identify a "kind" of atom, with two different states.
//
// TODO: It is likely that in a future change we will want to
// add a third case here for "alias" capabilities, which are
// pseudo-atomic capabilities that are just equivalent to
// the set of their bases.
//
typedef uint32_t CapabilityAtomFlags;
enum : CapabilityAtomFlags
{
    kCapabilityAtomFlags_Concrete = 0,
    kCapabilityAtomFlags_Abstract = 1 << 0,
};

// The macros in the `slang-capability-defs.h` file will be used
// to fill out a `static const` array of information about each
// capability atom.
//
struct CapabilityAtomInfo
{
        /// The API-/language-exposed name of the capability.
    char const*         name;

        /// Flags to determine if the capability is concrete-vs-abstract, etc.
    CapabilityAtomFlags flags;
    CapabilityAtom      bases[4];
};
//
// The array is going to be sized to include an entry for `CapabilityAtom::Invalid`
// which as a value of -1, so we need to size the array one larger than the `Count`
// value.
//
static const CapabilityAtomInfo kCapabilityAtoms[Int(CapabilityAtom::Count) + 1] =
{
    { "invalid", 0, { CapabilityAtom::Invalid, CapabilityAtom::Invalid, CapabilityAtom::Invalid, CapabilityAtom::Invalid } },

#define SLANG_CAPABILITY_ATOM(ENUMERATOR, NAME, FLAGS, BASE0, BASE1, BASE2, BASE3) \
    { #NAME, kCapabilityAtomFlags_##FLAGS, { CapabilityAtom::BASE0, CapabilityAtom::BASE1, CapabilityAtom::BASE2, CapabilityAtom::BASE3 } },
#include "slang-capability-defs.h"
};

    /// Get the extended information structure for the given capability `atom`
static CapabilityAtomInfo const& _getInfo(CapabilityAtom atom)
{
    SLANG_ASSERT(Int(atom) < Int(CapabilityAtom::Count));
    return kCapabilityAtoms[Int(atom) + 1];
}

// One capability set or capability atom A implies another set/atom B
// if any target that supports all of the atoms in A must also support
// all of those in B.

    /// Does `thisAtom` imply `thatAtom`?
static bool _implies(CapabilityAtom thisAtom, CapabilityAtom thatAtom)
{
    // When looking at atoms, the immediate easy case is when
    // the two atoms are the same: an atomic capability always
    // implies itself.
    //
    if(thisAtom == thatAtom)
        return true;

    // Otherwise, we want to look at the bases of `thisAtom`
    // to see if any of them imply `thatAtom`, since `thisAtom`
    // implies each of its bases.
    //
    auto& thisAtomInfo = _getInfo(thisAtom);
    for( auto thisAtomBase : thisAtomInfo.bases )
    {
        // The lists of bases are currently using `Invalid` as
        // a sentinel value to terminate them, so we need to
        // bail out of the loop when we see the sentinel.
        //
        if(thisAtomBase == CapabilityAtom::Invalid)
            break;

        if(_implies(thisAtomBase, thatAtom))
            return true;
    }

    return false;
}

    /// Does `base` have any abstract capabilities in common with `otherAtom`
    ///
    /// This subroutine is a helper for `_isIncompatible`.
static bool _hasAbstractBaseInCommon(CapabilityAtom base, CapabilityAtom otherAtom)
{
    // First we check the case where `base` itself is an abstract
    // capability atom.
    //
    auto& baseAtomInfo = _getInfo(base);
    if(baseAtomInfo.flags & kCapabilityAtomFlags_Abstract)
    {
        // If `base` is abstract, and `otherAtom` implies `base`,
        // then that means that `otherAtom` includes one or
        // more atoms that inherit from `base`, and thus the
        // two have an abstract base in common.
        //
        if( _implies(otherAtom, base) )
            return true;
    }

    // If `base` itself has bases, then we want to check if any
    // of *those* are abstract bases that overlap with `otherAtom`.
    //
    for( auto baseBase : baseAtomInfo.bases )
    {
        if(baseBase == CapabilityAtom::Invalid)
            break;

        if(_hasAbstractBaseInCommon(baseBase, otherAtom))
            return true;
    }

    // If we didn't manage to find any overlaps, then we conclude
    // that there are no shared abstract bases.
    //
    return false;
}

    /// Is `thisAtom` incompatible with `thatAtom` (such that no target could ever support both at once)
static bool _isIncompatible(CapabilityAtom thisAtom, CapabilityAtom thatAtom)
{
    // If either atom implies the other, then they aren't incompatible.
    //
    // For example, if there is an atom representing `sm_5_1` that inherits
    // from an atom representing `sm_5_0`, then clearly the two aren't
    // in any way incompatible (a single target can support both).
    //
    if(_implies(thisAtom, thatAtom) || _implies(thatAtom, thisAtom))
        return false;

    // If the two atoms are not in an inheritance relationship, then one of
    // a few cases can apply:
    //
    // * They have no common bases; in this case they are compatible.
    //   An example would be `vertex` and `sm_5_0`.
    //
    // * They have a common base, but it is not marked abstract; in
    //   this case they are compatible. E.g., two GLSL extensions that
    //   both inherit from the `glsl` capability should not conflict.
    //
    // * They have a common base that is marked abstract; in this
    //   case they are incompatible. An example would be `vertex`
    //   and `fragment` both inheriting from the abstract atom
    //   `__stage`.
    //
    // To summarize the above list, we note that two atoms are
    // incompatible with they have an abstract base in common.
    //
    return _hasAbstractBaseInCommon(thisAtom, thatAtom);

    // TODO: The above logic is a bit off, but in a way that doesn't
    // matter just yet.
    //
    // We currently have capabilities like:
    //
    //      abstract capability __target;
    //      capability hlsl : __target;
    //      capability glsl : __target;
    //
    // In this case it is clear that `hlsl` and `glsl` should
    // be incompatible, and that the rules as implemented
    // make that the case.
    //
    // A problem arises when we start to add things like extensions:
    //
    //      capability EXT_cool_thing : glsl;
    //      capability EXT_other_stuff : glsl;
    //
    // In this case, it also seems clear that `EXT_cool_thing`
    // and `EXT_other_stuff` should be mutually compatible.
    // However, with the rules implemented here right now, they
    // would be found incompatible because they share the
    // abstract base `__target`.
    //
    // In this specific case, we know that the relationship
    // between the extensions is fine because they both inherit
    // from `__target` *through* the concrete atom `glsl`.
    //
    // Before adding capabilities that represent optional
    // extensions like this we need to codify the semantics
    // for how incompatibility checks should work in terms
    // of the inheritance graph of capability atoms.
}

CapabilityAtom findCapabilityAtom(UnownedStringSlice const& name)
{
    // For now we are implementing a linear search over the
    // array of capability atoms to perform name lookup.
    //
    for( Index i = 0; i < Index(CapabilityAtom::Count); ++i )
    {
        // Note: using `_getInfo` here instead of accessing
        // the `kCapabilityAtoms` array directly lets us
        // avoid dealing with the offset-by-one indexing
        // choice.
        //
        auto& capInfo = _getInfo(CapabilityAtom(i));
        if(name == UnownedTerminatedStringSlice(capInfo.name))
            return CapabilityAtom(i);
    }
    return CapabilityAtom::Invalid;
}

//
// CapabilitySet
//

// The current design choice in `CapabilitySet` is that it blindly
// stores exactly the atoms it is told to, without any up-front
// processing.
//
// This choice has some down-sides, and there are other representations
// that could be much nicer in the future. Possible improcements include:
//
// * The list of atoms could be *expanded* so that if it contains atom A
//   and atom A implies atom B, then the list should also include B.
//
// * The list of atoms could be *minimized*, such that if atom A implies
//   atom B, then any list that contains A does not include B (both
//   expanded and minimized lists have different benefits).
//
// * The list of atoms could be deduplicated.
//
// * The list of atoms could be sorted.
//
// * The lists could be deduplicated and cached in some central place
//   (the like the session) so that repreated attempts to create the
//   same capability sets return the same objects.
//
// In some parts of the code below we will call out how these improvements
// could affect the algorithms used.

// Given our simple choices right now, the constructors for `CapabilitySet`
// are all straightforward: just adding the right atoms to the list.

CapabilitySet::CapabilitySet()
{}

CapabilitySet::CapabilitySet(Int atomCount, CapabilityAtom const* atoms)
{
    m_atoms.addRange(atoms, atomCount);
}

CapabilitySet::CapabilitySet(CapabilityAtom atom)
{
    m_atoms.add(atom);
}

CapabilitySet::CapabilitySet(List<CapabilityAtom> const& atoms)
    : m_atoms(atoms)
{}


CapabilitySet CapabilitySet::makeEmpty()
{
    return CapabilitySet();
}

CapabilitySet CapabilitySet::makeInvalid()
{
    return CapabilitySet(CapabilityAtom::Invalid);
}

bool CapabilitySet::isEmpty() const
{
    // Checking if a capability set is empty is trivial in any representation;
    // all we need to know is if it has zero atoms in its definition.
    //
    return m_atoms.getCount() == 0;
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
    if(m_atoms.getCount() != 1) return false;
    return m_atoms[0] == CapabilityAtom::Invalid;
}

bool CapabilitySet::isIncompatibleWith(CapabilityAtom that) const
{
    // We know that capabilities that are in an inheritnace
    // relationship with one another can't be incompatible.
    //
    if(this->implies(that) || CapabilitySet(that).implies(*this))
        return false;

    // Othwerise, we want to perform a check for each of the
    // atoms in this set, whether it is incompatible with any
    // of the atoms in the other set (which in this case is one atom).
    //
    for( auto thisAtom : this->m_atoms )
    {
        if(_isIncompatible(thisAtom, that))
            return true;
    }

    return false;
}

bool CapabilitySet::isIncompatibleWith(CapabilitySet const& that) const
{
    // We need to look at the atoms in `this` that are not
    // present in `that`, and vice versa. For each such atom
    // we will check if it is incompatible with the other, by
    // virtue of the other already including a concrete atom
    // that cannot co-exist with it.
    //
    for( auto thisAtom : this->m_atoms )
    {
        if(that.isIncompatibleWith(thisAtom))
            return true;
    }
    for( auto thatAtom : that.m_atoms )
    {
        if(this->isIncompatibleWith(thatAtom))
            return true;
    }
    return false;

    // TODO: If we had a representation that stored a minified,
    // sorted, deduplicated list of atoms, then it would be easy
    // to iterate over the two lists in tandem and identify any
    // element that is present in one list but not the other.
    //
    // Those elements would be the candidates that could cause
    // incompatiblity, so that we wouldn't need to perform
    // the check on each atom like we do above.
}

bool CapabilitySet::implies(CapabilitySet const& that) const
{
    // This capability set implies `other` if for every atom in `other`,
    // that atom is present in this sets list of atoms or it is
    // implies by something in the list of atoms.
    //
    for( auto atom : that.m_atoms )
    {
        if(!this->implies(atom))
            return false;
    }
    return true;

    // TODO: If we had a representation that stored an expanded
    // sorted, deduplicated list of atoms, then we could
    // check the `implies` relationship by scanning through
    // the two lists in tandem and identifying any element
    // in the `that` list that isn't in the `this` list.
    // Such elements would indicate that `that` is not a subset
    // of `this`.
}


bool CapabilitySet::implies(CapabilityAtom atom) const
{
    // If our list of explicit atoms contains `atom`, then
    // we definitely imply it.
    //
    // TODO: If we stored our atom lists sorted, then
    // this operation could be logarithmic rather than
    // linear.
    //
    if(m_atoms.contains(atom))
        return true;

    // If any of our atoms implies `atom` then we
    // also imply it.
    //
    // TODO: If we stored an expanded atom list, then
    // this recursion could be skipped completely, since
    // the containment check above would cover inheirtance
    // relationships too.
    //
    for( auto thisAtom : m_atoms )
    {
        if(_implies(thisAtom, atom))
            return true;
    }

    return false;
}

bool CapabilitySet::operator==(CapabilitySet const& other) const
{
    return this->implies(other) && other.implies(*this);
}

}
