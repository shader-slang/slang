// Test that specifying one vendor variant (EXT or NV) doesn't produce
// a spurious warning about the alternative vendor variant.
// Regression test for https://github.com/shader-slang/slang/issues/9756

// Test 1: Specifying EXT should not warn about NV
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK_EXT): -target spirv -capability spvShaderInvocationReorderEXT

// Test 2: Specifying NV should not warn about EXT
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK_NV): -target spirv -capability spvShaderInvocationReorderNV

// CHECK_EXT-NOT: warning 41012{{.*}}spvShaderInvocationReorderNV
// CHECK_EXT-NOT: warning 41012{{.*}}SPV_NV_shader_invocation_reorder

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
