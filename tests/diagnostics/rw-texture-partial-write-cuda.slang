// A write to part of a texel of an `RWTexture` on CUDA is lowered to a non-atomic
// read-modify-write of the whole texel, so we warn about it (shader-slang/slang#12627).
// A write that replaces the whole texel needs no read, so it is not diagnosed. The CUDA
// prelude converts on read only from half-based formats, and converts neither reads nor
// writes for layered surfaces, so a write that needs any other conversion is rejected.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target cuda -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=UNSUPPORTED, non-exhaustive): -target cuda -entry computeMain -stage compute -DUNSUPPORTED

RWTexture2D<float4> gTex;
[format("rgba8")]
RWTexture2D<float4> gUnormTex;

#ifdef UNSUPPORTED
[format("rgba16f")]
RWTexture2DArray<float4> gHalfArray;

[format("r16f")]
RWTexture2DArray<float> gHalfScalarArray;
#endif

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    gTex[tid.xy].w = 1.0f;
//CHECK:           ^ warning E56006
//CHECK:           ^ this write to part of a texel is lowered to a read of the whole texel followed by a write of the whole texel. On CUDA that read returns undefined data if the texel was already written earlier in the same kernel launch, so an earlier write to another component can be lost even within one thread. A concurrent write by another thread to another component of the texel can also be lost.

    gTex[tid.xy].xy = float2(1.0f, 2.0f);
//CHECK:            ^ warning E56006
//CHECK:            ^ this write to part of a texel is lowered to a read of the whole texel followed by a write of the whole texel. On CUDA that read returns undefined data if the texel was already written earlier in the same kernel launch, so an earlier write to another component can be lost even within one thread. A concurrent write by another thread to another component of the texel can also be lost.

    // Writing the whole texel, directly or through a swizzle that assigns every component,
    // is a plain surface write. A converting write from `rgba8` exists, so it is accepted.
    gTex[tid.xy] = float4(1.0f, 2.0f, 3.0f, 4.0f);
    gTex[tid.xy].xyzw = float4(1.0f, 2.0f, 3.0f, 4.0f);
    gUnormTex[tid.xy].wzyx = float4(1.0f, 2.0f, 3.0f, 4.0f);

#ifdef UNSUPPORTED
    // There is no converting read from `rgba8`.
    gUnormTex[tid.xy].w = 1.0f;
//UNSUPPORTED:          ^ error E56007
//UNSUPPORTED:          ^ this write to a texel is lowered to a CUDA surface read that converts

    // A layered surface has no converting read.
    gHalfArray[tid].w = 1.0f;
//UNSUPPORTED:        ^ error E56007
//UNSUPPORTED:        ^ this write to a texel is lowered to a CUDA surface read that converts

    // `.x` of a scalar texel writes the whole texel, but a layered surface has no
    // converting write either.
    gHalfScalarArray[tid].x = 1.0f;
//UNSUPPORTED:              ^ error E56007
//UNSUPPORTED:              ^ this write to a texel is lowered to a CUDA surface write that converts
#endif
}
