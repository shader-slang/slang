// optix-object-ray-stages.slang
// Test that ObjectRayOrigin/ObjectRayDirection are only available in anyhit/intersection on CUDA
// but remain available in closesthit on other targets.
// Regression test for https://github.com/shader-slang/slang/issues/9258

// Test 1: ObjectRayOrigin in anyhit stage (should work on CUDA)
//TEST:SIMPLE(filecheck=CHECK_ANYHIT): -target cuda -entry anyHitMain -stage anyhit

// Test 2: ObjectRayOrigin in closesthit should work on SPIRV (not CUDA)
//TEST:SIMPLE(filecheck=CHECK_SPIRV): -target spirv -entry closestHitMain -stage closesthit

[raypayload]
struct RayPayload
{
    float3 attribute : read(caller) : write(anyhit, closesthit);
};

// CHECK_ANYHIT: optixGetObjectRayOrigin
[shader("anyhit")]
void anyHitMain(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attr)
{
    payload.attribute = ObjectRayOrigin();
}

// CHECK_SPIRV: ObjectRayOriginKHR
[shader("closesthit")]
void closestHitMain(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attr)
{
    payload.attribute = ObjectRayOrigin();
}
