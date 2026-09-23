// slang-ir-use-uninitialized-values.h
#pragma once

#include "core/slang-array-view.h"

namespace Slang
{
class DiagnosticSink;
struct IRGlobalValueWithCode;
struct IRInst;
struct IRModule;
struct IRUse;

/// An `UninitializedVariableUseEffect` overrides the inferred read/write effect of one use during
/// uninitialized-value checking.
///
/// `use` must be an operand that supplies either the tracked variable itself or a value through
/// which the user instruction can access that variable. Normally the checker infers whether the
/// instruction reads or writes the variable from its IR opcode, operand role, and the SSA-value or
/// storage-access transfer that reaches the use. A client with more precise information can
/// override that inference for an exact `IRUse`. This override is intended for a transformation
/// that knows whether a use applies to the complete variable or only one of its subobjects, when
/// the generic checker cannot recover that distinction from the rewritten IR.
/// Examples include a generated call argument whose parameter direction is imprecise and a store
/// through an address known to select only part of the tracked value.
///
/// A read means that the instruction reads the tracked value. A possible write means that at least
/// one execution of the instruction can update the tracked value. A definite write means that
/// every normally completing execution initializes the complete value before control reaches a
/// successor.
/// The checker treats every definite write as a possible write; a caller need not also set
/// `mayWriteValue`.
struct UninitializedVariableUseEffect
{
    /// The exact operand use whose inferred effect should be replaced.
    IRUse* use = nullptr;

    /// The instruction reads the tracked value through `use`.
    bool readsValue = false;

    /// At least one execution that reaches the instruction can update the tracked value.
    bool mayWriteValue = false;

    /// Every normally completing execution initializes the complete value before any successor.
    bool definitelyWritesValue = false;
};

/// Diagnose reads of one variable that can execute before it is initialized in `code`.
///
/// Call this function for a variable introduced after the module-wide uninitialized-value check has
/// run. `variable` must belong to `code`. The function applies the same intraprocedural
/// control-flow analysis to that variable alone. Entries in `useEffects` override the inferred
/// effect of their exact uses; all other uses are classified from the IR as usual. Each entry must
/// name a distinct use reached by following SSA-value flow or storage-access transfers from
/// `variable`.
void checkForUsingUninitializedVariable(
    IRGlobalValueWithCode* code,
    IRInst* variable,
    ConstArrayView<UninitializedVariableUseEffect> useEffects,
    DiagnosticSink* sink);

/// Diagnose uses of uninitialized values throughout `module`.
void checkForUsingUninitializedValues(IRModule* module, DiagnosticSink* sink);
} // namespace Slang
