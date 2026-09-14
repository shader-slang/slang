#pragma once

#include "slang-ir.h"
#include "slang-structural-ray-tracing.h"

namespace Slang
{

struct IRModule;
class TargetRequest;

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

/// Lowers every schema-carrying program-descriptor type in `module`.
///
/// A schema found in `targetTypesBySchema` maps to its target-specific descriptor type. Every
/// other descriptor maps back to its ordinary source storage representation, including retained
/// generic templates that have no concrete target ABI.
void lowerStructuralRayTracingProgramDescriptorTypes(
    IRModule* module,
    const Dictionary<IRType*, IRType*>& targetTypesBySchema);

/// Diagnoses a selected entry point whose reachable IR combines structural operations with a
/// call marked as a legacy pipeline operation during AST lowering.
///
/// For example, a structural ray-generation entry point can call `RayTracer.trace` and then call an
/// imported helper whose body calls legacy `TraceRay`. The helper's checked AST is not revisited
/// when its serialized IR is linked into the program, so the source call carries an IR marker.
/// This validation follows direct IR calls plus the runtime-dispatch edges that schema lowering
/// records on structural trace and callable operations. It therefore consumes the same executable
/// call graph that target synthesis will lower without treating unrelated imported definitions as
/// reachable.
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
/// OptiX reports the word count selected by its hit-attribute transport. Built-in triangle
/// attributes are handled by schema reflection because they have no equivalent source storage
/// struct.
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

/// Finalizes the empty-payload contract after open schema sections have been completed.
///
/// A finalized schema may serve at most one semantic empty-payload type, regardless of which trace
/// overload happens to activate it. After checking that schema-wide invariant, this operation
/// resolves implicit-empty-payload traces from their producer-owned deferred records and final
/// linked entries. It never reconstructs an AST overload or interprets a generic/function
/// signature by position.
bool finalizeStructuralRayTracingSchemaPayloads(IRModule* module, DiagnosticSink* sink);

} // namespace Slang
