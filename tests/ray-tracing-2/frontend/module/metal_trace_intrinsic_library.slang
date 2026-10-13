//TEST_IGNORE_FILE:

module metal_trace_intrinsic_library;

namespace traceir
{
public interface IProgramShape {}

__magic_type(TraceProgramDescriptorType)
__intrinsic_type(TraceProgramDescriptor)
public struct ProgramHandle<Schema : IProgramShape>
{
    private __init(int unavailable) {}
}

// Ordinary source declarations must lower through the same intrinsic mechanism as the standard
// ray-tracing module. The two signatures encode motion by the presence of the final time operand.
__intrinsic_op(metalTraceRay)
public void trace<Schema : IProgramShape, Payload>(
    RaytracingAccelerationStructure scene,
    ProgramHandle<Schema> program,
    uint rayFlags,
    uint instanceMask,
    uint sbtOffset,
    uint sbtStride,
    uint missIndex,
    float3 rayOrigin,
    float rayTMin,
    float3 rayDirection,
    float rayTMax,
    inout Payload payload);

__intrinsic_op(metalTraceRay)
public void traceMotion<Schema : IProgramShape, Payload>(
    RaytracingAccelerationStructure scene,
    ProgramHandle<Schema> program,
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
    float time);
}
