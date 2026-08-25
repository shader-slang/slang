// A texel atomic on a *multisampled* `RWTexture` has no `sured` lowering on
// CUDA/PTX (`sured` has no multisample geometry). Unlike the other unsupported
// texel-atomic shapes (result-returning / `xor` / array), which the CUDA emitter
// turns into `E41405` at its emit point, a multisample texture type is not even
// representable by the CUDA emitter (`_calcCUDATextureTypeName` fails on it), so
// this must be diagnosed *before* emission — otherwise the compiler aborts when
// the resource type is emitted. The `checkUnsupportedTextureAtomic` pass does
// that, and runs at every optimization level (hence the
// `-minimum-slang-optimization` runs), because the abort is not
// optimization-dependent. See shader-slang/slang#12636.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target cuda -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target ptx -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target cuda -entry computeMain -stage compute -minimum-slang-optimization
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target ptx -entry computeMain -stage compute -minimum-slang-optimization

RWTexture2DMS<uint> texMS;
RWTexture2DMSArray<uint> texMSArray;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    // The second subscript argument is the sample index.
    InterlockedAdd(texMS[uint2(0, 0), 0], 1u);
    //CHECK:      ^ E41405

    InterlockedAdd(texMSArray[uint3(0, 0, 0), 0], 2u);
    //CHECK:      ^ E41405
}
