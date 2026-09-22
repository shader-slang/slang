// slang-ir-explicit-global-init.h
#pragma once

#include "core/slang-list.h"

namespace Slang
{
struct IRGlobalVar;
struct IRModule;
class TargetProgram;

/// Moves each eligible global initializer selected by the target policy to the start of every
/// defined entry point.
///
/// Selected globals retain their storage but no longer contain initializer bodies. Storage with an
/// independently managed global lifetime is not eligible for per-entry-point initialization.
void moveGlobalVarInitializationToEntryPoints(IRModule* module, TargetProgram* targetProgram);

/// Moves the eligible initializers of linked, per-invocation globals that transitively depend on
/// resource state to the start of every defined entry point.
///
/// On return, `outResourceDependentState` contains the conservative boundary-validation set: the
/// resource-state globals, all resource-dependent initializer targets (including immovable
/// targets), and every global that a moved initializer may mutate. `legalizeResourceGlobalVars`
/// uses this set to reject preserved storage and functions that can reach the state through an
/// invocation without a rewritable direct call.
///
/// This operation must run after linking, when source globals have linkage decorations and calls
/// between defined functions are direct. The ordinary target-policy operation still runs later for
/// any other initializer that the target cannot represent at global scope.
void moveResourceDependentGlobalVarInitializationToEntryPoints(
    IRModule* module,
    TargetProgram* targetProgram,
    List<IRGlobalVar*>& outResourceDependentState);
} // namespace Slang
