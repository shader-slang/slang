#pragma once

#include "core/slang-list.h"

namespace Slang
{

struct IRFunc;
struct IRModule;

/// Prepare structural ray-tracing programs for the Metal target before DCE.
///
/// Structural ray-generation entry points are physically emitted as Metal compute kernels. This
/// pass also consumes trace operations whose logical SBT is empty. Later slices extend the same
/// target-owned boundary with function-table and post-trace dispatch lowering.
void prepareMetalStructuralRayTracing(IRModule* module, List<IRFunc*>& entryPoints);

} // namespace Slang
