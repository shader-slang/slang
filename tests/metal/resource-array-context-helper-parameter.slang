//TEST:SIMPLE(filecheck=METAL): -target metal -stage compute -entry computeMain -O3
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage compute -entry computeMain -O3
//TEST:SIMPLE(filecheck=MUTABLE): -target metal -stage compute -entry computeCopy -O3

// Borrowing a resource array in a helper still uses a pointer parameter, even if address-space
// specialization discovers that its referent is a by-value entry-point parameter's storage.
// Only the entry point itself has the physical by-value ABI.

Texture2D<float4> textures[96];
SamplerState linearSampler;
Texture2D<float4> replacementTexture;
RWStructuredBuffer<float4> output;

// METAL: __attribute__((noinline))
// METAL-NEXT: float4 sampleArray_{{[0-9]+}}(const array<texture2d<float, access::sample>, int(96)> thread* [[HELPER_ARRAY:arrayTextures_[0-9]+]]
// METAL: [[HELPER_ARRAY]]

[noinline]
float4 sampleArray(__constref Texture2D<float4> arrayTextures[96], uint index)
{
    return arrayTextures[index % 96].SampleLevel(linearSampler, float2(0.5), 0);
}

// METAL: {{\[\[}}kernel{{\]\]}} void computeMain
// METAL-SAME: array<texture2d<float, access::sample>, int(96)> [[ENTRY_ARRAY:textures_[0-9]+]] {{\[\[}}texture(
// METAL-NOT: thread array<texture2d<float, access::sample>, int(96)>
// METAL: sampleArray_{{[0-9]+}}((&[[ENTRY_ARRAY]]),
// METALLIB: result code = 0
// METALLIB: define void @computeMain

[shader("compute")]
[numthreads(64, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    output[tid.x] = sampleArray(textures, tid.x);
}

// A source-level mutable copy has independent handles and must remain a local array.
// MUTABLE: thread array<texture2d<float, access::sample>, int(96)> [[LOCAL:localTextures_[0-9]+]]
// MUTABLE: [[LOCAL]][
// MUTABLE: sampleArray_{{[0-9]+}}(&[[LOCAL]],

[shader("compute")]
[numthreads(64, 1, 1)]
void computeCopy(uint3 tid : SV_DispatchThreadID)
{
    Texture2D<float4> localTextures[96] = textures;
    localTextures[tid.x % 96] = replacementTexture;
    output[tid.x] = sampleArray(localTextures, tid.x) + sampleArray(textures, tid.x);
}
