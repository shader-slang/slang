// Laying out a `DescriptorHandle` on a target with no profile or capability promotes the
// `descriptor_handle` capability into the target. The promotion must only add what the current
// target supports; if it also added the alias's HLSL branch, types such as
// `BuiltInTriangleIntersectionAttributes` and `RayDesc` would be emitted by their HLSL builtin
// names with no declaration. See shader-slang/slang#13329.

// `-g` keeps the attributes local alive in the CUDA output, which is where the bare name appeared.
//TEST:SIMPLE(filecheck=CUDA): -target cuda -g -entry closestHit -stage closesthit
//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry closestHit -stage closesthit
//TEST:SIMPLE(filecheck=METAL): -target metal -entry computeMain -stage compute

// CUDA: struct BuiltInTriangleIntersectionAttributes_0
// CUDA: BuiltInTriangleIntersectionAttributes_0 _S{{[0-9]+}} =

// GLSL: struct BuiltInTriangleIntersectionAttributes_0
// GLSL: hitAttributeEXT BuiltInTriangleIntersectionAttributes_0

// METAL: struct RayDesc_0
// METAL: RayDesc_0 ray_0

struct Payload
{
    float4 color;
};

uniform DescriptorHandle<RWStructuredBuffer<uint>> gCounter;
uniform RaytracingAccelerationStructure gScene;

[shader("closesthit")]
void closestHit(inout Payload payload, BuiltInTriangleIntersectionAttributes attribs)
{
    gCounter[0] = 1;
    payload.color = float4(attribs.barycentrics, 0.0, 1.0);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    RayDesc ray;
    ray.Origin = float3(0, 0, 0);
    ray.TMin = 0.01;
    ray.Direction = float3(0, 0, 1);
    ray.TMax = 1000.0;

    RayQuery<RAY_FLAG_NONE> query;
    query.TraceRayInline(gScene, RAY_FLAG_NONE, 0xFF, ray);
    query.Proceed();
    gCounter[0] = query.CommittedStatus();
}
