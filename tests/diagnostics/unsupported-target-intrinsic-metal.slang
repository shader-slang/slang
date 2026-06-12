// Regression test for https://github.com/shader-slang/slang/issues/7873 (Metal).
//
// Companion to unsupported-target-intrinsic.slang, covering the Metal target and
// the two sentinel spellings that appear only in Metal `case metal:` blocks of
// `hlsl.meta.slang`: the typo'd plural `"<invalid intrinsics>"` and
// `"<Not supported>"`. Metal is a non-C++ target, so this also exercises the
// `(0)` placeholder fallback branch. Each must produce `E55204` rather than
// being emitted verbatim.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target metal -entry computeMain -stage compute

RWStructuredBuffer<float> outBuf;

__target_intrinsic(metal, "<invalid intrinsics>")
float metalUnsupportedOp(float x);

__target_intrinsic(metal, "<Not supported>")
float metalNotSupportedOp(float x);

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    outBuf[0] = metalUnsupportedOp(1.0);
//CHECK:                          ^ unsupported intrinsic operation
//CHECK:                          ^ intrinsic operation 'metalUnsupportedOp' is not supported for the current target.
    outBuf[1] = metalNotSupportedOp(1.0);
//CHECK:                           ^ unsupported intrinsic operation
//CHECK:                           ^ intrinsic operation 'metalNotSupportedOp' is not supported for the current target.
}
