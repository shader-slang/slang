//TEST:SIMPLE(filecheck=BARE): -target metal -entry main_bare -stage compute
//TEST:SIMPLE(filecheck=ARR): -target metal -entry main_arr -stage compute
//TEST:SIMPLE(filecheck=SAMP): -target metal -entry main_samp -stage compute
//TEST:SIMPLE(filecheck=DH): -target metal -entry main_dh -stage compute

// Resources are bindless on Metal, so type legalization keeps a structured
// buffer element that is a resource, an array of resources, or a struct holding
// a `DescriptorHandle`, and the buffer compiles. An element that legalization
// would split, such as a struct with a texture field, is rejected with E38204
// (tests/diagnostics/structuredbuffer-resource-metal.slang).

RWStructuredBuffer<float4> outBuf;
SamplerState samp;
Texture2D tex;

StructuredBuffer<Texture2D> textures;
// BARE: texture2d<float, access::sample> device* textures
// BARE: .sample(

[shader("compute")]
[numthreads(1, 1, 1)]
void main_bare(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = textures[tid.x].SampleLevel(samp, float2(0.5, 0.5), 0);
}

StructuredBuffer<Texture2D[2]> texturePairs;
// ARR: array<texture2d<float, access::sample>, int(2)> device* texturePairs

[shader("compute")]
[numthreads(1, 1, 1)]
void main_arr(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = texturePairs[tid.x][1].SampleLevel(samp, float2(0.5, 0.5), 0);
}

StructuredBuffer<SamplerState> samplers;
// SAMP: sampler device* samplers

[shader("compute")]
[numthreads(1, 1, 1)]
void main_samp(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = tex.SampleLevel(samplers[tid.x], float2(0.5, 0.5), 0);
}

struct Material
{
    DescriptorHandle<Texture2D> t;
}

StructuredBuffer<Material> materials;
// DH: struct Material
// DH: texture2d<float, access::sample> t
// DH: .sample(

[shader("compute")]
[numthreads(1, 1, 1)]
void main_dh(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = materials[tid.x].t.SampleLevel(samp, float2(0.5, 0.5), 0);
}
