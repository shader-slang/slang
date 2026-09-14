#pragma once

#include "slang-ir.h"
#include "slang-structural-ray-tracing.h"

namespace Slang
{

struct IRModule;
class TargetRequest;

/// Native OptiX register limits shared by reflection and varying-parameter legalization.
static constexpr IRIntegerValue kOptiXRayTracingRegisterSize = 4;
static constexpr IRIntegerValue kOptiXMaxRayPayloadRegisterCount = 32;
static constexpr IRIntegerValue kOptiXIndirectPayloadRegisterCount = 2;
static constexpr IRIntegerValue kOptiXMinHitAttributeRegisterCount = 2;
static constexpr IRIntegerValue kOptiXMaxHitAttributeRegisterCount = 8;

/// Describes the physical OptiX payload transport selected by varying-parameter legalization.
struct OptiXRayTracingPayloadABIInfo
{
    /// Number of 32-bit OptiX payload registers passed to each native stage.
    IRIntegerValue registerCount = 0;
    /// True when the registers contain a pointer to the payload instead of the payload bytes.
    bool isIndirect = false;
};

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

/// Diagnoses a selected entry point whose reachable IR combines structural operations with a
/// call marked as a legacy pipeline operation during AST lowering.
///
/// For example, a structural ray-generation entry point can call `RayTracer.trace` and then call an
/// imported helper whose body calls legacy `TraceRay`. The helper's checked AST is not revisited
/// when its serialized IR is linked into the program, so the source call carries an IR marker.
/// This validation follows only direct IR calls and therefore consumes the same executable call
/// graph that target synthesis will lower.
void diagnoseMixedRayTracingAPIsInReachableIR(
    IRModule* module,
    List<IRFunc*> const& entryPoints,
    DiagnosticSink* sink);

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

/// Computes the OptiX payload ABI selected for `type`.
///
/// Payloads of at most 32 registers are transported inline according to the compiler's current
/// CUDA IR-layout calculation. Larger payloads use the compiler's two-register pointer
/// representation. Keeping this calculation beside the structural IR utilities gives reflection
/// and varying-parameter legalization one source of truth.
Result getOptiXRayTracingPayloadABIInfo(
    IRBuilder* builder,
    IRType* type,
    OptiXRayTracingPayloadABIInfo* outInfo);

/// Counts 32-bit OptiX attribute registers required by `type`.
///
/// OptiX assigns one register to each scalar leaf, including 8- and 16-bit leaves. This is not a
/// byte-packed data layout, so callers must not derive this value from a general type-layout API.
Result getOptiXRayTracingHitAttributeRegisterCount(
    IRBuilder* builder,
    IRType* type,
    IRIntegerValue* outRegisterCount);

/// Computes the host-visible native ray-payload ABI requirement in bytes for `targetRequest`.
///
/// D3D uses native DXIL aggregate allocation, while Vulkan uses its scalar-aligned block stride.
/// OptiX reports the selected payload register transport, including its two-register pointer
/// fallback. Targets without a native host payload-size setting report zero.
Result getStructuralRayTracingNativePayloadSize(
    TargetRequest* targetRequest,
    IRBuilder* builder,
    IRType* payloadType,
    IRType* payloadSemanticType,
    IRIntegerValue* outSize);

/// Computes one custom hit-attribute type's native host ABI requirement in bytes.
///
/// D3D uses native DXIL aggregate allocation, while Vulkan uses its scalar-aligned block stride.
/// OptiX uses one 32-bit register per scalar leaf. Built-in triangle attributes are handled by
/// schema reflection because they have no equivalent source storage struct.
Result getStructuralRayTracingNativeHitAttributeSize(
    TargetRequest* targetRequest,
    IRBuilder* builder,
    IRType* attributesType,
    IRIntegerValue* outSize);

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

/// Completes every reachable open schema operation from retained linked conformance tables.
///
/// The linker calls this after ordinary IR specialization and before payload-location assignment
/// or target synthesis. Listed metadata remains in source order; matching linked entries are
/// deduplicated by their canonical semantic type identity, sorted by qualified source name, and
/// appended with dense indices. Returns false after emitting any invalid-tag diagnostic.
bool completeOpenStructuralRayTracingSchemas(IRModule* module, DiagnosticSink* sink);

/// Resolves implicit-empty-payload traces whose hit or miss section was open at source lowering.
///
/// Open-section completion must run first so each trace owns its final linked entry metadata. The
/// resolver consumes only the producer-owned deferred record and those final entries; it never
/// reconstructs an AST overload or interprets a generic/function signature by position.
bool resolveDeferredStructuralRayTracingEmptyPayloads(IRModule* module, DiagnosticSink* sink);

} // namespace Slang
