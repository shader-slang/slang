//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry rayGenMain -stage raygeneration -entry closestHitMain -stage closesthit

// Ray-tracing payloads live in the `RayPayloadKHR` and `IncomingRayPayloadKHR` storage classes,
// which SPIR-V does not allow to hold a pointer in a logical storage class, so a payload holding
// one is rejected.

struct Payload
{
    Ptr<float, Access.ReadWrite, AddressSpace.GroupShared> data;
    float4 color;
}

RaytracingAccelerationStructure scene;
RWStructuredBuffer<float4> outputBuffer;

[shader("raygeneration")]
void rayGenMain()
{
    RayDesc ray;
    ray.Origin = 0;
    ray.Direction = float3(0, 0, 1);
    ray.TMin = 0;
    ray.TMax = 1;
    Payload payload;
    payload.color = 0;
    // The payload variable that `TraceRay` passes to the ray-tracing pipeline is declared in the
    // core module, so the error is reported there.
    TraceRay(scene, 0, 0xff, 0, 1, 0, ray, payload);
//CHECK: logical pointer in shared or external memory
//CHECK: this memory holds a pointer in a logical address space, which SPIR-V only allows in function-local or private variables
    outputBuffer[0] = payload.color;
}

[shader("closesthit")]
void closestHitMain(inout Payload payload, in BuiltInTriangleIntersectionAttributes attributes)
//CHECK:                          ^^^^^^^ logical pointer in shared or external memory
//CHECK:                          ^^^^^^^ this memory holds a pointer in a logical address space, which SPIR-V only allows in function-local or private variables
{
    payload.color = float4(attributes.barycentrics, 0, 1);
}
