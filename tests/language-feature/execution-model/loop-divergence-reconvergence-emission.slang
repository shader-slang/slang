// Test wave operations with loop divergence and reconvergence.
// From the docs: "Divergence occurs when some threads exit a loop while the rest
// continue. Reconvergence occurs when all threads have exited the loop."
//
// Verifies that wave ops emit correctly after a loop where threads iterate
// different numbers of times (causing divergence), then reconverge.

//TEST:SIMPLE(filecheck=CHECK_HLSL): -target hlsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CHECK_GLSL): -target glsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CHECK_SPIRV): -target spirv-asm -stage compute -entry computeMain -emit-spirv-directly

RWStructuredBuffer<int> output;

[numthreads(128, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int idx = int(tid.x);

    // Each thread iterates a different number of times based on its index.
    // This causes divergence within the loop.
    int numLoops = 10 + (idx & 3);  // 10, 11, 12, or 13 iterations
    int acc = 0;
    for (int i = 0; i < numLoops; ++i)
    {
        acc += i;
    }

    // After the loop, all threads have reconverged.
    // Wave operation should include all threads in the wave.
    // CHECK_HLSL: WaveActiveSum
    // CHECK_GLSL: subgroupAdd
    // CHECK_SPIRV: OpGroupNonUniformIAdd
    int waveTotal = WaveActiveSum(acc);

    // CHECK_HLSL: WaveActiveMax
    // CHECK_GLSL: subgroupMax
    // CHECK_SPIRV: OpGroupNonUniformSMax
    int waveMax = WaveActiveMax(acc);

    output[idx] = waveTotal + waveMax;
}
