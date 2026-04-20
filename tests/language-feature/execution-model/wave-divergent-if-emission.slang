// Test that wave operations inside divergent if/else branches emit correctly.
// From the docs on divergence/reconvergence:
// "When supported, wave-tangled functions apply only between the mutually convergent
//  thread set by default. That is, synchronization occurs between those threads that
//  are on the same path."
//
// This test verifies correct HLSL and GLSL emission of WaveActiveMin inside
// divergent branches, plus reconvergence with WaveActiveSum after the if/else.

//TEST:SIMPLE(filecheck=CHECK_HLSL): -target hlsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CHECK_GLSL): -target glsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CHECK_SPIRV): -target spirv-asm -stage compute -entry computeMain -emit-spirv-directly

RWStructuredBuffer<int> output;

// CHECK_HLSL: [numthreads(64, 1, 1)]
// CHECK_GLSL: layout(local_size_x = 64

[numthreads(64, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int idx = int(tid.x);
    int result = 0;

    // Divergent branch based on thread index
    // CHECK_HLSL: WaveActiveMin
    // CHECK_GLSL: subgroupMin
    // CHECK_SPIRV: OpGroupNonUniformSMin
    if ((idx & 1) == 0)
    {
        // Even threads: WaveActiveMin over even indices only
        result = WaveActiveMin(idx);
    }
    else
    {
        // Odd threads: WaveActiveMin over odd indices only
        // CHECK_HLSL: WaveActiveMin
        // CHECK_GLSL: subgroupMin
        // CHECK_SPIRV: OpGroupNonUniformSMin
        result = WaveActiveMin(idx * 2);
    }

    // After reconvergence: all threads participate
    // CHECK_HLSL: WaveActiveSum
    // CHECK_GLSL: subgroupAdd
    // CHECK_SPIRV: OpGroupNonUniformIAdd
    int total = WaveActiveSum(result);

    output[idx] = total;
}
