// slang-ir-legalize-varying-params.h
#pragma once
#include "slang-ir-insts.h"

namespace Slang
{

class DiagnosticSink;

struct IRModule;
struct IRInst;
struct IRFunc;
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

void legalizeOptiXReportIntersectionsForCUDA(IRModule* module, DiagnosticSink* sink);

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

/// Return whether WGSL's uniformity analysis treats the built-in input that `semantic` maps to as
/// uniform across a whole workgroup of a `stage` shader, so that control flow depending only on it
/// stays uniform for subgroup operations and workgroup barriers alike.
///
/// The WGSL specification's "Uniformity Rules for Expressions" lists `workgroup_id`,
/// `num_workgroups`, `subgroup_size` and `num_subgroups` as uniform built-in inputs of a compute
/// shader. Of these, only `workgroup_id` (`SV_GroupID`) and `subgroup_size` (`SV_WaveLaneCount`)
/// have a Slang system-value semantic. `subgroup_id` (`SV_WaveIndex`) is uniform only within a
/// subgroup, and outside compute shaders none of these values is uniform. Other targets can differ;
/// for example, whether the lane count is workgroup-uniform on Vulkan depends on subgroup-size
/// control.
bool isWGSLWorkgroupUniformSystemValue(SystemValueSemanticName semantic, Stage stage);

} // namespace Slang
