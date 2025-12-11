//TEST:SIMPLE(filecheck=CHECK): -target cuda -stage compute -entry testWaveOps

// Test for issue #8360: spvGroupNonUniformXX capabilities should work with CUDA target

[numthreads(256, 1, 1)]
[require(spvGroupNonUniformBallot, spvGroupNonUniformArithmetic)]
[shader("compute")]
void testWaveOps(uint32_t tid : SV_DispatchThreadID, uniform float* output)
{
    float value = (float)tid;

    // Test WaveActiveMax (requires spvGroupNonUniformArithmetic)
    float maxVal = WaveActiveMax(value);

    // Test WaveIsFirstLane (requires spvGroupNonUniformBallot)
    if (WaveIsFirstLane())
    {
        output[0] = maxVal;
    }
}

// CHECK: extern "C" __global__ void testWaveOps
