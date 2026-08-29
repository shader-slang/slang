// Direct NVVM currently selects the unified texture-level intrinsic only for a scalar Float
// element. A neighboring vector-element texture must stop in preflight instead of reaching the
// provider with an invented or truncated result shape.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_7_0 -o nvvm-sampled-texture-unsupported.ptx

Texture2D<float2> inputTexture;
SamplerState samplerState;
RWStructuredBuffer<float2> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = inputTexture.SampleLevel(samplerState, float2(0.0f), 0.0f);
}

//CHECK: direct NVVM lowering does not support Slang IR instruction or shape 'GenericAsm'
