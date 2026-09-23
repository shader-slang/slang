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
/// The use must pass the tracked variable, or an address derived from it, to an instruction.
/// Normally the checker infers whether that instruction reads or writes the variable from its IR
/// opcode and operand type. A client with more precise information can override that inference for
/// an exact `IRUse`. This override is intended for a transformation that introduces a call
/// argument whose IR parameter direction is less precise than the source operation being modeled.
///
/// A read observes the incoming value. A possible write means that at least one execution of the
/// instruction can update the tracked value. A definite write means that every normally completing
/// execution initializes the complete value before control reaches a successor.
/// `definitelyWritesValue` implies a possible write even when `mayWriteValue` is false.
struct UninitializedVariableUseEffect
{
    /// The exact operand use whose inferred effect should be replaced.
    IRUse* use = nullptr;

    /// The instruction observes the value that arrives through `use`.
    bool readsValue = false;

    /// At least one execution that reaches the instruction can update the tracked value.
    bool mayWriteValue = false;

    /// Every normally completing execution initializes the complete value before any successor.
    bool definitelyWritesValue = false;
};

/// Diagnose reads of one variable that can execute before it is initialized in `code`.
///
/// Call this function for a variable introduced after the module-wide uninitialized-value check has
/// run. `variable` must belong to `code`. The function applies the same intraprocedural control-flow
/// analysis to that variable alone. Entries in `useEffects` override the inferred effect of their
/// exact uses; all other uses are classified from the IR as usual. Each entry must name a distinct
/// use in the alias and use graph rooted at `variable`.
void checkForUsingUninitializedVariable(
    IRGlobalValueWithCode* code,
    IRInst* variable,
    ConstArrayView<UninitializedVariableUseEffect> useEffects,
    DiagnosticSink* sink);

/// Diagnose uses of uninitialized values throughout `module`.
void checkForUsingUninitializedValues(IRModule* module, DiagnosticSink* sink);
} // namespace Slang
