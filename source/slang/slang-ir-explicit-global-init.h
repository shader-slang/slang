// slang-ir-explicit-global-init.h
#pragma once

#include "core/slang-list.h"

namespace Slang
{
struct IRGlobalVar;
struct IRModule;
class TargetProgram;

/// Make target-selected global initialization explicit at the start of every entry point.
///
/// Selected globals retain their storage but no longer contain initializer bodies.
void moveGlobalVarInitializationToEntryPoints(IRModule* module, TargetProgram* targetProgram);

/// Move resource-dependent global initializers to the start of each entry point.
///
/// This operation moves only linked, per-invocation initializer targets whose values transitively
/// depend on resource state. The ordinary target-policy operation still runs later for any other
/// initializer that the target cannot represent at global scope.
///
/// `outResourceDependentState` receives the complete state whose initialization semantics depend
/// on an entry point: resource-bearing per-invocation globals, every resource-dependent initializer
/// target (including targets this operation cannot move), and globals that the moved initializer
/// call graph may mutate. A subsequent pass can use this wider set to validate other call roots.
///
/// Invoke this operation after linking, when source globals have linkage decorations and calls
/// between defined functions are direct.
void moveResourceDependentGlobalVarInitializationToEntryPoints(
    IRModule* module,
    TargetProgram* targetProgram,
    List<IRGlobalVar*>& outResourceDependentState);
} // namespace Slang
