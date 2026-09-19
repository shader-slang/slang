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

/// Overrides the inferred read/write effect of one use for uninitialized-value checking.
///
/// The use must pass the tracked variable, or an address derived from it, to an instruction.
/// Normally the checker infers whether that instruction reads or writes the variable from its IR
/// opcode and operand type. A client that has more precise information can override that inference
/// for an exact `IRUse`. This is useful when a transformation synthesizes an ABI whose parameter
/// direction does not exactly represent the operation being modeled.
///
/// A read observes the incoming value. A possible write is enough to establish that some
/// initialization can reach later uses; a definite write additionally establishes initialization
/// on every path through the instruction. `definitelyWritesValue` implies a possible write even
/// when `mayWriteValue` is false.
struct UninitializedVariableUseEffect
{
    /// The exact operand use whose inferred effect should be replaced.
    IRUse* use = nullptr;

    /// The instruction observes the value that arrives through `use`.
    bool readsValue = false;

    /// The instruction can write some or all of the value on at least one path.
    bool mayWriteValue = false;

    /// The instruction writes the complete value on every path through the instruction.
    bool definitelyWritesValue = false;
};

/// Diagnose reads of one variable that can execute before it is initialized in `code`.
///
/// This entry point is for transformations that introduce a variable after the module-wide
/// uninitialized-value check has run. It applies the same intraprocedural control-flow analysis to
/// that variable alone. Entries in `useEffects` override the inferred effect of their exact uses;
/// all other uses are classified from the IR as usual.
void checkForUsingUninitializedVariable(
    IRGlobalValueWithCode* code,
    IRInst* variable,
    ConstArrayView<UninitializedVariableUseEffect> useEffects,
    DiagnosticSink* sink);

void checkForUsingUninitializedValues(IRModule* module, DiagnosticSink* sink);
} // namespace Slang
