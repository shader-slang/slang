// Direct NVVM currently selects texture queries only for scalar Float element textures. A
// neighboring vector-element texture must stop in preflight instead of silently treating its
// resource handle as a supported query overload.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_7_0 -o nvvm-texture-query-unsupported.ptx

Texture2D<float2> inputTexture;
RWStructuredBuffer<uint2> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    uint width;
    uint height;
    inputTexture.GetDimensions(width, height);
    outputBuffer[0] = uint2(width, height);
}

//CHECK: direct NVVM lowering does not support Slang IR instruction or shape 'GenericAsm'
