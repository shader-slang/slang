//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main

[raypayload]
struct Payload
{
    float4 color : read(caller) : write(caller, closesthit, miss);
};

[shader("raygeneration")]
void rayGeneration(uniform RWTexture2D<float4> output)
{
    output[DispatchRaysIndex().xy] = float4(0.25, 0.5, 0.75, 1.0);
}

[shader("closesthit")]
void closestHit(inout Payload payload, in BuiltInTriangleIntersectionAttributes attributes)
{
    payload.color = float4(1.0, attributes.barycentrics, 1.0);
}

[shader("miss")]
void miss(inout Payload payload)
{
    payload.color = float4(0.0, 0.0, 1.0, 1.0);
}

[host]
void main(RWTexture2D<float4> output)
{
    raytrace(rayGeneration, closestHit, miss)(uint3(8, 8, 1), output);
}

// CHECK-NOT: OpRaytracingAccelerationStructureType
// CHECK: OpCall
// CHECK: OpMakeRayTracer
// CHECK-NOT: OpRaytracingAccelerationStructureType
