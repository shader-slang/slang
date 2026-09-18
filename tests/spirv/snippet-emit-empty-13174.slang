//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry computeMain -stage compute

// Regression for #13174: an empty snippet parses successfully but has no result instruction, so it
// must be diagnosed rather than crash at emit.

RWStructuredBuffer<float> outBuf;

//CHECK: E29001
//CHECK: contains no instructions
//CHECK-NOT: E99997
__target_intrinsic(spirv, "")
float emptySnippet(float a);

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    outBuf[0] = emptySnippet(1.0f);
}
