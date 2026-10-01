// Mip/count and multisample queries retain separate unsupported resource contracts.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target ptx -emit-cuda-via-nvvm -entry mipDimensions -stage compute -capability cuda_sm_8_0 -O0 -o nvvm-texture-query-unsupported.ptx
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target ptx -emit-cuda-via-nvvm -entry mipDimensions -stage compute -capability cuda_sm_8_0 -O3 -o nvvm-texture-query-unsupported.ptx
//DIAGNOSTIC_TEST:SIMPLE(diag=MS): -target ptx -emit-cuda-via-nvvm -entry multisampleDimensions -stage compute -capability cuda_sm_8_0 -O0 -o nvvm-texture-query-unsupported.ptx
//DIAGNOSTIC_TEST:SIMPLE(diag=MS): -target ptx -emit-cuda-via-nvvm -entry multisampleDimensions -stage compute -capability cuda_sm_8_0 -O3 -o nvvm-texture-query-unsupported.ptx

Texture2D<float2> inputTexture;
Texture2DMS<uint> multisampleTexture;
RWStructuredBuffer<float2> outputBuffer;

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

//CHECK: unavailable features in entry point
//CHECK: entrypoint 'mipDimensions' uses features that are not available in 'compute' stage for 'cuda' compilation target.
//CHECK: see using of 'GetDimensions'
//CHECK: see definition of 'GetDimensions'
//CHECK: see declaration of 'require'
//MS: multisampled texture is not supported on this target
//MS: 'Texture2DMS' is a multisampled texture, which is not supported by the current code generation target
