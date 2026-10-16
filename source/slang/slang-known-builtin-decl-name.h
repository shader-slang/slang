#pragma once

#include <stdint.h>

namespace Slang
{
/// Identifies declarations with compiler-defined meaning independently of their source names.
enum class KnownBuiltinDeclName : uint32_t
{
    GeometryStreamAppend,
    GeometryStreamRestart,
    GetAttributeAtVertex,
    DispatchMesh,
    saturated_cooperation,
    saturated_cooperation_using,
    IDifferentiable,
    IDifferentiablePtr,
    IForwardDifferentiable,
    IBackwardDifferentiable,
    IBwdCallable,
    NullDifferential,
    OperatorAddressOf,
    WaveIsFirstLane,
    WaveReadLaneFirst,
    CallShader,
    ReportHit = 16,
    RayTracingClosestHitInput,
    RayTracingAnyHitInput,
    RayTracingIntersectionInput,
    RayTracingMissInput,
    RayTracingCallableInput,
    RayTracingTrianglePrimitive,
    RayTracingCurvePrimitive,
    RayTracingStageContext,
    RayTracingStageRecord,
    RayTracingPayloadContextPayload,
    RayTracingHitPrimitive,
    RayTracingPrimitiveAttributes,
    RayTracingCallableData,
    RayTracingProgramHitGroups,
    RayTracingProgramMissShaders,
    RayTracingProgramCallableShaders,
    COUNT
};
} // namespace Slang
