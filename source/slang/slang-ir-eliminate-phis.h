// slang-ir-eliminate-phis.h
#pragma once

#include "slang-ir-liveness.h"

namespace Slang
{
struct CodeGenContext;
struct IRModule;

// Controls which block parameters are lowered to mutable temporary variables.
struct PhiEliminationOptions
{
    // Direct SPIR-V can emit ordinary scalar/vector parameters as OpPhi. Keep other types
    // in memory, including pointers that would otherwise require VariablePointers support.
    bool preserveScalarAndVectorPhis = false;
    bool useRegisterAllocation = true;
    // Returns whether a block parameter of this type needs a temporary. The register allocator
    // uses the same decision so that retained parameters never acquire mutable storage.
    bool shouldEliminatePhiForType(IRType* type) const;

    static PhiEliminationOptions getFast() { return PhiEliminationOptions{false, false}; }
};

/// Eliminate the "phi nodes" selected by `options` from the given `module`.
///
/// This process replaces selected block parameters with mutable storage, suitable for targets
/// that cannot emit those parameters in SSA form. Unselected parameters and their branch
/// arguments remain in SSA form.
///
/// If livenessMode is enabled LiveRangeStarts will be inserted into the module.
void eliminatePhis(LivenessMode livenessMode, IRModule* module, PhiEliminationOptions options);

// Overload that takes IRModule* first for use with SLANG_PASS macro
void eliminatePhis(IRModule* module, LivenessMode livenessMode, PhiEliminationOptions options);

void eliminatePhisInFunc(
    LivenessMode livenessMode,
    IRModule* module,
    IRGlobalValueWithCode* func,
    PhiEliminationOptions options);
} // namespace Slang
