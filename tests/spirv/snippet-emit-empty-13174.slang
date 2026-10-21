//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry computeMain -stage compute

// An empty snippet parses but has no result instruction, so it is diagnosed before emission.

RWStructuredBuffer<float> outBuf;

//CHECK: E29004
//CHECK: no SPIR-V instructions in target intrinsic snippet
__target_intrinsic(spirv, "")
float emptySnippet(float a);

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    outBuf[0] = emptySnippet(1.0f);
}
