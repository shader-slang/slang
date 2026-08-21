// An atomic operation on a texel of an `RWTexture` must produce a clear
// capability-style diagnostic on CUDA/PTX rather than compiling to
// `atomicAdd(surfObj[...])`, which subscripts an opaque `CUsurfObject` and is
// rejected by NVRTC (`no operator "[]" ... CUsurfObject [ uint2 ]`). See
// shader-slang/slang#12636. CUDA has no lowering for an atomic whose
// destination is a texture texel (no surface-atomic C++ intrinsic exists).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target cuda -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target ptx -entry computeMain -stage compute
// The `-minimum-slang-optimization` runs verify the diagnostic fires at every
// optimization level, not only the default one.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target cuda -entry computeMain -stage compute -minimum-slang-optimization
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target ptx -entry computeMain -stage compute -minimum-slang-optimization

RWTexture2D<uint> outputTexture;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    InterlockedAdd(outputTexture[uint2(0, 0)], 1u);

    // Result-returning overload: unsupported by the planned `sured` lowering,
    // since PTX `sured` does not return the prior value.
    uint prev;
    InterlockedAdd(outputTexture[uint2(1, 1)], 1u, prev);
}

//CHECK: E41405
//CHECK: E41405
