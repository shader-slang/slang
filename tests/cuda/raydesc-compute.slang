// https://github.com/shader-slang/slang/issues/12638
// A compute-only CUDA/PTX program must be able to use RayDesc as ordinary data
// without OptiX. The -target ptx lane is load-bearing: it compiles the emitted
// CUDA through NVRTC, so it verifies the prelude actually defines RayDesc for a
// non-OptiX compile. The -target cuda lane alone would not — CUDA-source
// emission succeeds even when the referenced prelude type is missing.

//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK-PTX): -target ptx -entry computeMain -stage compute

// CHECK: RayDesc {{[A-Za-z_0-9]+}};
// CHECK-PTX: .visible .entry{{.*}}computeMain

RWStructuredBuffer<float4> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    RayDesc ray;
    ray.Origin = float3(1.0f, 2.0f, 3.0f);
    ray.TMin = 4.0f;
    ray.Direction = float3(5.0f, 6.0f, 7.0f);
    ray.TMax = 8.0f;

    output[0] = float4(
        ray.Origin.x + ray.Direction.x,
        ray.Origin.y + ray.Direction.y,
        ray.Origin.z + ray.Direction.z,
        ray.TMin + ray.TMax);
}
