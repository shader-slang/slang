//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main

import slang.pipeline_rt;

[raypayload]
struct Payload
{
    float4 color : read(caller)
        : write(caller, closesthit, miss);
};

[shader("raygeneration")]
void rayGeneration(
    uniform RaytracingAccelerationStructure scene,
    uniform RWTexture2D<float4> output)
{
    uint2 pixel = DispatchRaysIndex().xy;
    RayDesc ray;
    ray.Origin = float3(0.0, 0.0, 0.0);
    ray.Direction = float3(0.0, 0.0, 1.0);
    ray.TMin = 0.001;
    ray.TMax = 1000.0;
    Payload payload = { float4(0.0) };
    TraceRay(scene, RAY_FLAG_NONE, 0xff, 0, 0, 0, ray, payload);
    output[pixel] = payload.color;
}

[shader("closesthit")]
void closestHit(inout Payload payload, in BuiltInTriangleIntersectionAttributes attributes)
{
    payload.color = float4(1.0, 0.0, 0.0, 1.0);
}

[shader("miss")]
void miss(inout Payload payload)
{
    payload.color = float4(0.0, 0.0, 1.0, 1.0);
}

[host]
void main(
    StructuredBuffer<float3> vertices,
    StructuredBuffer<uint> indices,
    RWTexture2D<float4> output)
{
    RaytracingAccelerationStructure scene = createRaytracingScene(3, vertices, 3, indices);
    let defaultRayTracer = raytrace(rayGeneration, closestHit, miss);
    let recursiveRayTracer = raytrace(rayGeneration, closestHit, miss, 3);
    defaultRayTracer(uint3(8, 8, 1), scene, output);
    recursiveRayTracer(uint3(8, 8, 1), scene, output);
}

// CHECK-DAG: OpRaytracingAccelerationStructureType
// CHECK-DAG: OpCreateRaytracingScene IndexedTriangles Float3 UInt32
// CHECK-DAG: OpCall {{%[A-Za-z0-9_]+}} {{%[A-Za-z0-9_]+}} {{%[A-Za-z0-9_]+}} {{%[A-Za-z0-9_]+}}
// CHECK-DAG: OpMakeRayTracer {{.*}} 1 {{.*}}
// CHECK-DAG: OpMakeRayTracer {{.*}} 3 {{.*}}
