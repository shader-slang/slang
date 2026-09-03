// LLVM's unified texture intrinsic returns one, two, or four selected lanes. A neighboring
// three-lane texture must stop in preflight instead of reaching the provider with an invented or
// truncated result shape.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_7_0 -o nvvm-sampled-texture-unsupported.ptx

Texture2D<float3> inputTexture;
SamplerState samplerState;
RWStructuredBuffer<float3> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = inputTexture.SampleLevel(samplerState, float2(0.0f), 0.0f);
}

//CHECK: direct NVVM lowering does not support Slang IR instruction or shape 'GenericAsm assembly=tex2DLod<$T0>($0, ($2).x, ($2).y, ($3)), signature=vector<float,3>(Texture2D, SamplerState, vector<float,2>, float)'
