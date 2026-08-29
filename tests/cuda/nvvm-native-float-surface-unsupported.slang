// Float3 has no matching NVVM surface payload row. Preflight must reject the selected resource
// shape rather than padding or decomposing it into a different physical surface operation.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_7_0 -o nvvm-native-float-surface-unsupported.ptx

RWTexture2D<float3> inputTexture;
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    outputBuffer[0] = inputTexture[int2(dispatchThreadID.xy)].x;
}

//CHECK: direct NVVM lowering does not support Slang IR instruction or shape 'helper function parameter'
