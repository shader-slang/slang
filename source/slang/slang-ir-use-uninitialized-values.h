// slang-ir-use-uninitialized-out-param.h
#pragma once

#include "core/slang-dictionary.h"
#include "core/slang-list.h"

namespace Slang
{
class DiagnosticSink;
struct IRModule;
struct IRBlock;
struct IRInst;
struct IRGlobalValueWithCode;

// The loads still needing an initialization check for one variable, together with
// its store blocks and loop breaks suppressed by the diagnostic policy.
// A load here means an instruction classified as Load by this pass, including
// IRLoad and calls with input arguments.
// Preparation removes loads after a same-block store, exempt wave broadcasts, and
// may-init violations (loads with no reaching store) before these records are analyzed.
struct VariableInitializationInfo
{
    HashSet<IRBlock*> blocksWithStore;
    HashSet<IRBlock*> suppressedBreakBlocks;
    List<IRInst*> loads;
};

// Remove candidate loads unreachable without a preceding store under the diagnostic rules.
// List indices remain fixed so callers can associate surviving loads with their variables.
// First run without condition tracking, then track one repeated SSA condition at a time.
// The initial walk has one work limit; all condition walks share a second limit.
// Both count scalar operations and mask word visits and are capped by nonnegative maxWork.
// If the first shared walk cannot finish, use unlimited per-variable walks. An incomplete condition
// walk leaves loads unchanged, retaining the results of earlier completed walks.
void removeInitializedLoads(
    IRGlobalValueWithCode* func,
    List<VariableInitializationInfo>& variables,
    Index maxWork = kMaxIndex);

void checkForUsingUninitializedValues(IRModule* module, DiagnosticSink* sink);
} // namespace Slang
