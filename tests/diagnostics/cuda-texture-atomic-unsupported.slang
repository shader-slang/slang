// CUDA/PTX lowers the result-discarding `InterlockedAdd/Min/Max/And/Or` on an
// `RWTexture` texel to the PTX `sured` surface-reduction instruction (see
// shader-slang/slang#12636 and
// `tests/hlsl-intrinsic/atomic/cuda-texture-atomic.slang`). The cases below are
// the ones `sured` genuinely cannot express, so they must still produce
// diagnostic `E41405` rather than emitting `atomicAdd(surfObj[...])`, which
// subscripts an opaque `CUsurfObject` and is rejected by NVRTC (`no operator
// "[]" ... CUsurfObject [ uint2 ]`):
//   * the read-modify-write overload that returns the previous value — `sured`
//     has no result register, so the prior value is unobservable;
//   * `InterlockedXor` — ptxas has no `sured.xor` form;
//   * an atomic on an array texture — `sured` has no array geometry.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target cuda -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target ptx -entry computeMain -stage compute
// The `-minimum-slang-optimization` runs verify the diagnostic still fires when
// DCE/inlining behave differently, not only at the default optimization level.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target cuda -entry computeMain -stage compute -minimum-slang-optimization
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target ptx -entry computeMain -stage compute -minimum-slang-optimization

RWTexture2D<uint> tex;
RWTexture2DArray<uint> texArray;

// A supported destination on CUDA — must NOT trigger E41405. Its presence proves
// the diagnostic keys on the unsupported texel-atomic *shape*, not on
// `InterlockedAdd` on a texture in general (a plain scalar-texel add lowers to
// `sured`; see the positive test).
RWStructuredBuffer<uint> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    // Buffer atomics are supported: no E41405 for this line.
    //CHECK-NOT: E41405
    InterlockedAdd(outBuf[0], 1u);

    // Result-returning overload: the previous value is consumed, so DCE keeps
    // the atomic live and this read-modify-write form has no `sured` lowering.
    uint prev;
    InterlockedAdd(tex[uint2(1, 1)], 1u, prev);
    //CHECK:      ^ E41405
    outBuf[1] = prev;

    // `sured` has no xor form.
    InterlockedXor(tex[uint2(2, 2)], 3u);
    //CHECK:      ^ E41405

    // Array textures have no `sured` geometry.
    InterlockedAdd(texArray[uint3(0, 0, 0)], 4u);
    //CHECK:      ^ E41405
}
