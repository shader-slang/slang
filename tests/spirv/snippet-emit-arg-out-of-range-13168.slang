//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry computeMain -stage compute

// `_1` on a one-argument call pins the exclusive `_N < argCount` bound.

RWStructuredBuffer<float> outBuf;

//CHECK: E29003
//CHECK: out-of-range argument reference '_1' in target intrinsic snippet: OpNop _1
__target_intrinsic(spirv, "OpNop _1")
float outOfRangeArg(float a);

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    outBuf[0] = outOfRangeArg(1.0f);
}
