// slang-ir-legalize-varying-params.h
#pragma once
#include "slang-ir-insts.h"

namespace Slang
{

class DiagnosticSink;

struct IRModule;
struct IRInst;
struct IRFunc;
struct IRParam;
struct IRStructField;
struct IRVarLayout;
struct IRVectorType;
struct IRBuilder;
struct IREntryPointDecoration;

struct EntryPointInfo
{
    IRFunc* entryPointFunc;
    IREntryPointDecoration* entryPointDecor;
};

void legalizeEntryPointVaryingParamsForCPU(
    IRModule* module,
    TargetProgram* target,
    DiagnosticSink* sink);

void legalizeEntryPointVaryingParamsForCUDA(IRModule* module, DiagnosticSink* sink);

void legalizeEntryPointVaryingParamsForMetal(
    IRModule* module,
    DiagnosticSink* sink,
    List<EntryPointInfo>& entryPoints);

void legalizeEntryPointVaryingParamsForWGSL(
    IRModule* module,
    DiagnosticSink* sink,
    List<EntryPointInfo>& entryPoints);

void depointerizeInputParams(IRFunc* entryPoint);

// SystemValueSemanticName member definition macro
#define SYSTEM_VALUE_SEMANTIC_NAMES(M)                   \
    M(Position, SV_Position)                             \
    M(ClipDistance, SV_ClipDistance)                     \
    M(CullDistance, SV_CullDistance)                     \
    M(Coverage, SV_Coverage)                             \
    M(InnerCoverage, SV_InnerCoverage)                   \
    M(Depth, SV_Depth)                                   \
    M(DepthGreaterEqual, SV_DepthGreaterEqual)           \
    M(DepthLessEqual, SV_DepthLessEqual)                 \
    M(DispatchThreadID, SV_DispatchThreadID)             \
    M(DomainLocation, SV_DomainLocation)                 \
    M(GroupID, SV_GroupID)                               \
    M(GroupIndex, SV_GroupIndex)                         \
    M(GroupThreadID, SV_GroupThreadID)                   \
    M(GSInstanceID, SV_GSInstanceID)                     \
    M(InstanceID, SV_InstanceID)                         \
    M(IsFrontFace, SV_IsFrontFace)                       \
    M(OutputControlPointID, SV_OutputControlPointID)     \
    M(PointSize, SV_PointSize)                           \
    M(PointCoord, SV_PointCoord)                         \
    M(PrimitiveID, SV_PrimitiveID)                       \
    M(DrawIndex, SV_DrawIndex)                           \
    M(DeviceIndex, SV_DeviceIndex)                       \
    M(RenderTargetArrayIndex, SV_RenderTargetArrayIndex) \
    M(SampleIndex, SV_SampleIndex)                       \
    M(StencilRef, SV_StencilRef)                         \
    M(TessFactor, SV_TessFactor)                         \
    M(VertexID, SV_VertexID)                             \
    M(ViewID, SV_ViewID)                                 \
    M(ViewportArrayIndex, SV_ViewportArrayIndex)         \
    M(Target, SV_Target)                                 \
    M(StartVertexLocation, SV_StartVertexLocation)       \
    M(StartInstanceLocation, SV_StartInstanceLocation)   \
    M(WaveLaneCount, SV_WaveLaneCount)                   \
    M(WaveLaneIndex, SV_WaveLaneIndex)                   \
    M(WaveIndex, SV_WaveIndex)                           \
    M(QuadLaneIndex, SV_QuadLaneIndex)                   \
    M(VulkanVertexID, SV_VulkanVertexID)                 \
    M(VulkanInstanceID, SV_VulkanInstanceID)             \
    M(VulkanSamplePosition, SV_VulkanSamplePosition)     \
    M(Barycentrics, SV_Barycentrics)                     \
/* end */

/// A known system-value semantic name that can be applied to a parameter
///
enum class SystemValueSemanticName
{
    None = 0,
    Unknown = 0,
#define CASE(ID, NAME) ID,
    SYSTEM_VALUE_SEMANTIC_NAMES(CASE)
#undef CASE
};

SystemValueSemanticName convertSystemValueSemanticNameToEnum(String rawSemanticName);

/// Returns true if `stage` is a ray-tracing hit stage where `SV_PrimitiveID` is a
/// runtime-provided builtin instead of a user varying or hit attribute.
bool isRayTracingHitStage(Stage stage);

/// Replaces an `SV_PrimitiveID` hit-stage parameter with the canonical primitive-index helper.
/// Returns true only when the parameter was removed, so callers must capture the next parameter
/// before calling because `param` is invalid after a successful call.
bool tryLegalizeRayTracingPrimitiveIDParam(
    IRModule* module,
    IRBuilder& builder,
    IRParam* param,
    bool* outParamRemoved = nullptr);

/// Emits the replacement value for an `SV_PrimitiveID` field while rewriting a hit-stage
/// struct parameter. `type` is the value type required at the use site, `field` and `layout`
/// identify the source field metadata, and `userData` is backend-owned context.
/// Return null only when the backend cannot materialize a replacement.
using RayTracingPrimitiveIDValueEmitterFunc =
    IRInst* (*)(
        IRModule* module,
        IRBuilder& builder,
        IRType* type,
        IRStructField* field,
        IRVarLayout* layout,
        void* userData);

struct RayTracingPrimitiveIDValueEmitter
{
    /// Optional target-specific emitter. When null, legalization uses the shared primitive-index
    /// helper decorated for the active backend.
    RayTracingPrimitiveIDValueEmitterFunc func = nullptr;
    void* userData = nullptr;
};

/// Rewrites `SV_PrimitiveID` fields in a hit-stage struct parameter.
/// Returns true when primitive-ID fields were rewritten. `outParamRemoved` is set when the
/// original parameter had no ordinary fields and was removed; otherwise the parameter is narrowed
/// to the remaining ordinary fields and must still be legalized by the caller.
bool tryLegalizeRayTracingPrimitiveIDStructParam(
    IRModule* module,
    IRBuilder& builder,
    IRParam* param,
    bool* outParamRemoved = nullptr,
    RayTracingPrimitiveIDValueEmitter const* valueEmitter = nullptr);

/// Legalizes hit-stage `SV_PrimitiveID` parameters before HLSL existential-type-layout
/// lowering removes empty struct parameters that are still needed for this rewrite.
void legalizeRayTracingPrimitiveIDParamsForHLSL(IRModule* module);

} // namespace Slang
