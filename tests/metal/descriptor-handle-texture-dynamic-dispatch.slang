// Regression test for https://github.com/shader-slang/slang/issues/13753
// A texture, sampler or acceleration structure DescriptorHandle carried in an interface value is
// packed into an AnyValue as its 64-bit value. Metal spells such a handle as the resource type
// itself, which no cast converts to or from an integer, so the bits must be read and rebuilt through
// a `thread` pointer instead of a C-style cast.

//TEST:SIMPLE(filecheck=CHECK): -target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=NOCAST): -target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=AS): -target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=LIB): -target metallib -entry computeMain -stage compute

interface ILight
{
    float4 eval(float2 uv);
}

struct TexLight : ILight
{
    Texture2D.Handle tex;
    SamplerState.Handle samp;
    float4 eval(float2 uv) { return tex.SampleLevel(samp, uv, 0); }
}

struct SceneLight : ILight
{
    RaytracingAccelerationStructure.Handle scene;
    float4 eval(float2 uv)
    {
        RayDesc ray;
        ray.Origin = float3(uv, 0);
        ray.Direction = float3(0, 0, 1);
        ray.TMin = 0;
        ray.TMax = 1;
        RayQuery<RAY_FLAG_NONE> q;
        q.TraceRayInline(scene, RAY_FLAG_NONE, 0xFF, ray);
        q.Proceed();
        return q.CommittedStatus() == COMMITTED_TRIANGLE_HIT ? float4(1) : float4(0);
    }
}

struct ConstLight : ILight
{
    float4 c;
    float4 eval(float2 uv) { return c; }
}

StructuredBuffer<TexLight> texLights;
StructuredBuffer<SceneLight> sceneLights;
RWStructuredBuffer<float4> outBuf;

ILight makeLight(uint i)
{
    if ((i & 3) == 1)
        return texLights[i];
    if ((i & 3) == 2)
        return sceneLights[i];
    ConstLight c;
    c.c = float4(i);
    return c;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    ILight l = makeLight(tid.x);
    outBuf[tid.x] = l.eval(float2(0.5));
}

// No C-style cast between a texture or sampler and an integer anywhere in the output.
// NOCAST-NOT: (ulong)(
// NOCAST-NOT: (texture2d<float, access::sample>)(
// NOCAST-NOT: (sampler)(
// NOCAST-NOT: (metal::raytracing::acceleration_structure<{{.*}}>)(

// Pack: each handle's bits are read through a `thread ulong` pointer.
// CHECK: texture2d<float, access::sample> _slang_reinterpret_src = {{.*}}tex
// CHECK-NEXT: = *(ulong thread*)(&_slang_reinterpret_src);
// CHECK: sampler _slang_reinterpret_src = {{.*}}samp
// CHECK-NEXT: = *(ulong thread*)(&_slang_reinterpret_src);

// Unpack: each handle is rebuilt from its bits through a pointer to the handle's type.
// CHECK: ulong _slang_reinterpret_src = {{.*}}field0
// CHECK-NEXT: = *(texture2d<float, access::sample> thread*)(&_slang_reinterpret_src);
// CHECK: ulong _slang_reinterpret_src = {{.*}}field2
// CHECK-NEXT: = *(sampler thread*)(&_slang_reinterpret_src);

// The acceleration structure handle takes the same route in both directions.
// AS: metal::raytracing::acceleration_structure<{{.*}}> _slang_reinterpret_src = {{.*}}scene
// AS-NEXT: = *(ulong thread*)(&_slang_reinterpret_src);
// AS: ulong _slang_reinterpret_src = {{.*}}field0
// AS-NEXT: = *(metal::raytracing::acceleration_structure<{{.*}}> thread*)(&_slang_reinterpret_src);

// LIB: define void @computeMain
