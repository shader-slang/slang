// The first conventional CUDA ABI slice supports one raw RWStructuredBuffer<int> in the
// synthesized global parameter block. Two resources deliberately cross that exact boundary.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_7_0 -o nvvm-conventional-global-unsupported.ptx

RWStructuredBuffer<int> outputBuffer;
RWStructuredBuffer<int> secondBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    outputBuffer[dispatchThreadID.x] = secondBuffer[dispatchThreadID.x];
}

//CHECK: direct NVVM lowering does not support Slang IR instruction or shape 'conventional global parameter field address'
