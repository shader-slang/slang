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

/// Describes the source-level effect of one use when its IR type alone is insufficient.
///
/// The use must pass the variable, or an address derived from it, to an instruction. A client uses
/// this structure when the instruction's IR type does not describe its source-level effect. For
/// example, a generated `inout` parameter can imply an incoming read that the source program did
/// not perform, while a store through a derived address initializes only part of the root value.
/// The client can state separately whether the exact use observes, may initialize, or definitely
/// initializes the value. `definitelyWritesValue` always counts as a write, independently of
/// `mayWriteValue`.
struct UninitializedVariableUseEffect
{
    IRUse* use = nullptr;
    bool readsValue = false;
    bool mayWriteValue = false;
    bool definitelyWritesValue = false;
};

/// Diagnose reads of `variable` that can execute before it is initialized in `code`.
void checkForUsingUninitializedVariable(
    IRGlobalValueWithCode* code,
    IRInst* variable,
    ConstArrayView<UninitializedVariableUseEffect> useEffects,
    DiagnosticSink* sink);

void checkForUsingUninitializedValues(IRModule* module, DiagnosticSink* sink);
} // namespace Slang
