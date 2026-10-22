#pragma once

#include "slang-ir.h"
#include "slang-structural-ray-tracing.h"

// Queries structural stage inputs in IR.

namespace Slang
{

/// Returns whether `func` accepts a structural stage-input type.
///
/// For example, `invoke(ClosestHitInput<C>)` keeps the input's KnownBuiltin annotation whether
/// its type is still a generic specialization or has become a concrete struct. Entry-point
/// consumers use this checked signature to recognize that native adapter synthesis is required.
bool hasStructuralRayTracingStageInput(IRFunc* func);

} // namespace Slang
