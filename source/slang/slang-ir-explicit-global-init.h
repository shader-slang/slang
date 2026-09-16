// slang-ir-explicit-global-init.h
#pragma once

#include "core/slang-list.h"

namespace Slang
{
struct IRGlobalVar;
struct IRModule;
class TargetProgram;

/// Move initialization logic selected by the target policy onto each entry point.
void moveGlobalVarInitializationToEntryPoints(IRModule* module, TargetProgram* targetProgram);

/// Move resource-global initialization and the global initializers that transitively use it into
/// entry points. The output identifies every resource global, every resource-dependent initializer
/// target, and any ordinary global the resource-dependent initializer call graph may mutate. The
/// resource-global pass uses that set to recognize state that cannot cross an independent call
/// root. The module must contain linked source globals, identified by their linkage decorations,
/// and calls between defined functions must be direct.
void moveResourceDependentGlobalVarInitializationToEntryPoints(
    IRModule* module,
    TargetProgram* targetProgram,
    List<IRGlobalVar*>& outResourceDependentState);
} // namespace Slang
