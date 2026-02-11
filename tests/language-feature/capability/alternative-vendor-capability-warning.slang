// Test that specifying both vendor variants (NV+EXT) or just NV doesn't
// produce a spurious warning about the alternative vendor variant.
// Regression test for https://github.com/shader-slang/slang/issues/9756

// Test 1: Specifying NV+EXT together should not warn
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK_BOTH): -target spirv -capability spvShaderInvocationReorderNV+spvShaderInvocationReorderEXT

// Test 2: Specifying NV should not warn about EXT (NV is superset of EXT)
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK_NV): -target spirv -capability spvShaderInvocationReorderNV

// CHECK_BOTH-NOT: warning 41012

// CHECK_NV-NOT: warning 41012{{.*}}spvShaderInvocationReorderEXT
// CHECK_NV-NOT: warning 41012{{.*}}SPV_EXT_shader_invocation_reorder

RaytracingAccelerationStructure tlas;
RWTexture2D<float> outImage;

struct Payload
{
    uint foo;
};

[shader("raygeneration")]
void main()
{
    RayDesc ray;
    ray.Origin    = float3(0.0);
    ray.Direction = float3(1.0);
    ray.TMin      = 0.0;
    ray.TMax      = 1e30;

    Payload payload;
    HitObject hitObj = HitObject::TraceRay(tlas, 0, 0xFF, 0, 0, 0, ray, payload);
    if (hitObj.IsHit())
    {
        outImage[DispatchRaysIndex().xy] = 1.0f;
    }
}
