//TEST:SIMPLE(filecheck=CHECK): -target spirv -emit-spirv-directly -capability spvShaderInvocationReorderEXT -entry main -stage raygeneration -line-directive-mode none

// Test for issue #42: Verify that using only EXT capability doesn't produce warnings
// and emits OpHitObjectTraceRayEXT instead of OpHitObjectTraceRayNV

//TEST_INPUT: set tlas = AccelerationStructure
//TEST_INPUT: set outImage = ubuffer(data=[0 0 0 0], stride=4)

RaytracingAccelerationStructure tlas;
RWTexture2D<float> outImage;

struct Payload{
    uint foo;
};

[shader("raygeneration")]
void main()
{
    RayDesc ray;
    ray.Origin    = float3(0.);
    ray.Direction = float3(1.);
    ray.TMin      = 0.;
    ray.TMax      = 1e30;

    Payload payload;
    HitObject hitObj = HitObject::TraceRay(tlas, 0, 0xFF, 0, 0, 0, ray, payload);
    if(hitObj.IsHit())
    {
        outImage[DispatchRaysIndex().xy] = 1.0f;
    }

    // CHECK-NOT: warning
    // CHECK: OpHitObjectTraceRayEXT
    // CHECK-NOT: OpHitObjectTraceRayNV
}
