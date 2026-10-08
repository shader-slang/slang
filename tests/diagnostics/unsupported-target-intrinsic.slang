// Regression test for https://github.com/shader-slang/slang/issues/7873
//
// When an operation has no valid lowering for the current target/shape, the
// meta-`.slang` modules mark it with a sentinel `__intrinsic_asm` definition
// (e.g. `"<invalid intrinsic>"` / `"<Not supported>"` for an unsupported CUDA
// texture op). These sentinels used to be emitted verbatim into the generated
// target code, producing uncompilable output (e.g. invalid PTX) with no
// diagnostic. They must instead produce a proper `E55204` error pointing at the
// use site. This file covers a C++-family target (CUDA) and both the
// `<invalid intrinsic>` and `<Not supported>` sentinel spellings.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target cuda -entry computeMain -stage compute

RWStructuredBuffer<float> outBuf;

// User-declared intrinsics carrying the same sentinel definitions the meta
// modules use for unsupported target/shape combinations.
__target_intrinsic(cuda, "<invalid intrinsic>")
float unsupportedOp(float x);

__target_intrinsic(cuda, "<Not supported>")
float notSupportedOp(float x);

// The exact `"invalid texture shape"` spelling (no angle brackets), used by the
// CUDA `Sample` default branches.
__target_intrinsic(cuda, "invalid texture shape")
float invalidShapeOp(float x);

// A `void`-returning intrinsic, exercising the branch that emits no placeholder
// value; the diagnostic must still fire.
__target_intrinsic(cuda, "<invalid intrinsic>")
void unsupportedVoidOp(float x);

// A non-scalar (vector) return type, so the C++-family `(T{})` placeholder
// branch runs `emitType` on something other than a scalar.
__target_intrinsic(cuda, "<invalid intrinsic>")
float4 unsupportedVecOp(float x);

// A generic intrinsic: the callee on the `IRCall` is a wrapper (an `IRSpecialize`
// of the generic) rather than the named function, so the operation name must be
// recovered through `getResolvedInstForDecorations` -- otherwise it would read
// '<unknown>'.
__target_intrinsic(cuda, "<invalid intrinsic>")
T unsupportedGenOp<T>(T x);

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    outBuf[0] = unsupportedOp(1.0);
//CHECK:                     ^ unsupported intrinsic operation
//CHECK:                     ^ intrinsic operation 'unsupportedOp' is not supported for the current target.
    outBuf[1] = notSupportedOp(1.0);
//CHECK:                      ^ unsupported intrinsic operation
//CHECK:                      ^ intrinsic operation 'notSupportedOp' is not supported for the current target.
    outBuf[2] = invalidShapeOp(1.0);
//CHECK:                      ^ unsupported intrinsic operation
//CHECK:                      ^ intrinsic operation 'invalidShapeOp' is not supported for the current target.
    unsupportedVoidOp(1.0);
//CHECK:             ^ unsupported intrinsic operation
//CHECK:             ^ intrinsic operation 'unsupportedVoidOp' is not supported for the current target.
    outBuf[3] = unsupportedVecOp(1.0).x;
//CHECK:                        ^ unsupported intrinsic operation
//CHECK:                        ^ intrinsic operation 'unsupportedVecOp' is not supported for the current target.
    outBuf[4] = unsupportedGenOp<float>(1.0);
//CHECK:                               ^ unsupported intrinsic operation
//CHECK:                               ^ intrinsic operation 'unsupportedGenOp' is not supported for the current target.
}
