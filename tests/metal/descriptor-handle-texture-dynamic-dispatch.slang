// Regression test for https://github.com/shader-slang/slang/issues/13753
// A texture, sampler or acceleration structure DescriptorHandle carried in an interface value is
// packed into an AnyValue as its 64-bit value. Metal spells such a handle as the resource type
// itself, which no cast converts to or from an integer, so the bits are copied through memory
// (`*(thread T*)(&value)`) instead of a C-style cast. The run-time round trip is checked by
// descriptor-handle-texture-dispatch-runtime.slang.

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

// Each handle is packed and unpacked by copying its bits through memory. The copied value is a
// named variable, because its address is taken. Each check names its own field and the handle's
// MSL type, so none depends on the order in which the marshalling functions are emitted.
// CHECK-DAG: texture2d<float, access::sample> [[TEX:_S[0-9]+]] = {{.*}}->tex_{{[0-9]+}};
// CHECK-DAG: = *(thread ulong*)(&[[TEX]]);
// CHECK-DAG: tex_{{[0-9]+}} = *(thread texture2d<float, access::sample>*)(&_S{{[0-9]+}});
// CHECK-DAG: sampler [[SAMP:_S[0-9]+]] = {{.*}}->samp_{{[0-9]+}};
// CHECK-DAG: = *(thread ulong*)(&[[SAMP]]);
// CHECK-DAG: samp_{{[0-9]+}} = *(thread sampler*)(&_S{{[0-9]+}});
// CHECK-DAG: metal::raytracing::acceleration_structure<{{.*}}> [[SCENE:_S[0-9]+]] = {{.*}}->scene_{{[0-9]+}};
// CHECK-DAG: = *(thread ulong*)(&[[SCENE]]);
// CHECK-DAG: scene_{{[0-9]+}} = *(thread metal::raytracing::acceleration_structure<{{.*}}>*)(&_S{{[0-9]+}});
// CHECK-DAG: depth2d<float, access::sample> [[DEPTH:_S[0-9]+]] = {{.*}}->depth_{{[0-9]+}};
// CHECK-DAG: = *(thread ulong*)(&[[DEPTH]]);
// CHECK-DAG: depth_{{[0-9]+}} = *(thread depth2d<float, access::sample>*)(&_S{{[0-9]+}});
// CHECK-DAG: sampler [[CMP:_S[0-9]+]] = {{.*}}->cmp_{{[0-9]+}};
// CHECK-DAG: = *(thread ulong*)(&[[CMP]]);
// CHECK-DAG: cmp_{{[0-9]+}} = *(thread sampler*)(&_S{{[0-9]+}});
// CHECK-DAG: texture2d<float, access::read_write> [[RW:_S[0-9]+]] = {{.*}}->rw_{{[0-9]+}};
// CHECK-DAG: = *(thread ulong*)(&[[RW]]);
// CHECK-DAG: rw_{{[0-9]+}} = *(thread texture2d<float, access::read_write>*)(&_S{{[0-9]+}});

// LIB: define void @computeMain
