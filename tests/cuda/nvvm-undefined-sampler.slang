//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

// The sampler reads are intentional placeholders on CUDA.
#pragma warning(disable: 41016)
#pragma warning(disable: 41035)

// CUDA texture objects own sampling state. The local sampler is only a source-level placeholder.
//TEST_INPUT: Texture2D(size=4, content=gradient):name inputTexture
DescriptorHandle<Texture2D<float4>> inputTexture;
//TEST_INPUT: Texture2D(size=4, content=zero):name zeroTexture
Texture2D<float4> zeroTexture;
//TEST_INPUT: ubuffer(data=[99 99 99 99], stride=4):out,name outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[noinline]
float4 sampleTexture(Texture2D<float4> texture, SamplerState sampler, float2 uv)
{
    return texture.SampleLevel(sampler, uv, 0);
}

[noinline]
float4 sampleHandle(uint64_t handle, float2 uv, uint lane)
{
    Texture2D<float4> texture = DescriptorHandle<Texture2D<float4>>(handle);
    SamplerState first;
    SamplerState second;
    SamplerState selected = (lane & 1) != 0 ? first : second;
    return sampleTexture(texture, selected, uv);
}

[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // Sample centers at all four corners. Only actual bound texture handles are dereferenced.
    float2 uv = float2((tid.x & 1) * 3 + 0.5, (tid.x >> 1) * 3 + 0.5) / 4.0;
    uint failures = any(
        sampleHandle(uint64_t(inputTexture), uv, tid.x) !=
        float4(float(tid.x & 1), float(tid.x >> 1), 0.0, 1.0)) ? 1 : 0;
    SamplerState sampler;
    if (any(sampleTexture(zeroTexture, sampler, uv) != float4(0.0)))
        failures |= 2;
    outputBuffer[tid.x] = failures;
}
//CHECK: 0
//CHECK: 0
//CHECK: 0
//CHECK: 0
