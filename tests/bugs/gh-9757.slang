// Test for issue #9757: InternalError when HitObject::TraceRay payload is an empty struct.
// Empty ray payload structs should compile without crashing.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -emit-spirv-directly -capability spvShaderInvocationReorderNV -entry main -stage raygeneration -skip-spirv-validation

struct Payload{};

RaytracingAccelerationStructure tlas;
RWTexture2D<float> outImage;

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
    if (hitObj.IsHit())
    {
        outImage[DispatchRaysIndex().xy] = 1.0f;
    }

    // CHECK: OpHitObjectTraceRayNV
}
