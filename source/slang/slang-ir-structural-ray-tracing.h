#pragma once

#include "slang-ir.h"
#include "slang-structural-ray-tracing.h"

// Defines the IR identities emitted from checked structural ray-tracing declarations. Consumers
// recognize stage contracts and source operations without retaining AST declaration pointers.

namespace Slang
{

/// Carries the checked source contract for one selected structural stage into IR.
///
/// This record does not define a target ABI. It preserves the semantic types that a later adapter
/// synthesis pass will need, even after the AST entry-point object is no longer available.
struct StructuralRayTracingEntryPointIRInfo
{
    StructuralRayTracingStageKind stageKind = StructuralRayTracingStageKind::Count;
    IRType* stageType = nullptr;
    IRStringLit* stageSourceTypeName = nullptr;
    IRStringLit* stageTypeIdentity = nullptr;
    IRType* contextType = nullptr;
    IRType* payloadType = nullptr;
    IRType* recordType = nullptr;
    IRType* hitAttributesType = nullptr;
    IRType* callableDataType = nullptr;
    StructuralRayTracingHitAttributesKind hitAttributesKind =
        StructuralRayTracingHitAttributesKind::None;
};

/// Returns the compiler-owned interface-type opcode for `kind`, or `kIROp_Invalid` for an invalid
/// kind. Distinct opcodes preserve structural stage identity after source declarations are linked
/// or serialized and their AST pointers are no longer available.
IROp getStructuralRayTracingStageInterfaceOp(StructuralRayTracingStageKind kind);

/// Attaches `info` to the selected logical `invoke` value.
///
/// A closed generic stage is represented by an `IRSpecialize` until entry-point linking eagerly
/// specializes it to an `IRFunc`. Accepting either representation keeps the checked declaration
/// specialization intact; the decoration owner is therefore the sole identity of the logical
/// `invoke`, and the linker transfers it to the resulting function. The decoration is intentionally
/// non-operational in this frontend slice. Its complete operand shape is established here so later
/// target-neutral and target-specific passes can consume one stable serialized representation of
/// the checked source contract.
void addStructuralRayTracingEntryPointInfo(
    IRBuilder& builder,
    IRInst* entryPointValue,
    const StructuralRayTracingEntryPointIRInfo& info);

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
