#pragma once

#include "core/slang-list.h"

namespace Slang
{

struct IRFunc;
struct IRModule;
class DiagnosticSink;

enum class MetalStructuralRayTracingTag : UInt
{
    Instancing = 1 << 0,
    TriangleData = 1 << 1,
    CurveData = 1 << 2,
    WorldSpaceData = 1 << 3,
    PrimitiveMotion = 1 << 4,
    InstanceMotion = 1 << 5,
    ExtendedLimits = 1 << 6,
};

enum class MetalStructuralRayTracingStageRequirement : UInt
{
    None = 0,
    HitAttributes = 1 << 0,
    TriangleBarycentricCoord = 1 << 1,
    TriangleFrontFacing = 1 << 2,
    CurveParameter = 1 << 3,
    Distance = 1 << 4,
    HitKind = 1 << 5,
    WorldSpaceOrigin = 1 << 6,
    WorldSpaceDirection = 1 << 7,
    Record = 1 << 8,
    CallableDispatch = 1 << 9,
};

enum class MetalStructuralRayTracingGeometryKind : UInt
{
    Unknown,
    Triangle,
    Curve,
    BoundingBox,
};

/// Prepare structural ray-tracing programs for the Metal target before DCE.
///
/// Structural ray-generation entry points are physically emitted as Metal compute kernels. This
/// pass also consumes trace operations whose logical SBT is empty. Later slices extend the same
/// target-owned boundary with function-table and post-trace dispatch lowering.
void prepareMetalStructuralRayTracing(
    IRModule* module,
    List<IRFunc*>& entryPoints,
    DiagnosticSink* sink);

} // namespace Slang
