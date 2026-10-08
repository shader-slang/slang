// Regression test for https://github.com/shader-slang/slang/issues/7873 (WGSL).
//
// Companion to unsupported-target-intrinsic.slang, exercising a non-C++ target.
// The WGSL meta module uses `"<invalid intrinsic: unimplemented shape>"` for
// unsupported array-texture sample shapes. The sentinel must produce `E55204`
// rather than being emitted verbatim, and the placeholder fallback value must
// not be C++-only `{}` syntax for this target.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target wgsl -entry computeMain -stage compute

RWStructuredBuffer<float> outBuf;

__target_intrinsic(wgsl, "<invalid intrinsic: unimplemented shape>")
float unsupportedWgslOp(float x);

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    outBuf[0] = unsupportedWgslOp(1.0);
//CHECK:                         ^ unsupported intrinsic operation
//CHECK:                         ^ intrinsic operation 'unsupportedWgslOp' is not supported for the current target.
}
