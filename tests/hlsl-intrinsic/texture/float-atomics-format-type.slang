//TEST:SIMPLE(filecheck=HLSL): -target hlsl -profile cs_6_6 -entry computeMain
//TEST:SIMPLE(filecheck=METAL): -target metal -stage compute -entry computeMain

// Float-specific texture methods must match the descriptor's payload type.
globallycoherent RWTexture2D<R32F> texture;
RWStructuredBuffer<float> output;

// HLSL: NvInterlockedAddFp32(
// METAL: atomic_fetch_add

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    float originalValue;
    texture.InterlockedAddF32(uint2(0), 1.0f, originalValue);
    output[0] = originalValue;
}
