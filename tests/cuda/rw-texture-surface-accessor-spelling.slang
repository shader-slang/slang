// The `RWTexture` `Load`/`Store` accessors spell CUDA surface calls through the `$C`
// (`_convert` suffix) and `$E` (x scale) intrinsic expansions. A format-converting read is
// byte addressed by the backing texel size, a converting write (`sust.p`) is not byte
// addressed, and an ordinary access is scaled by the element size.

//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=PTX): -target ptx -entry computeMain -stage compute

[format("rgba16f")]
RWTexture2D<float4> gHalfTex;
RWTexture2D<float4> gFloatTex;
RWStructuredBuffer<float4> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // CHECK: surf2Dread_convert<float4 >({{.*}}gHalfTex_{{[0-9]+}}{{.*}}.x * 8, {{.*}}.y, SLANG_CUDA_BOUNDARY_MODE)
    outputBuffer[0] = gHalfTex[tid.xy];

    // CHECK: surf2Dwrite_convert<float4 >({{.*}}gHalfTex_{{[0-9]+}}{{.*}}.x * 1, {{.*}}.y, SLANG_CUDA_BOUNDARY_MODE)
    gHalfTex[tid.xy] = float4(1.0f, 2.0f, 3.0f, 4.0f);

    // CHECK: surf2Dwrite<float4 >({{.*}}gFloatTex_{{[0-9]+}}{{.*}}.x * 16, {{.*}}.y, SLANG_CUDA_BOUNDARY_MODE)
    gFloatTex[tid.xy] = float4(1.0f, 2.0f, 3.0f, 4.0f);
}

// The converting read of a half format needs `half`, which nvrtc must accept.
// PTX: .entry
// PTX: suld
// PTX: sust
