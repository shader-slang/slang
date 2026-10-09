// shader-slang/slang#13530: unsized arrays whose elements are resource handles hold no
// ordinary data, so they stay valid as global uniform parameters. Resource legalization
// turns each of them into a descriptor array.

//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry main
//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry main

// CHECK-NOT: E31215
// CHECK: main

struct ResourceOnly
{
    Texture2D t;
    SamplerState s;
}

struct TrailingResources
{
    float4 addend;
    Texture2D t[];
}

// Each level of a nested generic struct is a distinct instantiation of the same declaration.
struct Box<T>
{
    T value;
}

struct Constants
{
    float4 value;
}

struct ResourceBase
{
    Texture2D t;
}
struct DerivedResources : ResourceBase
{
    SamplerState s;
}

uniform ResourceOnly resourceOnly[];
uniform Texture2D textures[];
uniform RWStructuredBuffer<float4> buffers[];
uniform ByteAddressBuffer byteAddressBuffers[];
uniform ConstantBuffer<Constants> constantBuffers[];
uniform RaytracingAccelerationStructure accelerationStructures[];
uniform globallycoherent RWTexture2D<float4> coherentTextures[];
uniform Tuple<Texture2D, SamplerState> resourceTuples[];
uniform Box<Box<Texture2D>> boxedTextures[];
uniform DerivedResources derivedResources[];
uniform TrailingResources trailingResources;

RWStructuredBuffer<float4> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    float4 result = resourceOnly[3].t.SampleLevel(resourceOnly[3].s, float2(0), 0);
    result += textures[2].Load(int3(0));
    result += buffers[1][0];
    result += asfloat(byteAddressBuffers[2].Load4(0)) + constantBuffers[1].value;
    result += coherentTextures[2][int2(0)];
    result += resourceTuples[2]._0.SampleLevel(resourceTuples[1]._1, float2(0), 0);
    result += boxedTextures[1].value.value.Load(int3(0));
    result += derivedResources[2].t.SampleLevel(derivedResources[1].s, float2(0), 0);
    result += trailingResources.addend + trailingResources.t[5].Load(int3(0));

    RayQuery<RAY_FLAG_NONE> query;
    RayDesc ray = {};
    query.TraceRayInline(accelerationStructures[2], 0, 0xff, ray);
    result.x += query.Proceed() ? 1 : 0;

    output[0] = result;
}
