#pragma once

// If we have these in the slang-capability.h header then we get some circular
// includes due to the use of capabilty sets somewhere underneath
// slang-ast-val.h (in slang-profile.h specifically)

#include "core/slang-performance-profiler.h"
#include "slang-ast-val.h"
#include "slang-capability.h"

// empty comment to preserve the order with clang-format
#include "slang-capability-val.h.fiddle"

FIDDLE()
namespace Slang
{

//
// Immutable representation of a capability stage set.
// Contains a stage atom and an associated UIntSetVal representing the capability atoms.
// These are deduplicated through the ASTBuilder to reduce memory usage.
//
FIDDLE()
class CapabilityStageSetVal : public Val
{
    FIDDLE(...)

    /// Get the stage atom for this stage set
    CapabilityAtom getStage() const { return CapabilityAtom(getIntConstOperand(0)); }

    /// Get the UIntSetVal containing the capability atoms for this stage
    UIntSetVal* getAtomSet() const { return as<UIntSetVal>(getOperand(1)); }

    void _toTextOverride(StringBuilder& out);
    Val* _resolveImplOverride() { return this; }
};

//
// Immutable representation of a capability target set.
// Contains a target atom and a sorted list of CapabilityStageSetVal objects.
// Uses sorted lists instead of dictionaries (these are never very long,
// usually ~4 elements)
//
FIDDLE()
class CapabilityTargetSetVal : public Val
{
    FIDDLE(...)

    /// Get the target atom for this target set
    CapabilityAtom getTarget() const { return CapabilityAtom(getIntConstOperand(0)); }

    /// Get the number of stage sets in this target set
    Index getStageSetCount() const { return getOperandCount() - 1; }

    /// Get a specific stage set by index (sorted by stage atom)
    CapabilityStageSetVal* getStageSet(Index index) const
    {
        return as<CapabilityStageSetVal>(getOperand(index + 1));
    }

    /// Get all stage sets as an operand view
    Val::OperandView<CapabilityStageSetVal> getStageSets() const
    {
        return Val::OperandView<CapabilityStageSetVal>(this, 1, getStageSetCount());
    }

    /// Find a stage set by stage atom using linear search
    /// Returns nullptr if not found
    CapabilityStageSetVal* findStageSet(CapabilityAtom stage) const;

    void _toTextOverride(StringBuilder& out);
    Val* _resolveImplOverride() { return this; }
};

//
// Immutable representation of a complete capability set.
// Contains a sorted list of CapabilityTargetSetVal objects.
// This is the top-level immutable capability representation that can be shared
// and deduplicated across the compilation process.
//
FIDDLE()
class CapabilitySetVal : public Val
{
    FIDDLE(...)

    /// Get the number of target sets in this capability set
    Index getTargetSetCount() const { return getOperandCount(); }

    /// Get a specific target set by index (sorted by target atom)
    CapabilityTargetSetVal* getTargetSet(Index index) const
    {
        return as<CapabilityTargetSetVal>(getOperand(index));
    }

    /// Get all target sets as an operand view
    Val::OperandView<CapabilityTargetSetVal> getTargetSets() const
    {
        return Val::OperandView<CapabilityTargetSetVal>(this, 0, getTargetSetCount());
    }

    /// Find a target set by target atom using linear search
    /// Returns nullptr if not found
    CapabilityTargetSetVal* findTargetSet(CapabilityAtom target) const;

    /// Convert this immutable capability set back to a mutable CapabilitySet
    CapabilitySet thaw() const;

    /// Check if this capability set is empty (no target sets)
    bool isEmpty() const
    {
        SLANG_PROFILE_CAPABILITY_SETS;
        return getTargetSetCount() == 0;
    }

    /// Check if this capability set is invalid
    bool isInvalid() const;

    // Const member functions from CapabilitySet (dummy implementations using thaw())

    /// Is this capability set incompatible with the given `other` atom.
    bool isIncompatibleWith(CapabilityAtom other) const;

    /// Is this capability set incompatible with the given `other` name.
    bool isIncompatibleWith(CapabilityName other) const;

    /// Is this capability set incompatible with the given `other` set.
    bool isIncompatibleWith(CapabilitySet const& other) const;

    /// Is this capability set incompatible with the given `other` set.
    bool isIncompatibleWith(CapabilitySetVal const* other) const;

    /// Does this capability set imply all the capabilities in `other`?
    bool implies(CapabilitySet const& other) const;
    bool implies(CapabilitySetVal const* other) const
    {
        SLANG_PROFILE_CAPABILITY_SETS;
        return (int)_implies(other, CapabilitySet::ImpliesFlags::None) &
               (int)CapabilitySet::ImpliesReturnFlags::Implied;
    }

    /// Does this capability set imply at least 1 set in other.
    CapabilitySet::ImpliesReturnFlags atLeastOneSetImpliedInOther(CapabilitySet const& other) const;
    CapabilitySet::ImpliesReturnFlags atLeastOneSetImpliedInOther(
        CapabilitySetVal const* other) const
    {
        SLANG_PROFILE_CAPABILITY_SETS;
        return _implies(other, CapabilitySet::ImpliesFlags::OnlyRequireASingleValidImply);
    }

    /// Will a `join` with `other` change `this`?
    bool joinWithOtherWillChangeThis(CapabilitySet const& other) const
    {
        SLANG_PROFILE_CAPABILITY_SETS;
        return CapabilitySet{this}.joinWithOtherWillChangeThis(other);
    }
    bool joinWithOtherWillChangeThis(CapabilitySetVal const* other) const
    {
        return !(
            (int)_implies(other, CapabilitySet::ImpliesFlags::CannotHaveMoreTargetAndStageSets) &
            (int)CapabilitySet::ImpliesReturnFlags::Implied);
    }

    /// Does this capability set imply the atomic capability `other`?
    bool implies(CapabilityAtom other) const
    {
        SLANG_PROFILE_CAPABILITY_SETS;
        return CapabilitySet{this}.implies(other);
    }

    /// Return a capability set of 'target' atoms 'this' has, but 'other' does not.
    CapabilitySet getTargetsThisHasButOtherDoesNot(const CapabilitySet& other) const
    {
        SLANG_PROFILE_CAPABILITY_SETS;
        return CapabilitySet{this}.getTargetsThisHasButOtherDoesNot(other);
    }
    CapabilitySet getTargetsThisHasButOtherDoesNot(CapabilitySetVal const* other) const
    {
        SLANG_PROFILE_CAPABILITY_SETS;
        return CapabilitySet{this}.getTargetsThisHasButOtherDoesNot(CapabilitySet{other});
    }

    /// Return a capability set of 'stage' atoms 'this' has, but 'other' does not.
    CapabilitySet getStagesThisHasButOtherDoesNot(const CapabilitySet& other) const
    {
        SLANG_PROFILE_CAPABILITY_SETS;
        return CapabilitySet{this}.getStagesThisHasButOtherDoesNot(other);
    }
    CapabilitySet getStagesThisHasButOtherDoesNot(CapabilitySetVal const* other) const
    {
        SLANG_PROFILE_CAPABILITY_SETS;
        return CapabilitySet{this}.getStagesThisHasButOtherDoesNot(CapabilitySet{other});
    }

    /// Are these two capability sets equal?
    bool operator==(CapabilitySet const& that) const
    {
        SLANG_PROFILE_CAPABILITY_SETS;
        return CapabilitySet{this} == that;
    }
    bool operator==(CapabilitySetVal const* that) const
    {
        SLANG_PROFILE_CAPABILITY_SETS;
        return CapabilitySet{this} == CapabilitySet{that};
    }

    /// returns true if 'this' is a better target for 'targetCaps' than 'that'
    /// isEqual: is `this` and `that` equal
    bool isBetterForTarget(
        CapabilitySet const& that,
        CapabilitySet const& targetCaps,
        bool& isEqual) const
    {
        SLANG_PROFILE_CAPABILITY_SETS;
        return CapabilitySet{this}.isBetterForTarget(that, targetCaps, isEqual);
    }
    bool isBetterForTarget(
        CapabilitySetVal const* that,
        CapabilitySetVal const* targetCaps,
        bool& isEqual) const
    {
        SLANG_PROFILE_CAPABILITY_SETS;
        return CapabilitySet{this}.isBetterForTarget(
            CapabilitySet{that},
            CapabilitySet{targetCaps},
            isEqual);
    }

    // If this capability set uniquely implies one stage atom, return it. Otherwise returns
    // CapabilityAtom::Invalid.
    CapabilityAtom getUniquelyImpliedStageAtom() const
    {
        SLANG_PROFILE_CAPABILITY_SETS;
        return CapabilitySet{this}.getUniquelyImpliedStageAtom();
    }

    /// Gets the first valid compile-target found in the CapabilitySet
    CapabilityAtom getCompileTarget() const
    {
        SLANG_PROFILE_CAPABILITY_SETS;
        return CapabilitySet{this}.getCompileTarget();
    }

    /// Gets the first valid stage found in the CapabilitySet
    CapabilityAtom getTargetStage() const
    {
        SLANG_PROFILE_CAPABILITY_SETS;
        return CapabilitySet{this}.getTargetStage();
    }

    void _toTextOverride(StringBuilder& out);
    Val* _resolveImplOverride() { return this; }

private:
    friend struct CapabilitySet;

    /// Helper method for implies operations
    CapabilitySet::ImpliesReturnFlags _implies(
        CapabilitySetVal const* other,
        CapabilitySet::ImpliesFlags flags) const;

    // It's a lot quicker to cache and copy the Capability set, thawing is done
    // about 130000 times for the core module, but only 360 unique results are
    // ever returned.
    mutable std::optional<CapabilitySet> cachedThawedCapabilitySet;
};
} // namespace Slang
