//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry computeMain -stage compute

// An operand identifier the snippet parser does not recognize is malformed user input and must be
// reported as spirv-snippet-unknown-operand (E29002), not abort the compiler.

RWStructuredBuffer<float> outBuf;

//CHECK: error E29002
//CHECK: unknown operand 'bogusOperand' in target intrinsic snippet: OpNop bogusOperand
__target_intrinsic(spirv, "OpNop bogusOperand")
float unknownOperand(float a);

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    outBuf[0] = unknownOperand(1.0f);
}
