#pragma once

// #include "../core/slang-dictionary.h"
// #include "../core/slang-list.h"
// #include "../core/slang-string.h"
#include "slang-ast-val.h"
#include "slang-capability.h"

// #include <optional>
// #include <stdint.h>

//
#include "slang-capability-val.h.fiddle"

FIDDLE()
namespace Slang
{

//
// Immutable capability set representations using ASTBuilder for deduplication
//

/// Immutable representation of a capability stage set.
/// Contains a stage atom and an associated UIntSetVal representing the capability atoms.
/// These are deduplicated through the ASTBuilder to reduce memory usage.
FIDDLE()
class CapabilityStageSetVal : public Val
{
    FIDDLE(...)

    /// Get the stage atom for this stage set
    CapabilityAtom getStage() const { return CapabilityAtom(getIntConstOperand(0)); }

    /// Get the UIntSetVal containing the capability atoms for this stage
    UIntSetVal* getAtomSet() const { return as<UIntSetVal>(getOperand(1)); }
};

/// Immutable representation of a capability target set.
/// Contains a target atom and a sorted list of CapabilityStageSetVal objects.
/// Uses sorted lists instead of dictionaries for better cache performance with small collections.
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
};

/// Immutable representation of a complete capability set.
/// Contains a sorted list of CapabilityTargetSetVal objects.
/// This is the top-level immutable capability representation that can be shared
/// and deduplicated across the compilation process.
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
    bool isEmpty() const { return getTargetSetCount() == 0; }

    /// Check if this capability set is invalid
    bool isInvalid() const;

    // Const member functions from CapabilitySet (dummy implementations using thaw())

    /// Is this capability set incompatible with the given `other` atom.
    bool isIncompatibleWith(CapabilityAtom other) const { return thaw().isIncompatibleWith(other); }

    /// Is this capability set incompatible with the given `other` name.
    bool isIncompatibleWith(CapabilityName other) const { return thaw().isIncompatibleWith(other); }

    /// Is this capability set incompatible with the given `other` set.
    bool isIncompatibleWith(CapabilitySet const& other) const { return thaw().isIncompatibleWith(other); }

    /// Does this capability set imply all the capabilities in `other`?
    bool implies(CapabilitySet const& other) const { return thaw().implies(other); }

    /// Does this capability set imply at least 1 set in other.
    CapabilitySet::ImpliesReturnFlags atLeastOneSetImpliedInOther(CapabilitySet const& other) const { return thaw().atLeastOneSetImpliedInOther(other); }

    /// Will a `join` with `other` change `this`?
    bool joinWithOtherWillChangeThis(CapabilitySet const& other) const { return thaw().joinWithOtherWillChangeThis(other); }

    /// Does this capability set imply the atomic capability `other`?
    bool implies(CapabilityAtom other) const { return thaw().implies(other); }

    /// Return a capability set of 'target' atoms 'this' has, but 'other' does not.
    CapabilitySet getTargetsThisHasButOtherDoesNot(const CapabilitySet& other) const { return thaw().getTargetsThisHasButOtherDoesNot(other); }

    /// Return a capability set of 'stage' atoms 'this' has, but 'other' does not.
    CapabilitySet getStagesThisHasButOtherDoesNot(const CapabilitySet& other) const { return thaw().getStagesThisHasButOtherDoesNot(other); }

    /// Are these two capability sets equal?
    bool operator==(CapabilitySet const& that) const { return thaw() == that; }

    /// returns true if 'this' is a better target for 'targetCaps' than 'that'
    /// isEqual: is `this` and `that` equal
    bool isBetterForTarget(
        CapabilitySet const& that,
        CapabilitySet const& targetCaps,
        bool& isEqual) const 
    { 
        return thaw().isBetterForTarget(that, targetCaps, isEqual); 
    }

    // If this capability set uniquely implies one stage atom, return it. Otherwise returns
    // CapabilityAtom::Invalid.
    CapabilityAtom getUniquelyImpliedStageAtom() const { return thaw().getUniquelyImpliedStageAtom(); }

    /// Get access to the raw atomic capabilities that define this set.
    /// Get all bottom level UIntSets for each CapabilityTargetSet.
    CapabilitySet::AtomSets::Iterator getAtomSets() const { return thaw().getAtomSets(); }

    /// Gets the first valid compile-target found in the CapabilitySet
    CapabilityAtom getCompileTarget() const { return thaw().getCompileTarget(); }

    /// Gets the first valid stage found in the CapabilitySet
    CapabilityAtom getTargetStage() const { return thaw().getTargetStage(); }
};
} // namespace Slang
