// `RWTexture2D<float>` alone does not say whether the surface stores Float32 or a converting
// format such as R16F. Direct NVVM must reject the unformatted resource instead of selecting a
// physical instruction from the semantic element type.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_7_0 -o nvvm-formatted-surface-requires-format.ptx

RWTexture2D<float> inputTexture;
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    outputBuffer[0] = inputTexture[int2(dispatchThreadID.xy)];
}

//CHECK: direct NVVM lowering does not support Slang IR instruction or shape 'conventional global parameter field address'
