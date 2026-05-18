//TEST_CATEGORY(wave, compute)
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -dx12 -profile cs_6_0 -shaderobj -render-feature hardware-device
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -emit-spirv-directly
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -emit-spirv-via-glsl
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-metal -compute -shaderobj
//TEST:SIMPLE(filecheck=CHECK_WGSL): -target wgsl -entry computeMain -stage compute

// Regression test for global-varying builtins that are exposed by generic
// interface specialization after the early global-varying translation pass.
// WGPU runtime coverage is omitted because the `subgroup_id` WGSL language
// extension is not supported by the WGPU parser used in CI yet.

//TEST_INPUT:ubuffer(stride=4, count=64):out,name outputBuffer
RWStructuredBuffer<uint> outputBuffer;

interface IWaveValue
{
    uint getValue();
}

struct WaveValue : IWaveValue
{
    uint getValue()
    {
        return WaveGetLaneIndex() + WaveGetWaveIndex() * WaveGetLaneCount();
    }
}

uint getGenericWaveValue<T: IWaveValue>(T value)
{
    return value.getValue();
}

[shader("compute")]
[numthreads(64, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    WaveValue value;
    uint expected = dispatchThreadID.x;
    outputBuffer[expected] = getGenericWaveValue<WaveValue>(value) == expected ? 1 : 0;
    // CHECK-COUNT-64: 1
    // CHECK_WGSL-DAG: enable subgroup_id;
    // CHECK_WGSL-DAG: @builtin(subgroup_id)
}
