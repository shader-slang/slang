#pragma once

#include "slang-ir.h"
#include "slang-structural-ray-tracing.h"

// Defines the IR identities emitted from checked structural ray-tracing declarations. Consumers
// recognize stage contracts and source operations without retaining AST declaration pointers.

namespace Slang
{

/// Returns the compiler-owned interface-type opcode for `kind`, or `kIROp_Invalid` for an invalid
/// kind. Distinct opcodes preserve structural stage identity after source declarations are linked
/// or serialized and their AST pointers are no longer available.
IROp getStructuralRayTracingStageInterfaceOp(Stage kind);

/// Returns whether `func` accepts a structural stage-input type.
///
/// For example, `invoke(ClosestHitInput<C>)` keeps the input's KnownBuiltin annotation whether
/// its type is still a generic specialization or has become a concrete struct. Entry-point
/// consumers use this checked signature to recognize that native adapter synthesis is required.
bool hasStructuralRayTracingStageInput(IRFunc* func);

/// Marks `func` as one annotated source operation that structural lowering must consume.
///
/// The marker has no execution semantics of its own. Keeping it on the function, rather than on a
/// particular call spelling, lets ordinary linking and generic specialization preserve the source
/// contract while those passes rewrite calls and clone function bodies.
void addStructuralRayTracingSourceOperation(
    IRBuilder& builder,
    IRFunc* func,
    StructuralRayTracingSourceOperationKind kind);

} // namespace Slang
