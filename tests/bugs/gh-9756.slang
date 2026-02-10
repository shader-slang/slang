// Test for issues #9756 and #9936: SER NV/EXT capability hierarchy fix.
// Verify that using EXT capability emits EXT instructions (not NV) and produces no warnings.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -emit-spirv-directly -capability spvShaderInvocationReorderEXT -entry main -stage raygeneration -line-directive-mode none

struct Payload
{
    uint data;
};

RaytracingAccelerationStructure tlas;

[shader("raygeneration")]
void main()
{
    RayDesc ray;
    ray.Origin = float3(0.0);
    ray.Direction = float3(0.0, 0.0, 1.0);
    ray.TMin = 0.0;
    ray.TMax = 1e30;

    Payload payload;
    HitObject hitObj = HitObject::TraceRay(tlas, 0, 0xFF, 0, 0, 0, ray, payload);

    // CHECK-NOT: warning
    // CHECK: OpHitObjectTraceRayEXT
    // CHECK-NOT: OpHitObjectTraceRayNV
}
