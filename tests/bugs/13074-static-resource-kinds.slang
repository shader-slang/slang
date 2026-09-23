//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry main -stage compute -validate-ir
//TEST:SIMPLE: -target spirv -entry main -stage compute -validate-ir -o 13074-static-resource-kinds.spv

//CHECK-NOT: cached
//CHECK: Texture2D<float4 >   [[TEXTURES:textures_[0-9]+]]
//CHECK: SamplerState [[SAMPLER:textureSampler_[0-9]+]]
//CHECK: ByteAddressBuffer [[BYTES:bytes_[0-9]+]]
//CHECK-LABEL: void main()
//CHECK: [[TEXTURES]][int(0)][int(0)].SampleLevel([[SAMPLER]]
//CHECK-SAME: asfloat([[BYTES]].Load4(0U))
//CHECK-NOT: cached

Texture2D<float4> textures[1][1];
SamplerState textureSampler;
ByteAddressBuffer bytes;
RWStructuredBuffer<float4> output;

static Texture2D<float4> cachedTextures[1][1] = textures;
static SamplerState cachedSampler = textureSampler;
static ByteAddressBuffer cachedBytes = bytes;

[numthreads(1, 1, 1)]
void main()
{
    float4 texel = cachedTextures[0][0].SampleLevel(cachedSampler, float2(0.0), 0.0);
    output[0] = texel + asfloat(cachedBytes.Load4(0));
}
