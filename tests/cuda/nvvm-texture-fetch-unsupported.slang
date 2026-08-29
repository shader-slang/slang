// Direct NVVM deliberately admits the retained CUDA integer-coordinate fetch helpers for 2D, 3D,
// and 2D-array textures only. A neighboring 1D fetch must stop in preflight instead of being
// approximated with a different LLVM texture intrinsic.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_7_0 -O3 -o nvvm-texture-fetch-unsupported.ptx

Texture1D<float> inputTexture;
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
[shader("compute")]
void computeMain()
{
    outputBuffer[0] = inputTexture[0];
}

//CHECK: direct NVVM lowering does not support Slang IR instruction or shape 'GenericAsm'
