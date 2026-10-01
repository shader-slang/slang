// A write to part of a texel of an `RWTexture` on CUDA is lowered to a non-atomic
// read-modify-write of the whole texel, so we warn about it (shader-slang/slang#12627).
// The CUDA prelude has converting surface reads only for half-based formats and not for
// layered surfaces, so a partial write to any other format-converted texture is rejected.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target cuda -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=UNSUPPORTED, non-exhaustive): -target cuda -entry computeMain -stage compute -DUNSUPPORTED

RWTexture2D<float4> gTex;

#ifdef UNSUPPORTED
[format("rgba8")]
RWTexture2D<float4> gUnormTex;

[format("rgba16f")]
RWTexture2DArray<float4> gHalfArray;
#endif

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    gTex[tid.xy].w = 1.0f;
//CHECK:           ^ warning E56006
//CHECK:           ^ on target 'cuda', this write to part of a texel is lowered to a read-modify-write of the whole texel. A concurrent write by another thread to other components of the same texel can be lost, and on CUDA the read returns undefined data if the texel was already written earlier in the same kernel launch.

    // Writing the whole texel is a plain surface write: no warning.
    gTex[tid.xy] = float4(1.0f, 2.0f, 3.0f, 4.0f);

#ifdef UNSUPPORTED
    // There is no converting read from `rgba8`; the converting write itself is supported.
    gUnormTex[tid.xy].w = 1.0f;
//UNSUPPORTED:          ^ warning E56006
//UNSUPPORTED:          ^ 'RWTexture partial texel write (surface read)' is not supported

    // A layered surface has neither a converting read nor a working converting write.
    gHalfArray[tid].w = 1.0f;
//UNSUPPORTED:        ^ warning E56006
//UNSUPPORTED:        ^ 'RWTexture partial texel write (surface read)' is not supported
//UNSUPPORTED:        ^ 'RWTexture partial texel write (surface write)' is not supported
#endif
}
