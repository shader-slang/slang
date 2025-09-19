// slang-ir-dce.h
#pragma once

#include "slang-ir-insts.h"

namespace Slang
{
struct IRModule;

struct IRDeadCodeEliminationOptions
{
    bool keepExportsAlive = false;
    bool keepLayoutsAlive = false;
    bool useFastAnalysis = false;
    bool keepGlobalParamsAlive = true;
};

/// Eliminate "dead" code from the given IR module.
///
/// This pass is primarily designed for flow-insensitive
/// "global" dead code elimination (DCE), such as removing
/// types that are unused, functions that are never called,
/// etc.
/// Returns true if changed.
bool eliminateDeadCode(
    IRModule* module,
    IRDeadCodeEliminationOptions const& options = IRDeadCodeEliminationOptions());

bool eliminateDeadCode(
    IRInst* root,
    IRDeadCodeEliminationOptions const& options = IRDeadCodeEliminationOptions());

bool shouldInstBeLiveIfParentIsLive(IRInst* inst, IRDeadCodeEliminationOptions options);

bool isWeakReferenceOperand(IRInst* inst, UInt operandIndex);

bool trimOptimizableTypes(IRModule* module);

/// Eliminate unnecessary load+store pairs when safe to do so.
/// This optimization looks for patterns where a value is loaded from a ConstRef
/// parameter and immediately stored to a temporary variable, then only passed
/// to functions that accept ConstRef parameters. In such cases, the temporary
/// variable can be eliminated and the original ConstRef parameter used directly.
/// Returns true if any changes were made.
bool eliminateLoadStorePairs(IRModule* module);

} // namespace Slang
