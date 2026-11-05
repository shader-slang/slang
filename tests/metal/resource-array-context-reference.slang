//TEST:SIMPLE(filecheck=METAL): -target metal -stage compute -entry computeMain -O3
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage compute -entry computeMain -O3

// Runtime indexing must use the entry-point resource array's storage without copying the whole
// array into each invocation's KernelContext. Cover textures, samplers, and descriptor handles;
// scalar resources and mutable thread-local arrays must keep their original representations.

Texture2D<float4> textures[96];
SamplerState samplers[4];
Texture2D<float4>.Handle handleTextures[2];
RWTexture2D<float4> writableTextures[2];
Texture2D<float4> singleTexture;
SamplerState singleSampler;
RWStructuredBuffer<float4> output;
static uint localIndices[4];
static bool alternateSample;

// METAL-LABEL: struct KernelContext
// METAL-DAG: const array<texture2d<float, access::sample>, int(96)> thread* [[TEXTURES:textures_[0-9]+]];
// METAL-DAG: const array<sampler, int(4)> thread* [[SAMPLERS:samplers_[0-9]+]];
// METAL-DAG: const array<texture2d<float, access::sample>, int(2)> thread* [[HANDLES:handleTextures_[0-9]+]];
// METAL-DAG: const array<texture2d<float, access::read_write>, int(2)> thread* [[WRITABLE:writableTextures_[0-9]+]];
// METAL-DAG: texture2d<float, access::sample> singleTexture_{{[0-9]+}};
// METAL-DAG: sampler singleSampler_{{[0-9]+}};
// METAL-DAG: array<uint, int(4)> localIndices_{{[0-9]+}};
// METAL: };
// METAL-LABEL: float4 sampleGlobal_{{[0-9]+}}
// METAL-NOT: array<texture2d<float, access::sample>, int(96)> {{[a-zA-Z_0-9]+}} =

[noinline]
float4 sampleGlobal(uint index)
{
    writableTextures[index % 2][uint2(0)] = float4(index);
    return textures[index % 96].SampleLevel(
        samplers[index % 4], float2(0.5), alternateSample ? 1.0 : 0.0) +
        handleTextures[index % 2].SampleLevel(singleSampler, float2(0.5), 0) +
        singleTexture.SampleLevel(singleSampler, float2(0.5), 0) + localIndices[index % 4];
}

// The physical signature remains a bound array, while the context stores its address.
// METAL-LABEL: {{\[\[}}kernel{{\]\]}} void computeMain
// METAL-DAG: array<texture2d<float, access::read_write>, int(2)> [[WRITABLE_PARAM:writableTextures_[0-9]+]] {{\[\[}}texture(
// METAL-DAG: array<texture2d<float, access::sample>, int(96)> [[TEXTURE_PARAM:textures_[0-9]+]] {{\[\[}}texture(
// METAL-DAG: array<sampler, int(4)> [[SAMPLER_PARAM:samplers_[0-9]+]] {{\[\[}}sampler(
// METAL-DAG: array<texture2d<float, access::sample>, int(2)> [[HANDLE_PARAM:handleTextures_[0-9]+]] {{\[\[}}texture(
// METAL-DAG: ->[[WRITABLE]] = (&[[WRITABLE_PARAM]]);
// METAL-DAG: ->[[TEXTURES]] = (&[[TEXTURE_PARAM]]);
// METAL-DAG: ->[[SAMPLERS]] = (&[[SAMPLER_PARAM]]);
// METAL-DAG: ->[[HANDLES]] = (&[[HANDLE_PARAM]]);
// METALLIB: result code = 0
// METALLIB: define void @computeMain

[shader("compute")]
[numthreads(64, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    localIndices[tid.x % 4] = tid.x;
    alternateSample = (tid.x & 1) != 0;
    output[tid.x] = sampleGlobal(tid.x);
}
