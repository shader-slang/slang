// Regression test for https://github.com/shader-slang/slang/issues/13753
// A texture, sampler or acceleration structure DescriptorHandle carried in an interface value is
// packed into an AnyValue as its 64-bit value. Metal spells such a handle as the resource type
// itself, which no cast converts to or from an integer, so the bits are copied through memory by
// the `_slang_reinterpretBits` helper instead of a C-style cast. The run-time round trip is checked
// by descriptor-handle-texture-dispatch-runtime.slang.

//TEST:SIMPLE(filecheck=CHECK): -target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=NOCAST): -target metal -entry computeMain -stage compute
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
    // The handle is still packed into and unpacked from the AnyValue, which is what this test
    // checks. eval() does not trace a ray: in a shader that uses a DescriptorHandle, the Metal
    // output declares a `RayDesc` local without defining the struct (also on master), and Apple's
    // compiler would then fail the metallib lane for that unrelated reason.
    RaytracingAccelerationStructure.Handle scene;
    float4 eval(float2 uv) { return float4(uv, 0, 1); }
}

// Depth, read-write and comparison-sampler handles are spelled differently in MSL
// (`depth2d<...>`, `access::read_write`, `sampler`); they take the same route.
struct DepthLight : ILight
{
    DepthTexture2D.Handle depth;
    SamplerComparisonState.Handle cmp;
    float4 eval(float2 uv) { return float4(depth.SampleCmpLevelZero(cmp, uv, 0.5)); }
}

struct RWLight : ILight
{
    RWTexture2D<float4>.Handle rw;
    float4 eval(float2 uv) { return rw[uint2(uv)]; }
}

struct ConstLight : ILight
{
    float4 c;
    float4 eval(float2 uv) { return c; }
}

StructuredBuffer<TexLight> texLights;
StructuredBuffer<SceneLight> sceneLights;
StructuredBuffer<DepthLight> depthLights;
StructuredBuffer<RWLight> rwLights;
RWStructuredBuffer<float4> outBuf;

ILight makeLight(uint i)
{
    if ((i & 7) == 1)
        return texLights[i];
    if ((i & 7) == 2)
        return sceneLights[i];
    if ((i & 7) == 3)
        return depthLights[i];
    if ((i & 7) == 4)
        return rwLights[i];
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
// NOCAST-NOT: (texture2d<{{.*}}>)(
// NOCAST-NOT: (depth2d<{{.*}}>)(
// NOCAST-NOT: (sampler)(
// NOCAST-NOT: (metal::raytracing::acceleration_structure<{{.*}}>)(

// The helper is defined once, and each handle is packed and unpacked through it. Each check names
// its own field and the handle's MSL type, so none depends on the order in which the marshalling
// functions are emitted.
// CHECK: To _slang_reinterpretBits(From from)
// CHECK-DAG: _slang_reinterpretBits<ulong>({{.*}}tex_{{[0-9]+}})
// CHECK-DAG: tex_{{[0-9]+}} = _slang_reinterpretBits<texture2d<float, access::sample>>(
// CHECK-DAG: _slang_reinterpretBits<ulong>({{.*}}samp_{{[0-9]+}})
// CHECK-DAG: samp_{{[0-9]+}} = _slang_reinterpretBits<sampler>(
// CHECK-DAG: _slang_reinterpretBits<ulong>({{.*}}scene_{{[0-9]+}})
// CHECK-DAG: scene_{{[0-9]+}} = _slang_reinterpretBits<metal::raytracing::acceleration_structure<{{.*}}>>(
// CHECK-DAG: _slang_reinterpretBits<ulong>({{.*}}depth_{{[0-9]+}})
// CHECK-DAG: depth_{{[0-9]+}} = _slang_reinterpretBits<depth2d<float, access::sample>>(
// CHECK-DAG: _slang_reinterpretBits<ulong>({{.*}}cmp_{{[0-9]+}})
// CHECK-DAG: cmp_{{[0-9]+}} = _slang_reinterpretBits<sampler>(
// CHECK-DAG: _slang_reinterpretBits<ulong>({{.*}}rw_{{[0-9]+}})
// CHECK-DAG: rw_{{[0-9]+}} = _slang_reinterpretBits<texture2d<float, access::read_write>>(

// LIB: define void @computeMain
