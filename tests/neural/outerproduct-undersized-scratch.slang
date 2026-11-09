//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute -experimental-feature
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target spirv -entry computeMain -stage compute -experimental-feature

import slang.neural;

public struct UndersizedScratch : ISharedMemorySize
{
    public static const uint Bytes = 8192;
}

uniform RWStructuredBuffer<half>.Handle gradients;

[shader("compute")]
[numthreads(64, 1, 1)]
void computeMain()
{
    half input[64] = {};
    half dOut[64] = {};
    neural_unittest.testOuterProductAccumulate<half, 64, 64, UndersizedScratch, 2>(
        input, dOut, gradients);
}

// CHECK: static assertion failed
// CHECK: neural_unittest.testOuterProductAccumulateImpl
