// An unused call of a GLSL subgroup shuffle function (which forwards to the wave intrinsics or
// lowers directly) is removed as dead code (#13910).
//TEST:SIMPLE(filecheck=SHUFFLE_WGSL): -target wgsl -entry shuffleMain -stage compute -allow-glsl
//TEST:SIMPLE(filecheck=SHUFFLE_SPIRV): -target spirv-asm -entry shuffleMain -stage compute -allow-glsl

RWStructuredBuffer<float> outputBuffer;

[numthreads(4, 1, 1)]
void shuffleMain(uint3 tid: SV_DispatchThreadID)
{
    float x = outputBuffer[tid.x];
    float unusedShuffle = subgroupShuffle(x, 1u);
    float unusedXor = subgroupShuffleXor(x, 1u);
    float2 unusedShuffle2 = subgroupShuffle(float2(x, x), 2u);
    float2 usedXor2 = subgroupShuffleXor(float2(x, 1.0), 3u);
    outputBuffer[tid.x] = usedXor2.x;
}

// SHUFFLE_WGSL-LABEL: fn shuffleMain
// SHUFFLE_WGSL-NOT: subgroupShuffle(
// SHUFFLE_WGSL: subgroupShuffleXor(
// SHUFFLE_WGSL-NOT: subgroupShuffleXor(
// SHUFFLE_WGSL-NOT: subgroupShuffle(

// SHUFFLE_SPIRV-NOT: OpGroupNonUniformShuffle {{.*}}
// SHUFFLE_SPIRV: OpGroupNonUniformShuffleXor
// SHUFFLE_SPIRV-NOT: OpGroupNonUniformShuffleXor
// SHUFFLE_SPIRV-NOT: OpGroupNonUniformShuffle {{.*}}
