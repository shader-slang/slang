// Reproducer-based regression test for:
// https://github.com/shader-slang/slang/issues/10092

//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry rayGenMain -stage raygeneration

uniform RaytracingAccelerationStructure sceneBVH;

//TEST_INPUT:set outputBuffer = out ubuffer(data=[0], stride=4)
uniform RWStructuredBuffer<uint> outputBuffer;

[raypayload]
struct Payload
{
    // int someValue : read(caller) : write(caller); // Uncommenting this line avoids the issue.
};

[shader("raygeneration")]
void rayGenMain()
{
    RayDesc ray;
    ray.Origin = float3(0, 0, 0);
    ray.Direction = float3(0, 0, 1);
    ray.TMin = 0;
    ray.TMax = 10000;

    Payload payload = {};

    // CHECK: optixTraverse
    HitObject hit = HitObject::TraceRay(sceneBVH, RAY_FLAG_NONE, ~0, 0, 0, 0, ray, payload);

    outputBuffer[0] = uint(hit.IsHit()) + uint(hit.IsMiss());
}
