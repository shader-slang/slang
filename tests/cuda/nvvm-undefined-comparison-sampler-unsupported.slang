// Comparison samplers have storage support only; ordinary sampler placeholder admission must
// not silently broaden the helper value contract to this distinct type.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_8_0 -O0 -o -
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_8_0 -O3 -o -
#pragma warning(disable: 41016)

RWStructuredBuffer<uint> outputBuffer;

[noinline]
uint useSampler(SamplerComparisonState sampler, uint value)
{
    return value;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    SamplerComparisonState sampler;
    outputBuffer[0] = useSampler(sampler, 7);
}
//CHECK: direct NVVM lowering does not support Slang IR instruction or shape 'helper function parameter: SamplerComparisonState'
