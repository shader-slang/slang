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
//   * an atomic on an array texture — `sured` has no array geometry;
//   * a texture whose *directly recoverable* `[format(...)]` is incompatible with
//     the element type (a different byte width / scalar type), because the byte
//     stride we would compute from the element type would not match the true
//     backing layout.
//
// The multisampled-texture case is diagnosed by a *different*, earlier path
// (`checkUnsupportedTextureAtomic`, which runs before emission because the CUDA
// emitter cannot even name a multisample resource type) and would short-circuit
// the compile before the emit-point diagnostics here fire, so it lives in its
// own file: `cuda-texture-atomic-multisample-unsupported.slang`.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target cuda -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target ptx -entry computeMain -stage compute
// The `-minimum-slang-optimization` runs verify the diagnostic still fires when
// DCE/inlining behave differently, not only at the default optimization level.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target cuda -entry computeMain -stage compute -minimum-slang-optimization
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target ptx -entry computeMain -stage compute -minimum-slang-optimization

RWTexture2D<uint> tex;
RWTexture2DArray<uint> texArray;

// A directly-declared texture whose format representation contradicts the element
// type: `r32ui` is a 4-byte channel but the element is 8-byte `uint64_t`. The
// format is recoverable here (the resource is used directly), so the incompatible
// representation is caught and diagnosed rather than emitting a wrong byte stride.
[format("r32ui")]
RWTexture2D<uint64_t> texBadFormat;

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

    // Directly recoverable format incompatible with the element type (r32ui vs
    // uint64_t): the byte stride is unprovable, so this is diagnosed rather than
    // emitting a wrong offset.
    InterlockedAdd(texBadFormat[uint2(3, 3)], 5ull);
    //CHECK:      ^ E41405
}
