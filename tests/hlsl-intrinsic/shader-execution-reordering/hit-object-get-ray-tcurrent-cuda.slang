// Test HitObject.GetRayTCurrent for CUDA/OptiX target

//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry rayGenShader -stage raygeneration

//TEST_INPUT: set sceneBVH = AccelerationStructure
uniform RaytracingAccelerationStructure sceneBVH;

//TEST_INPUT:set resultBuffer = out ubuffer(data=[0 0], stride=4)
uniform RWStructuredBuffer<float> resultBuffer;

struct MyPayload
{
    float val;
};

[shader("raygeneration")]
void rayGenShader()
{
    RayDesc ray;
    ray.Origin = float3(0.1, 0.1, 0.1);
    ray.Direction = float3(0.0, 0.0, 1.0);
    ray.TMin = 0.001;
    ray.TMax = 10000.0;

    MyPayload payload = { 0.0 };

    // CUDA: optixTraverse
    HitObject hitobj = HitObject::TraceRay(sceneBVH, RAY_FLAG_NONE, ~0, 0, 0, 0, ray, payload);

    // Test HitObject.GetRayTCurrent() lowered to optixHitObjectGetRayTmax on CUDA/OptiX
    // CUDA: slangOptixHitObjectGetRayTmax
    resultBuffer[0] = hitobj.GetRayTCurrent();
}
