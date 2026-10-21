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

/// Preserves the selected stage conformance on the logical `invoke` value.
///
/// Consider `MyHit<C>` with `typealias Context = C`. The witness carries that dependent Context
/// and its checked constraints without fixing payload or attribute types before C is supplied.
/// Lower it like any other witness so ordinary linking and specialization resolve its types.
/// A closed generic invoke is an IRSpecialize; entry-point linking transfers this decoration
/// to its resulting IRFunc. Keep the witness off the shared generic body.
void addStructuralRayTracingEntryPointInfo(
    IRBuilder& builder,
    IRInst* entryPointValue,
    IRInst* stageWitness);

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
