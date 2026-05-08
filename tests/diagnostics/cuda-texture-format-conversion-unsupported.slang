// Regression for #11088: CUDA codegen used to silently emit a half-family
// `_convert` call for any RWTexture access whose declared `[format(...)]`
// didn't match the element type, producing silently wrong results (e.g.
// reading 8 bytes as half when the backing format is rgba8).  The CUDA
// prelude only implements `_convert` specializations for the half-format
// family (r16f/rg16f/rgba16f).
//
// Until full format-conversion support lands, diagnose the mismatch with a
// clear error rather than generate silently wrong code.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target cuda -entry test -stage compute

[shader("compute")]
[numthreads(1, 1, 1)]
void test(
    [format("rgba8")] RWTexture2D<float4> texture,
    RWStructuredBuffer<float4> output)
{
    output[0] = texture[uint2(0)];
//CHECK: E31226
//CHECK: rgba8
}
