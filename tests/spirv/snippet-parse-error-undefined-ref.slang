//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry computeMain -stage compute

// A `%name` operand referencing an instruction the snippet never defines is malformed user input
// and must be reported as spirv-snippet-undefined-id (E29001), not abort the compiler.

RWStructuredBuffer<float> outBuf;

//CHECK: error E29001
//CHECK: undefined SPIR-V id '%missing' in target intrinsic snippet: OpNop %missing
__target_intrinsic(spirv, "OpNop %missing")
float undefinedRef(float a);

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    outBuf[0] = undefinedRef(1.0f);
}
