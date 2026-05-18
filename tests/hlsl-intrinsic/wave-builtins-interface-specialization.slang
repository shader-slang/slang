//TEST_CATEGORY(wave, compute)
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -dx12 -profile cs_6_0 -shaderobj -render-feature hardware-device
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -emit-spirv-directly
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -emit-spirv-via-glsl
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-wgpu -compute -shaderobj
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-metal -compute -shaderobj

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
