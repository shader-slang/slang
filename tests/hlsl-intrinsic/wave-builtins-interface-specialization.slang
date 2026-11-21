//TEST_CATEGORY(wave, compute)
//TEST:COMPARE_COMPUTE(targets=dx12, filecheck-buffer=CHECK):-profile cs_6_0 -render-feature hardware-device
//TEST:COMPARE_COMPUTE(targets=vk+glsl+metal+cuda, filecheck-buffer=CHECK):
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute
// Keep DX11 explicit so slang-test does not synthesize a runtime clone for
// wave intrinsics that require SM6-era support.
// Keep LLVM explicit so slang-test does not synthesize an LLVM runtime clone
// for GPU subgroup builtins.

// Regression test for global-varying builtins that are exposed by generic
// interface specialization after the early global-varying translation pass.

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

// WGSL-DAG: enable subgroups;
// WGSL-DAG: @builtin(subgroup_invocation_id)
// WGSL-DAG: @builtin(subgroup_id)

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
}
