// Test for issues #9756 and #9936: SER NV/EXT capability hierarchy fix.
// Verify that using EXT capability emits EXT instructions and NOT NV instructions.

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

    // CHECK: OpCapability ShaderInvocationReorderEXT
    // CHECK: OpExtension "SPV_EXT_shader_invocation_reorder"
    // CHECK: OpTypeHitObjectEXT
    // CHECK: OpHitObjectTraceRayEXT
    // CHECK-NOT: OpHitObjectTraceRayNV
    // CHECK-NOT: OpTypeHitObjectNV
}
