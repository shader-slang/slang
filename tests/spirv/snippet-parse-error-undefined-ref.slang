//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry computeMain -stage compute

// A `%name` operand referencing an instruction the snippet never defines is malformed user input
// and must be reported as snippet-parsing-failed (E29000), not abort the compiler.

RWStructuredBuffer<float> outBuf;

//CHECK: E29000
//CHECK: snippet
__target_intrinsic(spirv, "OpNop %missing")
float undefinedRef(float a);

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    outBuf[0] = undefinedRef(1.0f);
}
