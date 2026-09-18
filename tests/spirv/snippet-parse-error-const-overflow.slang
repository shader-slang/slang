//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry computeMain -stage compute

// A `const(...)` operand with more values than the fixed value-array capacity
// (`kMaxASMConstantValues` == 4) must be reported as snippet-parsing-failed (E29000). The value
// loop is bounded to that capacity so the extra values cannot be written past the arrays.

RWStructuredBuffer<float> outBuf;

//CHECK: E29000
//CHECK: snippet
__target_intrinsic(spirv, "OpNop const(uint, 1, 2, 3, 4, 5)")
float tooManyConstValues(float a);

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    outBuf[0] = tooManyConstValues(1.0f);
}
