//TEST_IGNORE_FILE:

module metal_trace_intrinsic_library;

import slang.raytracing;

namespace traceir
{
// This compiler-internal fixture tests low-level intrinsic-call lowering. Applications use
// RayTracer.trace, whose ordinary function call remains in frontend IR. Repeating the internal
// operation signatures lets this test inspect the instruction before backend lowering; the
// descriptor type, schema constraints, and resource rules all come from slang.raytracing.
__intrinsic_op(metalTraceRay)
public void trace<Schema : rt::ITraceProgramSchema, Payload>(
    RaytracingAccelerationStructure scene,
    rt::TraceProgramDescriptor<Schema> program,
    uint rayFlags,
    uint instanceMask,
    uint sbtOffset,
    uint sbtStride,
    uint missIndex,
    float3 rayOrigin,
    float rayTMin,
    float3 rayDirection,
    float rayTMax,
    inout Payload payload)
    where Schema.TraceContext.Motion == rt::NoMotion;

__intrinsic_op(metalTraceRay)
public void traceMotion<Schema : rt::ITraceProgramSchema, Payload>(
    RaytracingAccelerationStructure scene,
    rt::TraceProgramDescriptor<Schema> program,
    uint rayFlags,
    uint instanceMask,
    uint sbtOffset,
    uint sbtStride,
    uint missIndex,
    float3 rayOrigin,
    float rayTMin,
    float3 rayDirection,
    float rayTMax,
    inout Payload payload,
    float time)
    where Schema.TraceContext.Motion : rt::IEnabledRayMotion;
}
