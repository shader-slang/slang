// The SDK9 snapshot representation must not leak into an SDK8 compilation.
//TEST:SIMPLE(filecheck=BAD): -target ptx -entry main -stage raygeneration -emit-cuda-via-nvvm -capability cuda_sm_8_0 -optix-version 80000
//TEST:SIMPLE(filecheck=BAD): -target ptx -entry main -stage raygeneration -emit-cuda-via-nvvm -capability cuda_sm_8_0 -optix-version 80100
//TEST:SIMPLE(filecheck=PTX): -target ptx -entry main -stage raygeneration -emit-cuda-via-nvvm -capability cuda_sm_8_0 -optix-version 90000
// BAD: result code = {{-?[1-9][0-9]*}}
// BAD: OptiX 8 HitObject operations
// PTX: .entry __raygen__main

RaytracingAccelerationStructure scene;
RWStructuredBuffer<uint> output;

[shader("raygeneration")]
void main()
{
    uint payload = 0;
    RayDesc ray = { float3(0, 0, -1), 0, float3(0, 0, 1), 10 };
    HitObject object = HitObject.TraceRay(scene, RAY_FLAG_NONE, 255, 0, 1, 0, ray, payload);
    output[0] = object.IsHit();
}
