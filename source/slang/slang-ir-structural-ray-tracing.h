#pragma once

#include "slang-ir.h"
#include "slang-structural-ray-tracing.h"

namespace Slang
{

struct IRModule;

struct StructuralRayTracingEntryPointIRInfo
{
    StructuralRayTracingStageKind stageKind = StructuralRayTracingStageKind::Count;
    IRInst* invoke = nullptr;
    IRType* stageType = nullptr;
    IRStringLit* stageSourceTypeName = nullptr;
    IRStringLit* stageTypeIdentity = nullptr;
    IRType* contextType = nullptr;
    IRType* payloadType = nullptr;
    IRType* payloadSemanticType = nullptr;
    IRType* recordType = nullptr;
    IRType* hitAttributesType = nullptr;
    IRType* callableDataType = nullptr;
    StructuralRayTracingHitAttributesKind hitAttributesKind =
        StructuralRayTracingHitAttributesKind::None;
    IRIntegerValue payloadLocation = -1;
};

IROp getStructuralRayTracingStageInterfaceOp(StructuralRayTracingStageKind kind);
IROp getStructuralRayTracingStageInputOperationOp(StructuralRayTracingStageInputOperationKind kind);

/// Returns whether `op` is compiler-owned structural ray-tracing IR.
///
/// The source module never spells these operations directly. Keeping this classification beside
/// the IR definitions gives the parser one exhaustive reservation rule as the lowering grows new
/// metadata or target-only operations.
bool isCompilerOwnedStructuralRayTracingIROp(IROp op);

/// Returns the executable function for a hit-group stage, or null for its canonical placeholder.
///
/// This is the single consumer-side check for the explicit presence bit carried by hit-group
/// metadata. It release-asserts that the accompanying type, source name, and value use the
/// compiler-owned present or absent representation instead of treating malformed metadata as a
/// placeholder.
IRFunc* getStructuralRayTracingHitGroupStageInvoke(
    IRStructuralRayTracingHitGroupInfoDecoration* group,
    StructuralRayTracingStageKind stageKind);

/// Returns whether `type` is a source-semantic empty payload after IR specialization.
///
/// This deliberately reads preserved front-end metadata instead of inferring emptiness from the
/// target layout: an empty derived struct and a wrapper containing an empty field can have similar
/// physical representations but have different source-level payload semantics.
bool isSemanticallyEmptyStructuralRayTracingPayloadType(IRType* type);
void collectUsedVulkanRayPayloadLocations(IRInst* root, HashSet<IRIntegerValue>& outLocations);

/// Adds a linked-program payload assignment to `owner` without decorating the payload type.
///
/// The same nominal type may participate in multiple independently linked programs. Keeping the
/// assignment on the program or schema operation prevents one compilation's Vulkan `Location`
/// choice from becoming global type state. `payloadSemanticType` is the canonical logical type
/// value produced before ABI legalization; it specializes through ordinary IR substitution and is
/// the lookup key. `payloadType` is the realized representation consumed by the native adapter.
void addStructuralRayTracingProgramPayloadLocation(
    IRBuilder& builder,
    IRInst* owner,
    IRType* payloadType,
    IRType* payloadSemanticType,
    IRIntegerValue location);

/// Finds a payload assignment on `owner` by its canonical source-semantic type value.
IRIntegerValue findStructuralRayTracingProgramPayloadLocation(
    IRInst* owner,
    IRType* payloadSemanticType);

void addStructuralRayTracingEntryPointInfo(
    IRBuilder& builder,
    IRFunc* func,
    const StructuralRayTracingEntryPointIRInfo& info);

bool identifyStructuralRayTracingStageInterfaces(
    Module* module,
    const StructuralRayTracingDeclRegistry& registry,
    StructuralRayTracingStageKind* outMissingStage = nullptr);

} // namespace Slang
