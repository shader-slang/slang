//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry rayGenMain -stage raygeneration -entry closestHitMain -stage closesthit -entry intersectionMain -stage intersection

// Ray-tracing payloads, callable payloads and hit attributes live in storage classes such as
// `RayPayloadKHR`, `CallableDataKHR` and `HitAttributeKHR`, which SPIR-V does not allow to hold a
// pointer in a logical storage class, so any of them holding one is rejected.

struct Payload
{
    Ptr<float, Access.ReadWrite, AddressSpace.GroupShared> data;
    float4 color;
}

struct Attributes
{
    Ptr<float, Access.ReadWrite, AddressSpace.GroupShared> data;
    float2 barycentrics;
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
    // The variables that `TraceRay`, `CallShader` and `ReportHit` pass to the ray-tracing pipeline
    // are declared in the core module, so those errors are reported there.
    TraceRay(scene, 0, 0xff, 0, 1, 0, ray, payload);
//CHECK: logical pointer in shared or external memory
//CHECK: this memory holds a pointer in a logical address space, which SPIR-V only allows in function-local or private variables
    CallShader(0, payload);
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

[shader("intersection")]
void intersectionMain()
{
    Attributes attributes;
    attributes.barycentrics = 0;
    ReportHit(1.0f, 0, attributes);
//CHECK: logical pointer in shared or external memory
//CHECK: this memory holds a pointer in a logical address space, which SPIR-V only allows in function-local or private variables
}
