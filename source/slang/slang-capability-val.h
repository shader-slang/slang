#pragma once

#include "slang-ast-val.h"
#include "slang-capability.h"

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
};
} // namespace Slang
