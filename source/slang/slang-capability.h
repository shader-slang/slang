// slang-capability.h
#pragma once

#include "../core/slang-list.h"
#include "../core/slang-string.h"

#include <stdint.h>

namespace Slang
{

// This file defines a system for reasoning about the "capabilities" that a
// target supports or, conversely, the capabilities that a function or other
// symbol requires.
//
// The central idea is that we can think of the each of these cases as a set,
// where the elements of the set are atomic features that are either present
// on a target or not (no in-between states). For example, an atomic feature
// might be used to represent support for double-precision floating-point
// operations. When compiling for a target, we need to know whether the
// target supports double-precision or not, and for a particular function
// it either requires double-precision math to run, or not.
//
// In this system, the atomic capabilities are represented as cases of
// the `CapabilityAtom` enumeration, which is generated from declarations
// in the `slang-capability-defs.h` file.
//
enum class CapabilityAtom : int32_t
{
    // The "invalid" capability represents an atomic feature that no
    // platform can/will ever support. If we ever determine that a
    // function needs the invalid capability, it would be reasonable
    // to report that situation as an error.
    //
    Invalid = 0,

#define SLANG_CAPABILITY_ATOM(ENUMERATOR, NAME, FLAVOR, CONFLICT, RANK, BASE0, BASE1, BASE2, BASE3) \
    ENUMERATOR,

#include "slang-capability-defs.h"

    Count,
};

// Once we have a universe of suitable capability atoms, we can define
// the capabilities of a target as simply the set of all atomic capabilities
// that it supports.
//
// The situation is slightly more complicated for a function. A function
// might require a specific set of atomic feature, and that is the simple
// case. In this simple case, we know that a target can run a function
// if the features of the target are a super-set of those required by
// the function.
//
// In the more general case, we might have a function that can be used
// with multiple different combinations of features: e.g., you can use
// the function if your target supports features A and B, or if it supports
// features C and D. In our representation, that case is handled by
// assocaiting multiple distinct sets of capabilities with one declaration,
// with each set expressing one way that the declaration can be legally used.
//
// In all cases, we represent a set of capabilities with `CapabilitySet`.

    /// A set of capabilities, representing features that are either supported or required
struct CapabilitySet
{
public:
        /// Default-construct an empty capability set
    CapabilitySet();

        /// Construct a capability set from an explicit list of atomic capabilities
    CapabilitySet(Int atomCount, CapabilityAtom const* atoms);

        /// Construct a capability set from an explicit list of atomic capabilities
    explicit CapabilitySet(List<CapabilityAtom> const& atoms);

        /// Construct a singleton set from a single atomic capability
    explicit CapabilitySet(CapabilityAtom atom);

        /// Make an empty capability set
    static CapabilitySet makeEmpty();

        /// Make an invalid capability set (such that no target could ever support it)
    static CapabilitySet makeInvalid();

        /// Is this capability set empty (such that any target supports it)?
    bool isEmpty() const;

        /// Is this capability set invalid (such that no target could support it)?
    bool isInvalid() const;

    // Capabilities are "incompatible" if no target platform can ever support both
    // at the same time. For example, the `HLSL` and `GLSL` capabilities are
    // incompatible, because a single target cannot be both an HLSL target and
    // a GLSL target (at least for now).
    //
    // Note that we are using the term "incompatible" here even though it
    // seems like "disjoint" would be intuitively correct (HLSL and GLSL
    // targets sure do seem to be disjoint). The problem is that in our
    // set-theoretic representation of capabilities, incompatible capability
    // sets are *never* disjoint sets of atoms, and (valid) disjoint sets of atoms
    // *never* represent incompatible capability sets.

        /// Is this capability set incompatible with the given `other` set.
    bool isIncompatibleWith(CapabilityAtom other) const;

        /// Is this capability set incompatible with the given `other` atomic capability.
    bool isIncompatibleWith(CapabilitySet const& other) const;

    // One capability set A "implies" another set B if a target that
    // supports A must also support all of B.
    //
    // In practice, this means that "A implies B" is the same as
    // "A is a subset of B" in the set-theoretic model, but
    // we ant to think of this primarily as supported/required features,
    // and not get hung up on the set theory.

        /// Does this capability set imply all the capabilities in `other`?
    bool implies(CapabilitySet const& other) const;

        /// Does this capability set imply the atomic capability `other`?
    bool implies(CapabilityAtom other) const;

    // A capability set is equal to another if each implies the other.

        /// Are these two capability sets equal?
    bool operator==(CapabilitySet const& that) const;

        /// Get access to the raw atomic capabilities that define this set.
    List<CapabilityAtom> const& getExpandedAtoms() const { return m_expandedAtoms; }

        /// Calculate a list of "compacted" atoms, which excludes any atoms from the expanded list that are implies by another item in the list.
    void calcCompactedAtoms(List<CapabilityAtom>& outAtoms) const;

    Int countIntersectionWith(CapabilitySet const& that) const;

    bool isBetterForTarget(CapabilitySet const& that, CapabilitySet const& targetCaps);

private:
    void _init(Int atomCount, CapabilityAtom const* atoms);

    uint32_t _calcConflictMask() const;
    uint32_t _calcDifferenceScoreWith(CapabilitySet const& other) const;

    // The underlying representation we use is a sorted and deduplicated
    // list of all the (non-alias) atoms that are present in the set.
    // This "expanded" list uses the transitive closure over the inheritnace
    // relationship between the atoms.
    //
    List<CapabilityAtom> m_expandedAtoms;
};

    /// Are the `left` and `right` capability sets unequal?
inline bool operator!=(CapabilitySet const& left, CapabilitySet const& right)
{
    return !(left == right);
}

    /// Returns true if atom is derived from base
bool isCapabilityDerivedFrom(CapabilityAtom atom, CapabilityAtom base);

    /// Find a capability atom with the given `name`, or return CapabilityAtom::Invalid.
CapabilityAtom findCapabilityAtom(UnownedStringSlice const& name);
}
