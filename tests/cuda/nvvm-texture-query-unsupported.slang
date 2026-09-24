// Floating-point outputs need numeric conversion; mip and multisample queries have separate
// prelude contracts. They remain outside direct NVVM's non-mip integer dimension query domain.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target ptx -emit-cuda-via-nvvm -entry floatDimensions -stage compute -capability cuda_sm_8_0 -O0 -o nvvm-texture-query-unsupported.ptx
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target ptx -emit-cuda-via-nvvm -entry floatDimensions -stage compute -capability cuda_sm_8_0 -O3 -o nvvm-texture-query-unsupported.ptx
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target ptx -emit-cuda-via-nvvm -entry mipDimensions -stage compute -capability cuda_sm_8_0 -O0 -o nvvm-texture-query-unsupported.ptx
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target ptx -emit-cuda-via-nvvm -entry mipDimensions -stage compute -capability cuda_sm_8_0 -O3 -o nvvm-texture-query-unsupported.ptx
//DIAGNOSTIC_TEST:SIMPLE(diag=MS): -target ptx -emit-cuda-via-nvvm -entry multisampleDimensions -stage compute -capability cuda_sm_8_0 -O0 -o nvvm-texture-query-unsupported.ptx
//DIAGNOSTIC_TEST:SIMPLE(diag=MS): -target ptx -emit-cuda-via-nvvm -entry multisampleDimensions -stage compute -capability cuda_sm_8_0 -O3 -o nvvm-texture-query-unsupported.ptx

Texture2D<float2> inputTexture;
Texture2DMS<uint> multisampleTexture;
RWStructuredBuffer<float2> outputBuffer;

[numthreads(1, 1, 1)]
void floatDimensions()
{
    float width;
    float height;
    inputTexture.GetDimensions(width, height);
    outputBuffer[0] = float2(width, height);
}

[numthreads(1, 1, 1)]
void mipDimensions()
{
    uint width, height, levels;
    inputTexture.GetDimensions(0, width, height, levels);
    outputBuffer[0] = float2(width, height + levels);
}

[numthreads(1, 1, 1)]
void multisampleDimensions()
{
    uint width, height, samples;
    multisampleTexture.GetDimensions(width, height, samples);
    outputBuffer[0] = float2(width, height + samples);
}

//CHECK: direct NVVM lowering does not support Slang IR instruction or shape 'GenericAsm assembly=
//MS: direct NVVM lowering does not support
