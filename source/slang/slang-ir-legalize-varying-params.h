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

void legalizeEntryPointVaryingParamsForCPU(IRModule* module, DiagnosticSink* sink);

void legalizeEntryPointVaryingParamsForCUDA(IRModule* module, DiagnosticSink* sink);


// (#4375) Once `slang-ir-metal-legalize.cpp` is merged with
// `slang-ir-legalize-varying-params.cpp`, move the following
// below into `slang-ir-legalize-varying-params.cpp` as well

IRInst* emitCalcGroupExtents(IRBuilder& builder, IRFunc* entryPoint, IRVectorType* type);

IRInst* emitCalcGroupIndex(IRBuilder& builder, IRInst* groupThreadID, IRInst* groupExtents);

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
    M(PrimitiveID, SV_PrimitiveID)                       \
    M(DrawIndex, SV_DrawIndex)                           \
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

} // namespace Slang
