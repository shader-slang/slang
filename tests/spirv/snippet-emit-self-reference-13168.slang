//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry computeMain -stage compute

// The parser registers `%a` before its operands, so this self-reference reaches legalization,
// which must reject it.

RWStructuredBuffer<float> outBuf;

//CHECK: E29003
//CHECK: SPIR-V id not defined by an earlier instruction '%a' in target intrinsic snippet: %a = OpNop %a
__target_intrinsic(spirv, "%a = OpNop %a")
float selfReference(float a);

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    outBuf[0] = selfReference(1.0f);
}
