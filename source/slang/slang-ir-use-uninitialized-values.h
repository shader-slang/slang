// slang-ir-use-uninitialized-values.h
#pragma once

#include "core/slang-array-view.h"

namespace Slang
{
class DiagnosticSink;
struct IRFunc;
struct IRModule;
struct IRUse;
struct IRVar;

/// A `GeneratedResourceLocalUseEffect` records how one instruction accesses a generated resource
/// local through one pointer operand.
///
/// A read observes the value present immediately before the instruction executes. A possible write
/// may update the value. `stopsUninitializedStatePropagation` means that every execution that
/// continues past the instruction has a fully assigned value. A call that cannot return also
/// satisfies this condition because no execution continues past it. The checker also treats an
/// instruction that stops propagation as a possible write, so callers need not set `mayWriteValue`
/// solely to express that implication.
///
/// Every record must set at least one effect field.
struct GeneratedResourceLocalUseEffect
{
    /// The operand through which the instruction accesses the generated local.
    IRUse* use = nullptr;

    /// True when the instruction reads the value present immediately before it executes.
    bool readsValue = false;

    /// True when the instruction may write any part of the value.
    bool mayWriteValue = false;

    /// Whether every execution that continues past the instruction has a fully assigned value.
    bool stopsUninitializedStatePropagation = false;
};

/// Diagnose reads of a generated entry-point resource local that can execute before the local has
/// been fully assigned.
///
/// `variable` must be an `IRVar` in `func`. After the resource-global rewrite, `effects` must
/// contain exactly one `GeneratedResourceLocalUseEffect` for every runtime access reached by
/// following supported operations that transfer access from `variable` to another address. Each
/// `use` must be a pointer-typed operand of an instruction directly inside one of `func`'s blocks,
/// and each record must set at least one effect field. The function infers no missing effects. It
/// does not apply the module-wide warning pass's loop heuristic: it preserves zero-trip paths and
/// paths through the loop body that cross no propagation boundary.
void checkForUsingUninitializedGeneratedResourceLocal(
    IRFunc* func,
    IRVar* variable,
    ConstArrayView<GeneratedResourceLocalUseEffect> effects,
    DiagnosticSink* sink);

/// Diagnose uses of uninitialized values throughout `module`.
///
/// When the global-variable check finds a possible write to a global marked as file- or
/// namespace-scope mutable storage, it records that inconclusive result on the global. The later
/// resource-global analysis can use the record if the global is selected for per-invocation
/// replacement and entry-point linking removes every writer.
void checkForUsingUninitializedValues(IRModule* module, DiagnosticSink* sink);
} // namespace Slang
