// slang-ir-use-uninitialized-values.h
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
// its store blocks and loop exits treated as initialization barriers.
// A load here means an instruction classified as Load by this pass, including
// IRLoad and calls with input arguments.
// Preparation removes loads after a same-block store, exempt wave broadcasts, and
// may-init violations (loads with no reaching store) before these records are analyzed.
// Remaining loads must be instructions directly owned by a block.
struct VariableInitializationInfo
{
    HashSet<IRBlock*> blocksWithStore;
    HashSet<IRBlock*> suppressedBreakBlocks;
    List<IRInst*> loads;
};

// Remove loads that cannot be reached without a preceding store, after preparation.
// func must have a body. Only the loads lists are modified; no IR is changed.
// List indices remain fixed so callers can associate surviving loads with their variables.
// A store to any part counts as initializing the whole variable; component coverage is not checked.
// maxRefinementWork is a nonnegative test override for the condition-walk and discovery
// allowances. Production uses the default CFG-scaled limits. Zero disables refinement.
// The condition-independent result is always computed. Incomplete condition walks
// leave loads unchanged, retaining the results of earlier completed walks.
void removeInitializedLoads(
    IRGlobalValueWithCode* func,
    List<VariableInitializationInfo>& variables,
    Index maxRefinementWork = kMaxIndex);

void checkForUsingUninitializedValues(IRModule* module, DiagnosticSink* sink);
} // namespace Slang
